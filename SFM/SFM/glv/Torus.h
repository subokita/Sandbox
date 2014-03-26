//
//  Torus.h
//  OpenGLViewer
//
//  Created by Saburo Okita on 25/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __OpenGLViewer__Torus__
#define __OpenGLViewer__Torus__

#include <iostream>
#include "BasicShape.h"

using namespace std;
namespace glv {
    class Torus : public BasicShape {
        
    public:
        Torus();
        bool init( float inner_radius, float outer_radius, unsigned int slices, unsigned int stacks );
        bool init( float size );
    };
}

#endif /* defined(__OpenGLViewer__Torus__) */
