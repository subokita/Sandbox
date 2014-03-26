//
//  BasicShape.cpp
//  OpenGLViewer
//
//  Created by Saburo Okita on 13/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "BasicShape.h"


namespace glv {
    BasicShape::BasicShape(){
    }
    
    BasicShape::~BasicShape() {
        glDeleteBuffers( 6, bufferIds );
    }
    
    bool BasicShape::init( float size ) {
        noOfElements = 0;
        return false;
    }
    
    void BasicShape::draw(){
        /* Vertices */
        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[0] );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
        
        /* Normals */
        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[1] );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
        
        /* UVs */
        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[3] );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
        
        /* Indices */
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferIds[2] );
        glDrawElements( GL_TRIANGLES, noOfElements, GL_UNSIGNED_SHORT, (void*) 0);
    }
}