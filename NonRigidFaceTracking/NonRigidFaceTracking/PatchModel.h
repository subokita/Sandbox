//
//  PatchModel.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 11/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __NonRigidFaceTracking__PatchModel__
#define __NonRigidFaceTracking__PatchModel__

#include "Header.h"
#include "MUCTLandmark.h"


class PatchModel {
public:
    Mat P;
    Size patchSize();
    Mat convertImage( const Mat& image );
    Mat calcResponse( const Mat& image, const bool sum_to_one );
    vector<Point2f> applySimilarity( const Mat& S, const vector<Point2f>& points );
    
    
    void train( const vector<Mat>& images, const Size patch_size, const float variance, const float lambda, const float mu_init, const int n_samples, const bool visualize );
};

#endif /* defined(__NonRigidFaceTracking__PatchModel__) */
