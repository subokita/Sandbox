//
//  SimilarityTransform.cpp
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 11/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "SimilarityTransform.h"

/**
 * Recenter points by subtracting each of them from the mean
 */
Scalar SimilarityTransform::recenter( Mat& points ) {
    Scalar mu = cv::mean( points );
    points = points - Mat(points.size(), points.type(), mu);
    return mu;
}

/**
 * Based on Tim Cootes' "An Introduction to Active Shape Models"
 * http://personalpages.manchester.ac.uk/staff/timothy.f.cootes/papers/asm_overview.pdf 
 * Aligning two centered shapes by rotation
 */
Mat SimilarityTransform::rotateScaleAlign( const Mat& src, const Mat& dst ) {
    float d = src.dot( src );
    float a = src.dot( dst );
    
    float b = 0.0;
    for( int i = 0; i < src.rows; i++ ) {
        Point2f point1 = src.at<Point2f>(i);
        Point2f point2 = dst.at<Point2f>(i);
        b += point1.x * point2.y - point1.y * point2.x;
    }
    
    /* a = k cos theta, b = k sin theta */
    a /= d;
    b /= d;
     
    Mat R2 = (Mat_<float>(2, 2) <<
            a, -b,
            b, a);
    
    return R2;
}