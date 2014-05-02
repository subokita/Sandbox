//
//  Hough.cpp
//  TestHough
//
//  Created by Saburo Okita on 27/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Hough.h"
#include <tbb/tbb.h>

Hough::Hough() {
    cosines.reserve( thetaMax );
    sines.reserve( thetaMax );

    /* Pre calc the cosines and sines */
    for( int theta = 0; theta < thetaMax; theta++ ){
        float theta_rad = theta * M_PI / 180.0;
        cosines[theta]  = cosf( theta_rad );
        sines[theta]    = sinf( theta_rad );
    }
}

Hough::~Hough() {
}

/**
 * Initialize the accumulation matrix
 **/
void Hough::init(Mat &src){
    imageSize = src.size();
    
    /* Normalized so that the matrix values are either 0 or 255 */
    Mat image;
    normalize( src, image, 0, 255, NORM_MINMAX );
    
    /* Create the accumulator matrix */
    this->rhoRange = MAX( image.cols, image.rows ) * 0.70710678; /* 0.70710678 is SQRT(2.0) / 2.0 */
    this->accum = Mat::zeros( rhoRange * 2, thetaMax, CV_32FC1 );
    
    this->centerX = image.cols / 2;
    this->centerY = image.rows / 2;
    
    /* Find all non zero pixels */
    vector<Point> non_zero_pixels;
    cv::findNonZero( image, non_zero_pixels );
    
    tbb::parallel_for_each( non_zero_pixels.begin(), non_zero_pixels.end(), [&](Point pixel){
        float dy = pixel.y - centerY;
        float dx = pixel.x - centerX;
        
        for( int theta = 0; theta < thetaMax; theta++ ) {
            float r         = dx * cosines[theta] + dy * sines[theta];
            int rho_index   = rhoRange + r;
            
            /* Increase the accumulator */
            accum.at<float>(rho_index, theta)++;
        }
    });
}

/**
 * Retrieve the accumulation matrix
 */
Mat Hough::getAccumulationMatrix( float thresh ) {
    Mat thresholded;
    accum.copyTo( thresholded, accum >= thresh );
    return thresholded;
}

/**
 * Return a pair of points that describe a line based on the given rho and theta indices
 * from the accumulation matrix
 */
pair<Point, Point> Hough::getLine( int rho_index, int theta ) {
    float rho = rho_index - rhoRange;
    
    Point point_1;
    point_1.x = 0;
    point_1.y = (rho - (point_1.x - centerX) * cosines[theta]) / sines[theta] + centerY;
    
    Point point_2;
    point_2.x = imageSize.width;
    point_2.y = (rho - (point_2.x - centerX) * cosines[theta]) / sines[theta] + centerY;
    
    return pair<Point, Point>( point_1, point_2 );
}

/**
 * Return the lines detected from the accumulation matrix
 */
vector<pair<Point, Point>> Hough::getLines( int thresh ) {
    vector<pair<Point, Point>> lines;
    if( thresh < 1 )
        return lines;
    
    Mat thresholded;
    accum.copyTo( thresholded, accum >= thresh );
    
    /* Pointer to previous, current and next rows */
    float * prev_row = thresholded.ptr<float>(0),
          * curr_row = thresholded.ptr<float>(1),
          * next_row = thresholded.ptr<float>(2);
    
    tbb::concurrent_vector<pair<Point, Point>> c_lines;
    
    for( int rho_index = 1; rho_index < thresholded.rows-1; rho_index++ ) {
        tbb::parallel_for( 1, thetaMax - 1, 1, [&]( int theta ){
            float max = curr_row[theta];
            
            if( max != 0 ) {
                /* Are there any higher peaks in 3x3 region ? */
                for (int x = -1; x < 2; x++ ) {
                    max = MAX( max, prev_row[x] );
                    max = MAX( max, curr_row[x] );
                    max = MAX( max, next_row[x] );
                }
                
                /* Only process if it's local maxima */
                if( max == curr_row[theta])
                    c_lines.push_back( getLine( rho_index, theta ) );
            }
        });
        
        /* Move each pointer to the next row */
        prev_row = curr_row;
        curr_row = next_row;
        next_row = thresholded.ptr<float>( rho_index + 1 );
    }
    
    std::copy( c_lines.begin(), c_lines.end(), back_inserter(lines ));
    
    return lines;
}