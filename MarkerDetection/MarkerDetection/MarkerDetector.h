//
//  MarkerDetector.h
//  MarkerDetection
//
//  Created by Saburo Okita on 28/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __MarkerDetection__MarkerDetector__
#define __MarkerDetection__MarkerDetector__

#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct Marker {
    bool isMarker;
    unsigned int code;
    vector<Point2f> poly;
    Mat matrix;
};

class MarkerDetector {
public:
    MarkerDetector();
    MarkerDetector( float min_contour_size, int marker_width, int marker_height );
    
    void init( float min_contour_size, int marker_width, int marker_height );
    vector<Marker> findMarkerCandidates( Mat& frame );
    bool opticalFlowPrediction( Mat& prev_frame, Mat& frame, Marker& marker );
    
protected:
    float minContourSize;
    int markerWidth;
    int markerHeight;
    
    template<typename T>
    bool checkPoints( const vector<T>& points  );
};

#endif /* defined(__MarkerDetection__MarkerDetector__) */
