//
//  MarkerDetector.cpp
//  MarkerDetection
//
//  Created by Saburo Okita on 28/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "MarkerDetector.h"

MarkerDetector::MarkerDetector() {
}


MarkerDetector::MarkerDetector( float min_contour_size, int marker_width, int marker_height ) {
    init( min_contour_size, marker_width, marker_height );
}


void MarkerDetector::init( float min_contour_size, int marker_width, int marker_height ) {
    this->minContourSize    = min_contour_size;
    this->markerWidth       = marker_width;
    this->markerHeight      = marker_height;
}

/**
 * Check if the given points match our initial criteria of being a candidate marker, i.e.:
 * - has 4 points
 * - is convex
 * - at least is of specific contour size
 *
 * Since approxPolyDP requires Point, and calcOpticalFlowPyrLK requires Point2f, and both of them can't be used
 * interchangably, I decided to go for template instead.
 *
 * @param points vector of points
 * @return true if points pass the check to be candidate marker
 */
template<typename T>
bool MarkerDetector::checkPoints( const vector<T>& points  ) {
    /* Avoid anything that's not 4 sided and not convex */
    if( points.size() != 4 )
        return false;
    
    if (isContourConvex( points ) == false )
        return false;
    
    /* Make sure that, the length of the sides ain't too small */
    float min_dist = numeric_limits<float>::max();
    for( int i = 0; i < 4; i++ ) {
        T side = points[i] - points[(i+1)%4];
        float squared_length = side.dot( side );
        min_dist = min( min_dist, squared_length );
    }
    
    if( min_dist < minContourSize )
        return false;
    
    return true;
}

/**
 * This is the backup plan, when the normal way to find marker yields 0 result, we resort
 * to use optical flow from previous detected marker position.
 *
 * @param prev_frame previous captured frame, of 8UC1 type
 * @param frame current frame, also of 8UC1 / grayscale type
 * @param marker the previously detected Marker object, we're basically just looking at the points
 * @return true if optical flow found a candidate marker
 **/
bool MarkerDetector::opticalFlowPrediction( Mat& prev_frame, Mat& frame, Marker& marker ) {
    Mat status, error;
    vector<Point2f> new_points;
    
    /* Use sparse optical flow to predict where the points flowed to, dense version is too slow */
    calcOpticalFlowPyrLK( prev_frame, frame, marker.poly, new_points, status, error );
    
    /* Make sure that all points are detected */
    if ( sum( status ) != Scalar(4) )
        return false;
    
    /* make sure that it fits our candidates requirements */
    if( !checkPoints( new_points ))
        return false;
    
    /* just ignore it when it's already partially out of the scene */
    for( Point2f point : new_points ) {
        if( point.y <= 0 || point.x <= 0 || point.x >= frame.cols || point.y >= frame.rows )
            return false;
    }

    marker.poly.clear();
    copy( new_points.begin(), new_points.end(), back_inserter( marker.poly ));
    
    return true;
}

/**
 * Find a list of candidate marker from a given scene
 *
 * @param current frame, in grayscale 8UC1 format
 * @return a list of marker candidates
 **/
vector<Marker> MarkerDetector::findMarkerCandidates( Mat& frame ) {
    vector<Marker> candidates;
    
    /* Do some thresholding, in fact you should tune the parameters here a bit */
    Mat thresholded;
    threshold( frame, thresholded, 50.0, 255.0, CV_THRESH_BINARY );
    
    /* Find contours */
    vector<vector<Point>> contours;
    findContours( thresholded.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE );
    
    for( vector<Point> contour: contours ) {
        /* Approximate polygons out of these contours */
        vector<Point> approxed;
        approxPolyDP( contour, approxed, contour.size() * 0.05, true );
        
        /* Make sure it passes our first candidate check */
        if( !checkPoints( approxed ) )
            continue;
        
        /* Do some perspective transformation on the candidate marker to a predetermined square */
        Marker marker;
        marker.matrix = Mat( markerHeight, markerWidth, CV_8UC1 );
        std::copy( approxed.begin(), approxed.end(), back_inserter( marker.poly ) );
        
        /* Apply sub pixel search */
        cornerSubPix( thresholded, marker.poly, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 40, 0.001) );
        
        /* Projection target */
        const static vector<Point2f> target_corners = {
            Point2f( -0.5f, -0.5f ),
            Point2f( +5.5f, -0.5f ),
            Point2f( +5.5f, +5.5f ),
            Point2f( -0.5f, +5.5f ),
        };
        
        /* Apply perspective transformation, to project our 3D marker to a predefined 2D coords */
        Mat projection = getPerspectiveTransform( marker.poly, target_corners );
        warpPerspective( thresholded, marker.matrix, projection, marker.matrix.size() );
        
        /* Ignore those region that's fully black, or not surrounded by black bars */
        if( sum(marker.matrix) == Scalar(0) ||
           countNonZero( marker.matrix.row(0)) != 0 ||
           countNonZero( marker.matrix.row(markerHeight - 1)) != 0 ||
           countNonZero( marker.matrix.col(0)) != 0 ||
           countNonZero( marker.matrix.col(markerWidth - 1)) != 0 ) {
            continue;
        }
        
        
        /* Find the rotation that has the smallest hex value */
        pair<unsigned int, unsigned int> minimum = { numeric_limits<unsigned int>::max(), 0 };
        vector<unsigned int> codes(markerHeight);
        unsigned int power = 1 << (markerWidth - 3);
        
        /* Rotate the marker 4 times, store the hex code upon each rotation */
        for( int rotation = 0; rotation < 4; rotation++ ) {
            stringstream ss;
            codes[rotation] = 0;
            
            for( int i = 1; i < markerHeight - 1; i++ ) {
                unsigned int code = 0;
                for ( int j = 1; j < markerWidth - 1; j++ ){
                    int value = static_cast<int>(marker.matrix.at<uchar>(i, j));
                    if( value == 0 )
                        code = code + ( power >> j );
                }
                
                ss << hex << code;
            }
            ss >> codes[rotation];
            
            if( minimum.first > codes[rotation] ) {
                minimum.first  = codes[rotation];
                minimum.second = rotation;
            }
            
            flip( marker.matrix, marker.matrix, 1 );
            marker.matrix = marker.matrix.t();
        }
        
        
        rotate( marker.poly.begin(), marker.poly.begin() + ((minimum.second + 2) % 4), marker.poly.end() );
        for( int i = 0; i < minimum.second; i++ ) {
            flip( marker.matrix, marker.matrix, 1 );
            marker.matrix = marker.matrix.t();
        }
        
        marker.code = minimum.first;
        
        candidates.push_back( marker );
    }
    
    return candidates;
}
