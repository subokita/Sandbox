//
//  Calibration.h
//  MarkerDetection
//
//  Created by Saburo Okita on 01/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __MarkerDetection__Calibration__
#define __MarkerDetection__Calibration__

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Calibration {
public:
    Mat camMatrix;
    Mat distCoeff;
    vector<Mat> rvecs;
    vector<Mat> tvecs;
    Size patternSize;
    Size imageSize;
    int imageCount;
    
    
    Calibration();
    Calibration( Size pattern_size );
    ~Calibration();
    
    void calibrateFromVideo( string filename );
    void save( string filename );
    static Calibration load( string filename );
};

#endif /* defined(__MarkerDetection__Calibration__) */
