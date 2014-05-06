//
//  FastSymmetryDetector.h
//  FSD
//
//  Created by Saburo Okita on 02/05/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __FSD__FastSymmetryDetector__
#define __FSD__FastSymmetryDetector__

#include <iostream>
#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>

using namespace std;
using namespace cv;


class FastSymmetryDetector {
public:
    FastSymmetryDetector( const Size image_size, const Size hough_size, const int rot_resolution = 1 );
    void vote( Mat& image, int min_pair_dist, int max_pair_dist  );
    inline void rotateEdges( vector<Point2f>& edges, int theta );
    
    Mat getAccumulationMatrix( float thresh = 0.0 );
    
    vector<pair<Point, Point>> getResult( int no_of_peaks, float threshold = -1.0f );
    pair<Point, Point> getLine( float rho, float theta );
    
private:
    vector<Mat> rotMatrices;
    Mat rotEdges;
    vector<float*> reRows;
    Mat accum;
    
    Size imageSize;
    Point2f center;
    float diagonal;
    int rhoDivision;
    int rhoMax;
    int thetaMax;
};

#endif /* defined(__FSD__FastSymmetryDetector__) */
