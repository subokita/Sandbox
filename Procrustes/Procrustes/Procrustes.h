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
    
    std::vector<std::vector<cv::Point2f>> generalizedProcrustes( std::vector<std::vector<cv::Point2f>>& X, std::vector<cv::Point2f>& mean_shape, const int itol = 1000, const float ftol = 1e-6 );
    std::vector<cv::Mat> generalizedProcrustes( std::vector<cv::Mat>& X, cv::Mat& mean_shape, const int itol = 1000, const float ftol = 1e-6 );
    
protected:
    static inline float sumSquared( const cv::Mat& mat );
    std::vector<cv::Mat> recenter( const std::vector<cv::Mat>& X );
    std::vector<cv::Mat> normalize( const std::vector<cv::Mat>& X );
    std::vector<cv::Mat> align( const std::vector<cv::Mat>& X, cv::Mat& mean_shape );
    
};

#endif /* defined(__Procrustes__Procrustes__) */
