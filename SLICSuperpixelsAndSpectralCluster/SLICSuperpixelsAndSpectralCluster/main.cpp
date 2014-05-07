//
//  main.cpp
//  SLICSuperpixelsAndSpectralCluster
//
//  Created by Saburo Okita on 07/05/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>
#include "SLICSuperpixel.h"
#include "SuperpixelSegmentation.h"

using namespace std;
using namespace cv;

void testSpectralSegment( Mat& image, int no_of_superpixels, int k_clusters, float sigma ) {
    namedWindow( "" );
    moveWindow("", 0, 0);
    
    /* First generate SLIC superpixels */
    SLICSuperpixel slic( image, no_of_superpixels );
    slic.generateSuperPixels();
    
    
    /* Perform spectral clusterings on the 400 superpixel cluster centers */
    SuperpixelSegmentation segmenter( image.size(), sigma );
    
    vector<ColorRep> centers = slic.getCenters();
    Mat clusters_index = slic.getClustersIndex();
    
    segmenter.calculateEigenvectors( centers, slic.getS(), slic.getM() );
    Mat mask = segmenter.applySegmentation( k_clusters, clusters_index );
    
    
    
    /* UI purposes, the region where to draw the segmented image */
    CvFont font = cvFontQt("Helvetica", 28.0, CV_RGB(0, 255, 0) );
    vector<Rect> regions = {
        Rect(0, 0, image.cols, image.rows),
        Rect(image.cols, 0, image.cols, image.rows),
        Rect(0, image.rows, image.cols, image.rows),
        Rect(image.cols, image.rows, image.cols, image.rows),
    };
    
    Mat appended( image.rows * 2, image.cols * 2, CV_8UC3, Scalar::all(0) );
    
    
    /* Draw original image, and then all the segmented ones */
    image.copyTo( Mat(appended, regions[0]) );
    addText( appended, "Original", Point( regions[0].x + 30, regions[0].y + 30 ), font );
    
    
    /* Draw the superpixel clusters */
    Mat slic_contour = slic.recolor();
    cvtColor( slic_contour, slic_contour, CV_Lab2BGR );
    
    vector<Point2i> contours = slic.getContours();
    for( Point2i contour: contours )
        slic_contour.at<Vec3b>( contour.y, contour.x ) = Vec3b(255, 0, 255);
    
    slic_contour.copyTo( Mat(appended, regions[1]) );
    addText( appended, "Superpixels", Point( regions[1].x + 30, regions[1].y + 30 ), font );
    
    
    /* Finally draw the segmented image */
    char temp[255];
    for( int k = 0; k < k_clusters; k++ ) {
        Rect region = regions[k+2];
        image.copyTo( Mat(appended, region), mask == k );
        sprintf( temp, "Segment [%d]", k + 1 );
        addText( appended, temp, Point( region.x + 30, region.y + 30 ), font );
    }
    
    
    /* Uhh, try to fit my screen */
    Size screen_size(1366 * 0.8, 768 * 0.8);
    float resize_ratio = MIN( 1.0 * screen_size.width / appended.cols, 1.0 * screen_size.height / appended.rows  );
    
    resize( appended, appended, Size(), resize_ratio, resize_ratio );
    imshow( "", appended );
    while( waitKey(10) != 'q' );
}

int main(int argc, const char * argv[])
{

    Mat image_1 = imread( "/Users/saburookita/Sandbox/SLICSuperpixelsAndSpectralCluster/QSI4E.jpg" );
    testSpectralSegment( image_1, 400, 2, 1.0f );
    
    Mat image_2 = imread( "/Users/saburookita/Sandbox/SLICSuperpixelsAndSpectralCluster/sample1.jpg" );
    testSpectralSegment( image_2, 200, 2, 2.0f );
    
    return 0;
}

