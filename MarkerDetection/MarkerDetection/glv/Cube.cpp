//
//  Cube.cpp
//  OpenGLViewer
//
//  Created by Saburo Okita on 13/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Cube.h"
#include <vector>

namespace glv {
    Cube::Cube(){
        
    }
    
    
    bool Cube::init( float size ) {
        if( size <= 0.0 )
            return false;
        
        vector<glm::vec3> vertices = {
            /* Bottom */
            glm::vec3( -1.0f, -1.0f, -1.0f ),
            glm::vec3( -1.0f, -1.0f, +1.0f ),
            glm::vec3( +1.0f, -1.0f, +1.0f ),
            glm::vec3( +1.0f, -1.0f, -1.0f ),
            
            /* Top */
            glm::vec3( -1.0f, +1.0f, -1.0f ),
            glm::vec3( -1.0f, +1.0f, +1.0f ),
            glm::vec3( +1.0f, +1.0f, +1.0f ),
            glm::vec3( +1.0f, +1.0f, -1.0f ),
            
            /* Back */
            glm::vec3( -1.0f, -1.0f, -1.0f ),
            glm::vec3( -1.0f, +1.0f, -1.0f ),
            glm::vec3( +1.0f, +1.0f, -1.0f ),
            glm::vec3( +1.0f, -1.0f, -1.0f ),
            
            /* Front */
            glm::vec3( -1.0f, -1.0f, +1.0f ),
            glm::vec3( -1.0f, +1.0f, +1.0f ),
            glm::vec3( +1.0f, +1.0f, +1.0f ),
            glm::vec3( +1.0f, -1.0f, +1.0f ),
            
            /* Left */
            glm::vec3( -1.0f, -1.0f, -1.0f ),
            glm::vec3( -1.0f, -1.0f, +1.0f ),
            glm::vec3( -1.0f, +1.0f, +1.0f ),
            glm::vec3( -1.0f, +1.0f, -1.0f ),
            
            /* Right */
            glm::vec3( +1.0f, -1.0f, -1.0f ),
            glm::vec3( +1.0f, -1.0f, +1.0f ),
            glm::vec3( +1.0f, +1.0f, +1.0f ),
            glm::vec3( +1.0f, +1.0f, -1.0f ),
        };
        
        
        /* Resize them */
        if( size != 1.0f ) {
            std::transform( vertices.begin(), vertices.end(), vertices.begin(),
                           [&]( glm::vec3 vertex ){
                               return vertex * size;
                           });
        }
        
        /* 36 indices */
        const static vector<unsigned short> index = {
            0, 2, 1, 0, 3, 2,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11,
            12, 15, 14, 12, 14, 13,
            16, 17, 18, 16, 18, 19,
            20, 23, 22, 20, 22, 21
        };
        
        
        noOfElements = static_cast<int>(index.size());
        
        
        const static vector<glm::vec3> normals = {
            glm::vec3( 0.0f, -1.0f, 0.0f ),
            glm::vec3( 0.0f, -1.0f, 0.0f ),
            glm::vec3( 0.0f, -1.0f, 0.0f ),
            glm::vec3( 0.0f, -1.0f, 0.0f ),
            
            glm::vec3( 0.0f, +1.0f, 0.0f ),
            glm::vec3( 0.0f, +1.0f, 0.0f ),
            glm::vec3( 0.0f, +1.0f, 0.0f ),
            glm::vec3( 0.0f, +1.0f, 0.0f ),
            
            glm::vec3( 0.0f, 0.0f, -1.0f ),
            glm::vec3( 0.0f, 0.0f, -1.0f ),
            glm::vec3( 0.0f, 0.0f, -1.0f ),
            glm::vec3( 0.0f, 0.0f, -1.0f ),
            
            glm::vec3( 0.0f, 0.0f, +1.0f ),
            glm::vec3( 0.0f, 0.0f, +1.0f ),
            glm::vec3( 0.0f, 0.0f, +1.0f ),
            glm::vec3( 0.0f, 0.0f, +1.0f ),
            
            glm::vec3( -1.0f, 0.0f, 0.0f ),
            glm::vec3( -1.0f, 0.0f, 0.0f ),
            glm::vec3( -1.0f, 0.0f, 0.0f ),
            glm::vec3( -1.0f, 0.0f, 0.0f ),
            
            glm::vec3( +1.0f, 0.0f, 0.0f ),
            glm::vec3( +1.0f, 0.0f, 0.0f ),
            glm::vec3( +1.0f, 0.0f, 0.0f ),
            glm::vec3( +1.0f, 0.0f, 0.0f ),
        };
        
        
        const static vector<glm::vec2> uvs {
            glm::vec2( 0.0f, 0.0f ),
            glm::vec2( 0.0f, 1.0f ),
            glm::vec2( 1.0f, 1.0f ),
            glm::vec2( 1.0f, 0.0f ),
            
            glm::vec2( 0.0f, 1.0f ),
            glm::vec2( 0.0f, 0.0f ),
            glm::vec2( 1.0f, 0.0f ),
            glm::vec2( 1.0f, 1.0f ),
            
            glm::vec2( 1.0f, 0.0f ),
            glm::vec2( 1.0f, 1.0f ),
            glm::vec2( 0.0f, 1.0f ),
            glm::vec2( 0.0f, 0.0f ),
            
            glm::vec2( 0.0f, 0.0f ),
            glm::vec2( 0.0f, 1.0f ),
            glm::vec2( 1.0f, 1.0f ),
            glm::vec2( 1.0f, 0.0f ),
            
            glm::vec2( 0.0f, 0.0f ),
            glm::vec2( 1.0f, 0.0f ),
            glm::vec2( 1.0f, 1.0f ),
            glm::vec2( 0.0f, 1.0f ),
            
            glm::vec2( 1.0f, 0.0f ),
            glm::vec2( 0.0f, 0.0f ),
            glm::vec2( 0.0f, 1.0f ),
            glm::vec2( 1.0f, 1.0f ),
        };
        
        glGenBuffers( 4, bufferIds );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * vertices.size(), &vertices[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[1] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * normals.size(), &normals[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferIds[2] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned short ) * index.size(), &index[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[3] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec2 ) * uvs.size(), &uvs[0], GL_STATIC_DRAW );
        
        return true;
    }
}