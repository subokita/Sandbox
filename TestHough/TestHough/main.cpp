//
//  main.cpp
//  TestHough
//
//  Created by Saburo Okita on 27/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>


#include "Hough.h"

using namespace cv;
using namespace std;

static Point accumIndex(-1, -1);

static void onMouse( int event, int x, int y, int, void * data );


Vec3b HSVtoRGB( float h, float s, float v ) {
    float r, g, b;
	int i;
	float f, p, q, t;
	if( s == 0 ) {
		// achromatic (grey)
		r = g = b = v;
		return Vec3b( b, g, r );
	}
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		default:		// case 5:
			r = v;
			g = p;
			b = q;
			break;
	}
    return Vec3b( b, g, r );
}

int main(int argc, const char * argv[]) {
    namedWindow( "" );
    moveWindow("", 0, 0);
    
    Mat image = imread( "/Users/saburookita/Sandbox/TestHough/live-action-patlabor-backdrop.jpg" );
    resize( image, image, Size(), 0.5, 0.5 );
    
    
    /* Find the edges of the image */
    Mat gray, edges;
    cvtColor( image, gray, CV_BGR2GRAY );
    Canny( gray, edges, 100, 300, 3 );
    
    
    /* Initialize the Hough accumulation matrix */
    Hough hough;
    hough.init( edges );
    
    
    /* Make the Canny edges, blue, real blue */
    vector<Mat> temp = {
        edges,
        Mat::zeros( edges.size(), CV_8UC1),
        Mat::zeros( edges.size(), CV_8UC1)
    };
    merge( temp, edges );
    
    
    /* Get the size of accum matrix in advance, for the mouse call back purpose  */
    Mat accum = hough.getAccumulationMatrix();
    resize( accum, accum, Size(), 2.0, 0.5 );
    Rect region( image.cols, 0, accum.cols, accum.rows );
    setMouseCallback( "", onMouse, static_cast<void*>( &region ) );
    
    
    /* Final output matrix, will be combination of the original image and the accumulation matrix */
    Mat appended( MAX( accum.rows, image.rows ), image.cols + accum.cols, CV_8UC3, Scalar(0, 0, 0) );
    
    
    bool show_canny = false;
    int threshold = 0;
    createTrackbar( "Hough threshold", "", &threshold, 1000 );
    
    /* Stuff for drawing text */
    CvFont font = cvFontQt("Helvetica", 14.0, CV_RGB(0, 255, 0) );
    char str[255];
    
    while( true ) {
        Mat temp = image.clone();
        if( show_canny )
            temp += edges;
        
        /* Try to visualize the accumulation matrix */
        accum = hough.getAccumulationMatrix( threshold );
        accum.convertTo( accum, CV_8UC1 );
        equalizeHist( accum, accum );
        
        /* Apply colormap for better representation of the accum matrix */
        applyColorMap( accum, accum, cv::COLORMAP_JET );
        resize( accum, accum, Size(), 2.0, 0.5 );
        
        /* Draw the lines based on threshold */
        vector<pair<Point, Point>> lines = hough.getLines( threshold );
        for( pair<Point, Point> point_pair : lines )
            line( temp, point_pair.first, point_pair.second, CV_RGB(255, 0, 0), 1 );
    
        
        /* Draw lines based on cursor position */
        if(accumIndex.x != -1 && accumIndex.y != -1 ) {
            pair<Point, Point> point_pair = hough.getLine( accumIndex.y, accumIndex.x );
            line( temp, point_pair.first, point_pair.second, CV_RGB(0, 255, 0), 1 );
        }
    
        
        /* Copy everything to output matrix */
        appended = Scalar::all(0);
        temp.copyTo ( Mat(appended, Rect(0, 0, temp.cols, temp.rows)) );
        accum.copyTo( Mat(appended, Rect(temp.cols, 0, accum.cols, accum.rows))  );
        
        
        /* Output some text */
        addText( appended, "Accum matrix", Point( temp.cols + 10, 15 ), font );
        addText( appended, "[C] to show Canny edges", Point( 10, image.rows + 15 ), font );
        addText( appended, "[Q] to quit", Point( 10, image.rows + 30 ), font );
        sprintf( str, "Threshold: %d", threshold );
        addText( appended, str, Point( 10, image.rows + 45 ), font );
        sprintf( str, "Rho: %d   Theta: %d", accumIndex.y - accum.rows / 2, accumIndex.x );
        addText( appended, str, Point( 10, image.rows + 60 ), font );

        
        imshow( "", appended );
        char key = waitKey(10);
        if( key == 'q' )
            break;
        else if( key == 'c' )
            show_canny = !show_canny;
    }
    
    return 0;
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
