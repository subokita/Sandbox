//
//  PatchModels.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 11/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __NonRigidFaceTracking__PatchModels__
#define __NonRigidFaceTracking__PatchModels__

#include "Header.h"
#include "PatchModel.h"
#include "MUCTLandmark.h"


class PatchModels {
public:
    Mat reference;
    vector<PatchModel> patches;
    
    void save( string filename );
    void load( string filename );
    
    int noOfPatches();
    
    vector<Point2f> applySimilarity( const Mat& S, const vector<Point2f>& points );
    Mat inverseSimilarityTransform( const Mat& S );
    Mat calcSimilarityTransform( const vector<Point2f>& pts, Mat& ref );
    
    void train( const vector<MUCTLandmark>& data, const vector<Point2f>& ref, const Size patch_size, const Size search_window_size, const bool mirror = false, const float variance = 1.0, const float lambda = 1e-6, const float mu_init = 1e-3, const int n_samples = 1000, const bool visualize = true);
    
    void visualize();
    vector<Point2f> calcPeaks( const Mat& image, const vector<Point2f>& points, const Size ssize = Size(21,21));
};

#endif /* defined(__NonRigidFaceTracking__PatchModels__) */
