//
//  FastSymmetryDetector.cpp
//  FSD
//
//  Created by Saburo Okita on 02/05/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "FastSymmetryDetector.h"

#define within( val, bottom, top ) ( val > bottom && val < top )

FastSymmetryDetector::FastSymmetryDetector( const Size image_size, const Size hough_size, const int rot_resolution ) {
    this->imageSize     = image_size;
    this->center        = Point2f( imageSize.width - 1.0, imageSize.height - 1.0 ) * 0.5;
    this->diagonal      = hypotf( imageSize.width, imageSize.height );
    this->rhoDivision   = diagonal;
    this->rhoMax        = hough_size.width;
    this->thetaMax      = hough_size.height;
    
    rotMatrices.resize( thetaMax, Mat(2, 2, CV_32FC1) );
    
    float thetaIncDeg = 180.0f / thetaMax;
    float half_theta_max = thetaMax * 0.5f;
    
    /* Pre calculate rotation matrices from -90 deg to 90 deg (actually to 89 deg) */
    for( int t = 0; t < thetaMax; t++ ){
        double angle = thetaIncDeg * ( t - half_theta_max );
        Mat rotation = getRotationMatrix2D( Point2f(0.0, 0.0), angle, 1.0);
        rotation.convertTo( rotation, CV_32FC1 );
        
        rotMatrices[t] = Mat( rotation, Rect(0, 0, 2, 2) );
        rotMatrices[t].row(0) *= 0.5;
    }
    
    accum       = Mat::zeros( thetaMax + 2, rhoMax, CV_32FC1 );
    rotEdges    = Mat::zeros( rhoDivision, diagonal, CV_32FC1 );
    reRows.resize( rhoDivision );
}

/**
 * Rotate each edge for a given theta, and append them into rotated edges matrix
 */
void FastSymmetryDetector::rotateEdges( vector<Point2f>& edges, int theta ) {
    /* Get the cos and sin values from our pre-calculated rotation matrices */
    float r0 = rotMatrices[theta].at<float>(0, 0);
    float r1 = rotMatrices[theta].at<float>(0, 1);
    float r2 = rotMatrices[theta].at<float>(1, 0);
    float r3 = rotMatrices[theta].at<float>(1, 1);
    
    /* Reset our row pointers to start of each row in rotated edges matrix */
    tbb::parallel_for( 0, rhoDivision, 1, [&](int i) {
        reRows[i] = rotEdges.ptr<float>(i);
    });
    
    float half_diag = cvRound(diagonal) * 0.5;
    float fourth_rho = rhoMax * 0.25;
    
    /* Now append the corresponding rho values of the rotated edges ot rotated edges matrix */
    for( Point2f& edge: edges ) {
        int rho = r2 * edge.x + r3 * edge.y + half_diag;
        *(reRows[rho]++) = r0 * edge.x + r1 * edge.y + fourth_rho;
    }
}

/**
 * Create the hough accumulation matrix, and vote for each pair of symmetrical edges
 */
void FastSymmetryDetector::vote( Mat& image, int min_pair_dist, int max_pair_dist ) {
    float min_dist = min_pair_dist * 0.5;
    float max_dist = max_pair_dist * 0.5;
    
    /* Make sure that we reset the accumulation matrix and rotated edges matrix */
    accum    = Scalar::all(0);
    rotEdges = Scalar::all(0);
    
    /* Find all the pixels of the edges */
    vector<Point> temp_edges;
    findNonZero( image, temp_edges );
    
    /* Translate them in relation to center of the image */
    vector<Point2f> edges;
    for( Point point: temp_edges )
        edges.push_back( Point2f( point.x - center.x, point.y - center.y ) );
    
    /* For each degree of rotation */
    for( int t = 0; t < thetaMax; t++ ) {
        float * accum_ptr = accum.ptr<float>(t);
        
        /* Rotate edge to that degree */
        rotateEdges( edges, t );
        
        for( int i = 0; i < rhoDivision; i++ ) {
            float * col_start   = rotEdges.ptr<float>(i);
            float * col_end     = reRows[i];
            
            /* Ignore edges that have smaller number of pairings */
            if( (col_end - col_start) <= 1 ) {
                continue;
            }
            
            /* Vote for Hough matrix */
            for( float * x0 = col_start; x0 != col_end - 1; x0++ ) {
                for( float * x1 = x0 + 1; x1 != col_end; x1++ ) {
                    float dist = fabs( *x1 - *x0 );

                    if( dist > max_dist || dist < min_dist )
                        break;
                    
                    int rho_index = static_cast<int>(*x0 + *x1);
                    accum_ptr[rho_index]++;
                }
            }
        }
    }
}


/**
 * Retrieve the accumulation matrix
 */
Mat FastSymmetryDetector::getAccumulationMatrix( float thresh ) {
    Mat thresholded;
    accum.copyTo( thresholded, accum >= thresh );
    return thresholded.t();
}



