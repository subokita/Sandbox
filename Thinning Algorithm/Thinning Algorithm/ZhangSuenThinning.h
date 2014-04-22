//
//  ZhangSuenThinning.h
//  Zhang-Suen-Thinning
//
//  Created by Saburo Okita on 22/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

/**
 * An implementation of Zhang-Suen's Fast thinning algorithm
 * "A Fast Parallel Algorithm for Thinning Digital Patterns"
 * http://www-prima.inrialpes.fr/perso/Tran/Draft/gateway.cfm.pdf"
 **/

#ifndef __Zhang_Suen_Thinning__ZhangSuenThinning__
#define __Zhang_Suen_Thinning__ZhangSuenThinning__

#include <iostream>
#include <opencv2/opencv.hpp>

#define USE_TBB

class ZhangSuenThinning {
public:
    static cv::Mat apply( cv::Mat& image );
    
protected:
    static cv::Mat preprocess( cv::Mat& image );
    static inline int countNonZeroNeighbors( std::vector<int>& neighbors );
    static inline int countTransitionPatterns( std::vector<int>& neighbors );
    static inline bool checkCondition( std::vector<int>& p );
    static inline std::vector<int> getNeighbors( cv::Mat& mat, int x, int y );
};

#endif /* defined(__Zhang_Suen_Thinning__ZhangSuenThinning__) */
