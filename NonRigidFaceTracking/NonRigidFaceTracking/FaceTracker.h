//
//  FaceTracker.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 18/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __NonRigidFaceTracking__FaceTracker__
#define __NonRigidFaceTracking__FaceTracker__

#include "Header.h"
#include "FaceDetector.h"
#include "ShapeModel.h"
#include "PatchModel.h"
#include "PatchModels.h"

class FaceTracker {
public:
    FaceDetector detector;
    ShapeModel shapeModel;
    PatchModels patchModels;
    
    vector<Point2f> points;
    bool tracking = false;
    Mat prevHistogram;
    
    double probSameScene( Mat& gray );
    
    
    vector<Point2f> fit( const Mat& image, const vector<Point2f>& init,
                         const Size ssize = Size(21, 21), const bool robust = false,
                         const int itol = 20, const float ftol = 1e-3 );
    
    bool track( const Mat& image, vector<Rect>& faces, const vector<Size>& levels,
               const bool robust, const int itol, const float ftol,
               const float scale_factor = 1.1, const int min_neighbors = 2, const Size min_size = Size(30, 30)  );
};


#endif /* defined(__NonRigidFaceTracking__FaceTracker__) */
