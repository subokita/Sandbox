//
//  main.cpp
//  FSD
//
//  Created by Saburo Okita on 02/05/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>

#include <tbb/tbb.h>
#include "FastSymmetryDetector.h"

using namespace std;
using namespace cv;

string filename = "/Users/saburookita/Desktop/sample1.jpg";

static Point accumIndex(-1, -1);
static void onMouse( int event, int x, int y, int, void * data );

void testImage();
void testVideo();

int main(int argc, const char * argv[]) {
    testImage();
//    testVideo();
    
    return 0;
}

/**
 * Test using the scene captured from the webcam video
 */
void testVideo() {
    namedWindow("");
    moveWindow("", 0, 0);
    
    Mat frame;
    VideoCapture cap(0);
    while( frame.empty() )
        cap >> frame;
    
    /* Resize the image accordingly */
    resize( frame, frame, Size(), 0.7, 0.7 );
    
    /* Determine the shape of Hough accumulationmatrix */
    float rho_divs   = hypotf( frame.rows, frame.cols ) + 1;
    float theta_divs = 180.0;
    
    FastSymmetryDetector detector( frame.size(), Size(rho_divs, theta_divs), 1 );
    
    /* Adjustable parameters, depending on the scene condition */
    int canny_thresh_1 = 30;
    int canny_thresh_2 = 90;
    int min_pair_dist  = 25;
    int max_pair_dist  = 500;
    int no_of_peaks    = 1;
    
    createTrackbar( "canny_thresh_1", "", &canny_thresh_1, 500 );
    createTrackbar( "canny_thresh_2", "", &canny_thresh_2, 500 );
    createTrackbar( "min_pair_dist", "", &min_pair_dist, 500 );
    createTrackbar( "max_pair_dist", "", &max_pair_dist, 500 );
    createTrackbar( "no_of_peaks", "", &no_of_peaks, 10 );
    
    Mat edge;
    while( true ){
        cap >> frame;
        flip( frame, frame, 1 );
        resize( frame, frame, Size(), 0.7, 0.7 );
        
        /* Find the edges of the image */
        cvtColor( frame, edge, CV_BGR2GRAY );
        Canny( edge, edge, canny_thresh_1, canny_thresh_2 );
        
        /* Vote for the hough matrix */
        detector.vote( edge, min_pair_dist, max_pair_dist );
        Mat accum = detector.getAccumulationMatrix();
        
        /* Get the result and draw the symmetrical line */
        vector<pair<Point, Point>> result = detector.getResult( no_of_peaks );
        for( auto point_pair: result )
            line(frame, point_pair.first, point_pair.second, Scalar(0, 0, 255), 3);

        /* Convert our Hough accum matrix to heat map */
        accum.convertTo( accum, CV_8UC3 );
        applyColorMap( accum, accum, COLORMAP_JET );
        resize( accum, accum, Size(), 2.0, 0.5 );
        
        /* Show the original, edge and the accumulation image */
        Mat appended = Mat::zeros( frame.rows + accum.rows, frame.cols * 2, CV_8UC3 );
        frame.copyTo( Mat(appended, Rect(0, 0, frame.cols, frame.rows)) );
        cvtColor( edge, Mat(appended, Rect(frame.cols, 0, edge.cols, edge.rows)), CV_GRAY2BGR );
        accum.copyTo( Mat( appended, Rect(0, frame.rows, accum.cols, accum.rows) ) );

        
        imshow( "", appended );
        if(waitKey(10) == 'q')
            break;
    }
}


/*
 * Test on given image
 */
void testImage() {
    namedWindow("");
    moveWindow("", 0, 0);
    
    Mat frame = imread( filename );
    
    /* Determine the shape of Hough accumulationmatrix */
    float rho_divs   = hypotf( frame.rows, frame.cols ) + 1;
    float theta_divs = 180.0;
    
    FastSymmetryDetector detector( frame.size(), Size(rho_divs, theta_divs), 1 );
    
    
    Rect region( 0, frame.rows, theta_divs * 2.0, rho_divs * 0.5 );
    setMouseCallback( "", onMouse, static_cast<void*>( &region ) );
    Mat temp, edge;
    
    /* Adjustable parameters, depending on the scene condition */
    int canny_thresh_1 = 30;
    int canny_thresh_2 = 90;
    int min_pair_dist  = 25;
    int max_pair_dist  = 500;
    int no_of_peaks    = 1;
    
    createTrackbar( "canny_thresh_1", "", &canny_thresh_1, 500 );
    createTrackbar( "canny_thresh_2", "", &canny_thresh_2, 500 );
    createTrackbar( "min_pair_dist", "", &min_pair_dist, 500 );
    createTrackbar( "max_pair_dist", "", &max_pair_dist, 500 );
    createTrackbar( "no_of_peaks", "", &no_of_peaks, 10 );
    
    while( true ) {
        temp = frame.clone();
        
        /* Find the edges */
        cvtColor( temp, edge, CV_BGR2GRAY );
        Canny( edge, edge, canny_thresh_1, canny_thresh_2 );
        
        /* Vote for the accumulation matrix */
        detector.vote( edge, min_pair_dist, max_pair_dist );
        
        /* Draw the symmetrical line */
        vector<pair<Point, Point>> result = detector.getResult( no_of_peaks );
        for( auto point_pair: result )
            line(temp, point_pair.first, point_pair.second, Scalar(0, 0, 255), 2);
        
        /* Visualize the Hough accum matrix */
        Mat accum = detector.getAccumulationMatrix();
        accum.convertTo( accum, CV_8UC3 );
        applyColorMap( accum, accum, COLORMAP_JET );
        resize( accum, accum, Size(), 2.0, 0.5 );
        
        /* Draw lines based on cursor position */
        if(accumIndex.x != -1 && accumIndex.y != -1 ) {
            pair<Point, Point> point_pair = detector.getLine( accumIndex.y, accumIndex.x );
            line( temp, point_pair.first, point_pair.second, CV_RGB(0, 255, 0), 2 );
        }
        
        /* Show the original and edge images */
        Mat appended = Mat::zeros( temp.rows + accum.rows, temp.cols * 2, CV_8UC3 );
        temp.copyTo( Mat(appended, Rect(0, 0, temp.cols, temp.rows)) );
        cvtColor( edge, Mat(appended, Rect(temp.cols, 0, edge.cols, edge.rows)), CV_GRAY2BGR );
        accum.copyTo( Mat( appended, Rect(0, temp.rows, accum.cols, accum.rows) ) );
        
        imshow( "", appended );
        if(waitKey(10) == 'q')
            break;
    }
}



/**
 * Mouse callback, to show the line based on which part of accumulation matrix the cursor is.
 */
static void onMouse( int event, int x, int y, int, void * data ) {
    Rect *region = (Rect*) data;
    Point point( x, y );
    
    if( (*region).contains( point ) ) {
        accumIndex.x = (point.x - region->x) / 2.0;
        accumIndex.y = (point.y - region->y) * 2.0;
    }
    else {
        accumIndex.x = -1;
        accumIndex.y = -1;
    }
}
