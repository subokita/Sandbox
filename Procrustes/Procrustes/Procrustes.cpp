//
//  Procrustes.cpp
//  Procrustes
//
//  Created by Saburo Okita on 07/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Procrustes.h"
#include <random>

using namespace std;
using namespace cv;


Procrustes::Procrustes() :
    scale(1.0f),
    error(0.0f),
    scaling( true ),
    bestReflection( true )
{
    
}

Procrustes::Procrustes( bool use_scaling, bool best_reflection ) :
    scale(1.0f),
    error(0.0f),
    scaling( use_scaling ),
    bestReflection( best_reflection )
{
    
}

Procrustes::~Procrustes(){
}

/**
 * Convert YPrime back from Mat to vector<Point2f>
 * convenience method.
 */
vector<Point2f> Procrustes::yPrimeAsVector() {
    vector<Point2f> result;
    Yprime.copyTo( result );
    return result;
}

/**
 * Sum of squared of the 2 channels matrix
 */
float Procrustes::sumSquared( const Mat& mat ) {
    Mat temp;
    pow( mat, 2.0, temp );
    Scalar temp_scalar = sum( temp );
    return temp_scalar[0] + temp_scalar[1];
}

float Procrustes::procrustes(vector<Point2f> &X, vector<Point2f> &Y) {
    return procrustes( Mat(X), Mat(Y));
}

/**
 * Perform procrustes analysis / superimpose (?)
 */
float Procrustes::procrustes( const Mat& X, const Mat& Y ){
    /* Recenter the points based on their mean ... */
    Scalar mu_x = cv::mean(X);
    Mat X0      = X - Mat(X.size(), X.type(), mu_x);
    
    Scalar mu_y = cv::mean(Y);
    Mat Y0      = Y - Mat(Y.size(), Y.type(), mu_y);
    
    /* ... and normalize them */
    float ss_X      = sumSquared( X0 );
    float norm_X    = sqrt( ss_X );
    X0              /= norm_X;
    
    float ss_Y      = sumSquared( Y0 );
    float norm_Y    = sqrt( ss_Y );
    Y0              /= norm_Y;
    
    
    /* Pad with zeros is Y has less points than X */
    if( Y.rows < X.rows )
        vconcat( Y0, Mat::zeros( X.rows - Y.rows, 1, Y.type()), Y0 );
    
    
    /* Perform SVD */
    Mat A = X0.reshape(1).t() * Y0.reshape(1);
    Mat U, s, Vt;
    SVDecomp( A, s, U, Vt );
    
    Mat V           = Vt.t();
    this->rotation  = V * U.t();
    
    if( !bestReflection ) {
        bool have_reflection = determinant( this->rotation ) < 0;
        if( bestReflection != have_reflection ) {
            V.colRange( V.cols-1, V.cols) *= -1;
            s.rowRange( s.rows-1, s.rows) *= -1;
            this->rotation = V * U.t();
        }
    }
    
    /* Rotate Y0 first */
    Mat rotated_Y0;
    cv::transform( Y0, rotated_Y0, rotation.t() );
    
    /* Trace of eigenvalues is basically the scale */
    float trace_TA = sum( s )[0];
    
    if( scaling ) {
        scale   = trace_TA * norm_X / norm_Y;
        error   = 1 - trace_TA * trace_TA;
        Yprime  = norm_X * trace_TA * rotated_Y0 + mu_x;
    }
    else {
        error   = 1 + ss_Y / ss_X - 2 * trace_TA * norm_X / norm_Y;
        Yprime  = norm_Y * rotated_Y0 + mu_x;
    }
    
    if( Y.rows < X.rows )
        rotation = rotation.rowRange(0, Y.rows );
    
    translation = Mat(1, 1, CV_32FC2, mu_x).reshape(1) - scale * Mat(1, 1, CV_32FC2, mu_y).reshape(1) * rotation;
    
    return error;
}
