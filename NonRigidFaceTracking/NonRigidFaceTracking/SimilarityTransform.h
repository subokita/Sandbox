//
//  SimilarityTransform.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 11/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __NonRigidFaceTracking__SimilarityTransform__
#define __NonRigidFaceTracking__SimilarityTransform__

#include "Header.h"

class SimilarityTransform{
public:
    static Scalar recenter( Mat& points );
    static Mat rotateScaleAlign( const Mat& src, const Mat& dst );
};

#endif /* defined(__NonRigidFaceTracking__SimilarityTransform__) */
