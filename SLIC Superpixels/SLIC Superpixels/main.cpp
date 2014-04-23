//
//  main.cpp
//  SLICSuperpixel Superpixels
//
//  Created by Saburo Okita on 22/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "SLICSuperpixel.h"

using namespace std;
using namespace cv;

int main(int argc, const char * argv[]) {
    Mat image = imread( "/Users/saburookita/Desktop/Blog stuff/QSI4E.jpg" );
    imshow( "Original RGB", image );
    
    /* Generate super pixels */
    SLICSuperpixel slic( image, 400 );
    slic.generateSuperPixels();
    imshow( "CIELab space", slic.getImage() );
    
    /* Recolor based on the average cluster color */
    Mat result = slic.recolor();
    cvtColor( result, result, CV_Lab2BGR );
    imshow( "Clustered color", result );
    
    /* Draw the contours bordering the clusters */
    vector<Point2i> contours = slic.getContours();
    for( Point2i contour: contours )
        image.at<Vec3b>( contour.y, contour.x ) = Vec3b(255, 0, 255);
    
    imshow( "Contours", image );
    
    waitKey();

    return 0;
}

