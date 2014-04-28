//
//  Hough.h
//  TestHough
//
//  Created by Saburo Okita on 27/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __TestHough__Hough__
#define __TestHough__Hough__

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Hough {
public:
    Hough();
    ~Hough();
    void init( Mat& src );
    vector<pair<Point, Point>> getLines( int thresh );
    pair<Point, Point> getLine( int rho_index, int theta );
    Mat getAccumulationMatrix( float thresh = 0.0 );
    
protected:
    float centerX, centerY;
    int rhoRange;
    Size imageSize;
    Mat accum;
    const int thetaMax = 90 * 2;
    vector<float> cosines, sines;
};

#endif /* defined(__TestHough__Hough__) */