/**
 * Find the lines that fit the symmetrical object from the calculated Hough accumulation matrix
 **/
vector<pair<Point, Point>> FastSymmetryDetector::getResult(int no_of_peaks, float threshold ) {
    vector<pair<Point, Point>> result;
    
    /* Make sure that we have appropriate peaks */
    no_of_peaks = MAX( 0, no_of_peaks );
    
    /* Pre-set the size of the neighbors */
    int rho_neighbors   = rhoMax / 20.0f,
        theta_neighbors = thetaMax / 20.0f;
    
    /* Create a mask to avoid searching peaks around the padding, and for everything that's below the specified
     threshold */
    Mat mask( accum.size(), CV_8UC1, Scalar(255) );
    mask.row(0)             = Scalar(0);
    mask.row(mask.rows - 1) = Scalar(0);
    mask = mask & (accum >= threshold);
    
    Mat temp = accum.clone();
    
    float half_rho_max   = rhoMax * 0.5f;
    float half_theta_max = thetaMax * 0.5f;
    
    for( int peak = 0; peak < no_of_peaks; peak++ ) {
        /* Find the peak from the Hough accumulation matrix */
        double max_val;
        Point max_loc;
        minMaxLoc( temp, NULL, &max_val, NULL, &max_loc, mask );
        
        int rho_index   = max_loc.x;
        int theta_index = max_loc.y;
        
        if( !within(rho_index, 0, rhoMax - 1))
            break;
        
        if( !within(theta_index, 0, thetaMax))
            break;
        
        /* Convert from Hough space back to x-y space */
        float rho   = (rho_index - half_rho_max + 0.5f) * (diagonal / (rhoMax - 1.0f));
        float theta = (theta_index - half_theta_max - 1.0f) * (M_PI / thetaMax);
        result.push_back( getLine(rho, theta ));
        
        /* Try to zero out the peak and the neighborhood of the peak, so that */
        /* we can move on to find the second highest peak */
        int r0 = MAX( 0, rho_index - rho_neighbors );
        int r1 = MIN( rhoMax - 1, rho_index + rho_neighbors );
        int t0 = theta_index - theta_neighbors;
        int t1 = theta_index + theta_neighbors;
        
        if( t0 <= 0 || t1 >= thetaMax + 1 ) {
            /* Handles the edge case that wraps around the matrix */
            Rect neighbor_1, neighbor_2;
            if( t0 <= 0 ) {
                neighbor_1 = Rect( r0, 0, r1 - r0, t1 );
                r1 = rhoDivision - r1;
                neighbor_2 = Rect( r1, t0 + thetaMax - 1, r1 - r0, thetaMax + 1 );
            }
            
            if( t1 >= thetaMax + 1 ) {
                neighbor_1 = Rect( r0, t0, r1 - r0, thetaMax + 1 );
                neighbor_2 = Rect( r0, 0, r1 - r0, t1 - (thetaMax + 1) );
            }
            
            rectangle( temp, neighbor_1, Scalar(0), CV_FILLED );
            rectangle( temp, neighbor_2, Scalar(0), CV_FILLED );
        }
        else {
            rectangle( temp, Rect(r0, t0, r1 - r0, t1 - t0), Scalar(0), CV_FILLED );
        }
    }
    
    
    return result;
}


/**
 * Return a pair of points that describe the line based on the given
 * rho and theta in the Hough space
 */
pair<Point, Point> FastSymmetryDetector::getLine( float rho, float theta ) {
    float cos_theta = cosf( theta );
    float sin_theta = sinf( theta );
    
    float x_r = center.x + rho * cos_theta;
    float y_r = center.y + rho * sin_theta;
    
    vector<float> d(4, std::numeric_limits<float>::max());
    if( sin_theta != 0.0 ) {
        d[0] = x_r / sin_theta;
        d[1] = (x_r - imageSize.width + 1) / sin_theta;
    }
    if( cos_theta != 0.0 ) {
        d[2] = -y_r / cos_theta;
        d[3] = (imageSize.height - 1 - y_r) / cos_theta;
    }
    
    float min_d = std::numeric_limits<float>::max();
    for( float val : d ) {
        if (val > 0)
            min_d = MIN( min_d, val );
    }
    
    Point p0( -min_d * sin_theta + x_r, min_d * cos_theta + y_r );
    
    d.assign(4, std::numeric_limits<float>::max());
    if( sin_theta != 0.0 ) {
        d[0] = -x_r / sin_theta;
        d[1] = (imageSize.width - x_r - 1) / sin_theta;
    }
    if( cos_theta != 0.0 ) {
        d[2] = y_r / cos_theta;
        d[3] = (1 + y_r - imageSize.height) / cos_theta;
    }
    min_d = std::numeric_limits<float>::max();
    for( float val : d ) {
        if (val > 0)
            min_d = MIN( min_d, val );
    }
    
    Point p1( min_d * sin_theta + x_r, -min_d * cos_theta + y_r );
    
    return pair<Point, Point> { p0, p1, };
}