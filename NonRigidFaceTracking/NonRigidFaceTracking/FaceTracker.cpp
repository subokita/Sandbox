//
//  FaceTracker.cpp
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 18/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "FaceTracker.h"


double FaceTracker::probSameScene( Mat& gray ) {
    static int histSize = 256;
    static float range[] = { 0, 256 } ;
    static const float* histRange = { range };
    double result = 0.0;
    
    Mat histogram;
    calcHist( &gray, 1, 0, Mat(), histogram, 1, &histSize, &histRange, true, false );
    normalize(histogram, histogram, 0, 1, NORM_MINMAX, -1, Mat() );
    
    if( !prevHistogram.empty() )
        result = compareHist(histogram, prevHistogram, CV_COMP_CORREL );
    
    prevHistogram = histogram;
    return result;
}

bool FaceTracker::track( const Mat& image, vector<Rect>& faces, const vector<Size>& levels, const bool robust, const int itol, const float ftol, const float scale_factor, const int min_neighbors, const Size min_size ) {
    Mat gray;
    cvtColor( image, gray, CV_RGB2GRAY );
    
    if( probSameScene( gray ) < 0.8 )
        tracking = false;
    
    if( !tracking )
        points = detector.detect( gray, faces );
    
    if( points.size() != shapeModel.noOfPoints() )
        return false;
    
    for( Size level: levels )
        points = fit( gray, points, level, robust, itol, ftol );
    
    tracking = true;
    
    return true;
}


vector<Point2f> FaceTracker::fit( const Mat& image, const vector<Point2f>& init,
                                  const Size ssize, const bool robust, const int itol, const float ftol ) {
    int n = shapeModel.noOfPoints();
    shapeModel.calcParams( init );
    vector<Point2f> points = shapeModel.calcShape();
    vector<Point2f> peaks = patchModels.calcPeaks( image, points, ssize );
    
    if( !robust ){
        shapeModel.calcParams( peaks );
        points = shapeModel.calcShape();
    }
    else {
        Mat weight( n, 1, CV_32F );
        Mat weight_sort( n, 1, CV_32F );
        vector<Point2f> prev_points = points;
        
        for( int iter = 0; iter < itol; iter++ ) {
            for( int i = 0; i < n; i++ )
                weight.at<float>(i) = norm( points[i] - peaks[i] );
            
            cv::sort( weight, weight_sort, CV_SORT_EVERY_COLUMN | CV_SORT_ASCENDING );
            
            /* Estimating standard deviation from median absolute deviation, which can be done by 
             * std dev ~= 1.4826 x MAD
             * http://en.wikipedia.org/wiki/Median_absolute_deviation */
            double stddev = 1.4826 * weight_sort.at<float>(n / 2);
            if( stddev < 0.1 )
                stddev = 0.1;
            
            pow( weight, 2, weight );
            weight *= -0.5 / (stddev * stddev);
            exp( weight, weight );
            
            shapeModel.calcParams( peaks, weight );
            points = shapeModel.calcShape();
            
            float v = 0;
            for( int i = 0; i < n; i++ )
                v += norm( points[i] - prev_points[i] );
            
            if( v < ftol )
                break;
            else
                prev_points = points;
        }
    }
    
    return points;
}