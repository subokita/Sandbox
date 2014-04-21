//
//  ShapeModel.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 10/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __NonRigidFaceTracking__ShapeModel__
#define __NonRigidFaceTracking__ShapeModel__

#include "Header.h"
#include "MUCTLandmark.h"


class ShapeModel {
public:
    Mat parameterVec; // parameter vector
    Mat shapeBasis; // shape basis
    Mat paramVariance; // parameter variance
    
    int noOfPoints();
    void calcParams( const vector<Point2f>& points, const Mat weight = Mat(), const float c_factor = 3.0 );
    vector<Point2f> calcShape( const float width = 100.0 );
    void clamp(const float c);
    void setIdentityParams();
    
    Mat procrustes(vector<Mat>& points_matrices, const int itol = 100, const float ftol = 1e-6 );
    Mat gramSchmidt( const Mat& A );
    Mat flatten( Mat& mat );
    Mat calcRigidBasis( Mat& X );
    void train( const vector<MUCTLandmark>& landmarks, const int kmax = 10, const float frac = 0.95 );
    
    void save( string filename );
    void load( string filename );
    
    float calcScale(const Mat &X, const float width);
    
    void draw_shape(Mat &img, const vector<Point2f> &q );
    void draw_string(Mat img, const string text);
    void visualize();
};

#endif /* defined(__NonRigidFaceTracking__ShapeModel__) */
