//
//  Procrustes.h
//  Procrustes
//
//  Created by Saburo Okita on 07/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __Procrustes__Procrustes__
#define __Procrustes__Procrustes__

#include <iostream>
#include <opencv2/opencv.hpp>

class Procrustes {
public:
    cv::Mat translation;    /* Translation involved to obtain Yprime */
    cv::Mat rotation;       /* Rotation involved to obtain Yprime */
    cv::Mat Yprime;         /* The transformed point from Y to X */
    float scale;            /* Scaling invovled to obtain Yprime */
    float error;            /* Squared error of final result */
    bool scaling;
    bool bestReflection;
    
    Procrustes();
    Procrustes( bool use_scaling, bool best_reflection );
    ~Procrustes();
    
    std::vector<cv::Point2f> yPrimeAsVector();
    float procrustes( std::vector<cv::Point2f>& X, std::vector<cv::Point2f>& Y );
    float procrustes( const cv::Mat& X, const cv::Mat& Y );
    
protected:
    static inline float sumSquared( const cv::Mat& mat );
};

#endif /* defined(__Procrustes__Procrustes__) */
