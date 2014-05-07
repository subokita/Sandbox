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
#include <tbb/tbb.h>

#include "SLICSuperpixel.h"

using namespace std;
using namespace cv;

int main(int argc, const char * argv[]) {
    Mat image = imread( "/Users/saburookita/Desktop/Blog stuff/QSI4E.jpg" );
    imshow( "Original RGB", image );
    
    tbb::tick_count begin = tbb::tick_count::now();
    
    /* Generate super pixels */
    SLICSuperpixel slic( image, 400 );
    slic.generateSuperPixels();
    
    /* Recolor based on the average cluster color */
    Mat result = slic.recolor();
    
    cout << (tbb::tick_count::now() - begin).seconds() << " seconds elapsed" << endl;
    
    cvtColor( result, result, CV_Lab2BGR );
    imshow( "Clustered color", result );
    
    /* Draw the contours bordering the clusters */
    vector<Point2i> contours = slic.getContours();
    for( Point2i contour: contours )
        image.at<Vec3b>( contour.y, contour.x ) = Vec3b(255, 0, 255);
    
    imshow( "Contours", image );
    imshow( "CIELab space", slic.getImage() );
    
    waitKey();

    return 0;
}

