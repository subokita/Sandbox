//
//  Plane.h
//  OpenGLViewer
//
//  Created by Saburo Okita on 13/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __OpenGLViewer__Plane__
#define __OpenGLViewer__Plane__

#include <iostream>
#include <vector>
#include "GLVHeader.h"
#include "BasicShape.h"

using namespace std;

namespace glv {
    class Plane : public BasicShape {
        
    public:
        Plane();
        bool init( float size );
        void draw();
    };
};


#endif /* defined(__OpenGLViewer__Plane__) */
