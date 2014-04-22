//
//  ZhangSuenThinning.cpp
//  Zhang-Suen-Thinning
//
//  Created by Saburo Okita on 22/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "ZhangSuenThinning.h"
#include <numeric>

#ifdef USE_TBB
#include <tbb/tbb.h>
#endif

using namespace std;
using namespace cv;

/**
 * Preprocess image before applying thinning, by converting the image into grayscale
 * and thresholding it using Otsu's algorithm so that the end matrix is just 0 and 1s
 */
Mat ZhangSuenThinning::preprocess( Mat& image ) {
    Mat gray;
    if( image.channels() == 1 )
        gray = image.clone();
    else
        cvtColor( image, gray, CV_BGR2GRAY );
    
    threshold( gray, gray, 0, 1, CV_THRESH_OTSU | CV_THRESH_BINARY_INV );
    return gray;
}

#ifdef USE_TBB
/**
 * Apply Zhang Suen's thinning algorithm, this version is parallelized using Intel TBB
 */
Mat ZhangSuenThinning::apply(Mat &image){
    Mat gray = preprocess( image );
    
    tbb::concurrent_vector<Point2i> update_list_1 = {Point2i(-1, -1)},
                                    update_list_2 = {Point2i(-1, -1)};
    
    while( !update_list_1.empty() || !update_list_2.empty() ) {
        /* 1st subiteration */
        update_list_1.clear();
        tbb::parallel_for( 1, gray.rows -1, 1, [&](int y) {
            tbb::parallel_for( 1, gray.cols -1, 1, [&](int x) {
                vector<int> p = getNeighbors( gray, x, y );
                if(checkCondition( p ))
                    update_list_1.push_back( Point2i(x, y) );
            });
        });
        
        /* Update offending points to zero */
        tbb::parallel_for_each( update_list_1.begin(), update_list_1.end(), [&](Point2i point) {
            gray.data[ point.y * gray.cols + point.x ] = 0;
        });
        
        
        /* 2nd subiteration */
        update_list_2.clear();
        tbb::parallel_for( 1, gray.rows -1, 1, [&](int y) {
            tbb::parallel_for( 1, gray.cols -1, 1, [&](int x) {
                vector<int> p = getNeighbors( gray, x, y );
                if(checkCondition( p ))
                    update_list_2.push_back( Point2i(x, y) );
            });
        });
        
        /* Update offending points to zero */
        tbb::parallel_for_each( update_list_2.begin(), update_list_2.end(), [&](Point2i point) {
            gray.data[ point.y * gray.cols + point.x ] = 0;
        });
    }
    
    return gray;
}

#else
/**
 * Apply Zhang Suen's thinning algorithm, this version is the non-parallelized version
 */
Mat ZhangSuenThinning::apply(Mat &image){
    Mat gray = preprocess( image );
    
    vector<Point2i> update_list_1 = {Point2i(-1, -1)},
                    update_list_2 = {Point2i(-1, -1)};
    
    while( !update_list_1.empty() || !update_list_2.empty() ) {
        /* 1st subiteration */
        update_list_1.clear();
        for(int y = 1; y < gray.rows - 1; y++) {
            for(int x = 1; x < gray.cols - 1; x++) {
                vector<int> p = getNeighbors( gray, x, y );
                if(checkCondition( p ))
                    update_list_1.push_back( Point2i(x, y) );
            }
        }
        
        /* Update offending points to zero */
        for( Point2i point: update_list_1 )
            gray.data[ point.y * gray.cols + point.x ] = 0;
        
        
        /* 2nd subiteration */
        update_list_2.clear();
        for(int y = 1; y < gray.rows - 1; y++) {
            for(int x = 1; x < gray.cols - 1; x++) {
                vector<int> p = getNeighbors( gray, x, y );
                if(checkCondition( p ))
                    update_list_2.push_back( Point2i(x, y) );
            }
        }
        
        /* Update offending points to zero */
        for( Point2i point: update_list_2 )
            gray.data[ point.y * gray.cols + point.x ] = 0;
    }
    
    return gray;
}
#endif

/**
 * Count number of non zero neighbors around P1
 */
inline int ZhangSuenThinning::countNonZeroNeighbors( vector<int>& neighbors ){
    const static int init = 0;
    return std::accumulate( neighbors.begin() + 1, neighbors.end() - 1, init );
}


/**
 * Count number of transition patterns around P1
 */
inline int ZhangSuenThinning::countTransitionPatterns( vector<int>& neighbors ){
    int count = 0;
    for( int i = 1; i < 9; i++ ) {
        if( neighbors[i+1] - neighbors[i] == 1 )
            count++;
    }
    return count;
}

/**
 * Check if the given list of 'neighbor' pixels follow the zhang suen thinning condition
 **/
inline bool ZhangSuenThinning::checkCondition( vector<int>& p ) {
    if( p[0] == 1                           && /* Only consider if P1 is 1 */
       ( p[1] * p[3] * p[5] == 0 )          && /* if P2 * P4 * P6 == 0 */
       ( p[3] * p[5] * p[7] == 0 )          && /* if P4 * P6 * P8 == 0 */
       ( countTransitionPatterns(p) == 1 ) ) {
        
        int non_zeros = countNonZeroNeighbors( p );
        if (non_zeros >= 2 && non_zeros <= 6)  /* if 2 <= B(P1) <= 6 */
            return true;
    }
    return false;
}

/**
 * For a 3 x 3 sub matrix that has the following arrangement
 * P9 P2 P3
 * P8 P1 P4
 * P7 P6 P5
 * 
 * returns a vector consisting of {P1, P2, P3, P4, P5, P6, P7, P8, P9, P2}
 */
inline vector<int> ZhangSuenThinning::getNeighbors( Mat& mat, int x, int y ){
    uchar * row1 = mat.ptr<uchar>(y - 1);
    uchar * row2 = mat.ptr<uchar>(y    );
    uchar * row3 = mat.ptr<uchar>(y + 1);
    
    vector<int> neighbors = {
        /* P1  , P2     , P3       , P4       , P5       , P6     , P7       , P8       , P9       , P2 */
        row2[x], row1[x], row1[x+1], row2[x+1], row3[x+1], row3[x], row3[x-1], row2[x-1], row1[x-1], row1[x]
    };
    
    return neighbors;
}