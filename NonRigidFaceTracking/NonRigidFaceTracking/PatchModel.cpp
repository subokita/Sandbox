//
//  PatchModel.cpp
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 11/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "PatchModel.h"


Size PatchModel::patchSize() {
    return P.size();
}

vector<Point2f> PatchModel::applySimilarity( const Mat& S, const vector<Point2f>& points ) {
    return vector<Point2f>();
}

Mat PatchModel::calcResponse( const Mat& image, const bool sum_to_one ) {
    Mat I = convertImage( image );
    Mat response;
    matchTemplate( I, P, response, CV_TM_CCOEFF_NORMED );
    
    if( sum_to_one ) {
        normalize( response, response, 0, 1, NORM_MINMAX );
        response /= cv::sum(response)[0];
    }
    
    return response;
}

/**
 * Convert image into CV_32F + 1.0, and apply log function onto it
 */
Mat PatchModel::convertImage( const Mat& image ) {
    Mat res;
    
    if( image.channels() == 1 ){
        if( image.type() != CV_32F )
            image.convertTo( res, CV_32F );
        else
            res = image;
    }
    else {
        if( image.channels() == 3  ){
            Mat gray;
            cvtColor( image, gray, CV_RGB2GRAY );
            gray.convertTo( res, CV_32F );
        }
    }
    res += 1.0;
    cv::log( res, res );
    
    return res;
}

void PatchModel::train( const vector<Mat>& images, const Size patch_size, const float variance, const float lambda, const float mu_init, const int n_samples, const bool visualize ) {
    int N = static_cast<int>(images.size());
    int n = patch_size.width * patch_size.height;
    
    Size window_size = images[0].size();
    
    int dx = window_size.width - patch_size.width;
    int dy = window_size.height - patch_size.height;
    
    /* F is ideal response map */
    Mat F(dy, dx, CV_32F);
    for( int y = 0; y < dy; y++ ){
        float vy = (dy - 1) / 2 - y;
        
        for( int x = 0; x < dx; x++ ){
            float vx = (dx - 1) / 2 - x;
            
            /* Use gaussian */
            F.at<float>(y, x) = exp( -0.5 * (vx * vx + vy * vy) / variance );
        }
    }
    normalize( F, F, 0, 1, NORM_MINMAX );
    
    Mat I(window_size, CV_32F);
    Mat dP( patch_size, CV_32F );
    Mat O = Mat::ones( patch_size, CV_32F ) / n;
    P = Mat::zeros( patch_size, CV_32F );

    
//    RNG rng(getTickCount());
    RNG rng;
    
    /* Apply stochastic gradient descent */
    double mu = mu_init;
    double learning_rate = pow( 1e-8 / mu_init, 1.0 / n_samples );
    
    Mat Wi( patch_size, CV_32FC1 );
    
    for( int sample = 0; sample < n_samples; sample++ ) {
        int i = rng.uniform(0,N);
        I = convertImage(images[i]);
        dP = 0.0;
        
        for(int y = 0; y < dy; y++){
            for(int x = 0; x < dx; x++){
                Mat Wi = I( Rect( Point(x, y), patch_size ) ).clone();
                
                Wi -= Wi.dot(O);
                normalize(Wi,Wi);
                dP += (F.at<float>(y,x) - P.dot(Wi))*Wi;
                
            }
        }
        
        P += mu * (dP - lambda * P);
        mu *= learning_rate;
        
        if( visualize ){
            Mat R;
            matchTemplate( I, P, R, CV_TM_CCOEFF_NORMED );
            Mat PP;
            normalize( P, PP, 0, 1, NORM_MINMAX );
            normalize(dP, dP, 0, 1, NORM_MINMAX );
            normalize( R, R, 0, 1, NORM_MINMAX );
            imshow( "P", PP );
            imshow( "dP", dP );
            imshow( "R", R );
            
            if(waitKey(10) == 27)
                break;
        }
    }
}