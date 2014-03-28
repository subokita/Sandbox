//
//  Cube.h
//  OpenGLViewer
//
//  Created by Saburo Okita on 13/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __OpenGLViewer__Cube__
#define __OpenGLViewer__Cube__

#include <iostream>
#include "BasicShape.h"

using namespace std;

namespace glv {
    class Cube : public BasicShape {
        
        public:
            Cube();
            bool init( float size );
    };
};

#endif /* defined(__OpenGLViewer__Cube__) */
