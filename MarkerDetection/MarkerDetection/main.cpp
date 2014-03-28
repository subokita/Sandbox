//
//  main.cpp
//  MarkerDetection
//
//  Created by Saburo Okita on 27/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//


#include <iostream>
#include <opencv2/opencv.hpp>

#include "MarkerDetector.h"


using namespace std;
using namespace cv;

int main(int argc, const char * argv[]) {
    /* Load our intrinsic paramaters and distortion coefficients */
    Mat cam_matrix, dist_coeff;
    FileStorage fs;
    fs.open("/Users/saburookita/Desktop/iPhone Calib/calibration.yml", FileStorage::READ );
    fs["camera matrix"] >> cam_matrix;
    fs["dist coeff"]    >> dist_coeff;
    
    /* Start recording scene (in this case from my video) */
    VideoCapture cap("/Users/saburookita/Desktop/IMG_0314.MOV");
    Mat frame, prev_frame, output;
    
    cap.set( CV_CAP_PROP_BRIGHTNESS, 50.0 );
    cap.set( CV_CAP_PROP_SATURATION, 50.0 );
    cap.set( CV_CAP_PROP_CONTRAST, 50.0 );

    namedWindow( "" );
    moveWindow("", 0, 0 );
    
    MarkerDetector detector( 100.0f, 6, 6 );
    Marker marker;
    bool found = false;
    
    while( true ) {
        cap >> frame;
        
        if( frame.empty() )
            break;
        
        /* Perform some resize and rotation, due to large portrait video taken from my phone */
        resize( frame, frame, Size(), 0.5f, 0.5f );
        Mat rotation = getRotationMatrix2D(Point2f(frame.cols/2, frame.rows/2), -90, 1.0 );
        warpAffine( frame, frame, rotation, Size() );
        output = frame.clone();
        
        cvtColor( frame, frame, CV_BGR2GRAY );
        
        /* Find candidate markers */
        vector<Marker> candidates = detector.findMarkerCandidates( frame );
        
        if( !candidates.empty()  ) {
            /* For our example, just concentrate on one marker */
            marker = candidates[0];
          
            /* this section is supposed to be used later to determine the rotation and translation of camera */
            /* but place it here for now */
            static vector<Point3f> target_corners = {
                Point3f( -0.5, -0.5, 0.0 ),
                Point3f(  0.5, -0.5, 0.0 ),
                Point3f(  0.5,  0.5, 0.0 ),
                Point3f( -0.5,  0.5, 0.0 ),
            };
            
            Mat rvec, tvec;
            solvePnP( target_corners, marker.poly, cam_matrix, dist_coeff, rvec, tvec );
            
            /* Convert from rodrigues' rotation vector to rotation matrix */
            Rodrigues( rvec, rvec );
            
            /* Check if it's projecting back to correct points */
            vector<Point2f> points;
            projectPoints( target_corners, rvec, tvec, cam_matrix, dist_coeff, points );
             
            line( output, points[0], points[1], Scalar(255, 0, 0), 3 );
            line( output, points[1], points[2], Scalar(255, 0, 0), 3 );
            line( output, points[2], points[3], Scalar(255, 0, 0), 3 );
            line( output, points[3], points[0], Scalar(255, 0, 0), 3 );
            
            /* Might as well, show on the left hand corner the marker that we've seen */
            int rows = marker.matrix.rows;
            int cols = marker.matrix.cols;
            Mat temp( rows * 10, cols * 10, CV_8UC1, Scalar(0) );
            for( int y = 0; y < rows; y++ ) {
                uchar * ptr = marker.matrix.ptr(y);
                for( int x = 0; x < cols; x++ ) {
                    temp.rowRange(y * 10, y * 10+10).colRange( x * 10, x * 10 + 10 ) = ptr[x];
                }
            }
            
            vector<Mat> out = {temp, temp, temp};
            merge( out, temp );
            temp.copyTo( Mat(output, Rect(0, 0, temp.cols, temp.rows)) );
            
            found = true;
        }
        else {
            /* If we can't find marker from usual method, resort to optical flow */
            if( !marker.poly.empty() ) {
                if (detector.opticalFlowPrediction( prev_frame, frame, marker ) ){
                    line( output, marker.poly[0], marker.poly[1], Scalar(0, 0, 255), 3 );
                    line( output, marker.poly[1], marker.poly[2], Scalar(0, 0, 255), 3 );
                    line( output, marker.poly[2], marker.poly[3], Scalar(0, 0, 255), 3 );
                    line( output, marker.poly[3], marker.poly[0], Scalar(0, 0, 255), 3 );
                }
                else
                    marker.poly.clear();
            }
        }
        
        
        prev_frame = frame.clone();
        
        imshow( "", output );
        
        if(waitKey(10) == 27)
            break;
    }
    return 0;
}

