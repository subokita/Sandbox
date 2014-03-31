//
//  Plane.cpp
//  OpenGLViewer
//
//  Created by Saburo Okita on 13/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Plane.h"
#include "Util.h"

namespace glv {
    Plane::Plane(){
        
    }
    
    bool Plane::init( float size ) {
        if( size <= 0.0 )
            return false;
        
        vector<glm::vec3> vertices = {
            glm::vec3( -1.0f,   1.0f, 0.0f ),
            glm::vec3( -1.0f,  -1.0f, 0.0f ),
            glm::vec3(  1.0f,  -1.0f, 0.0f ),
            
            glm::vec3( -1.0f,   1.0f, 0.0f ),
            glm::vec3(  1.0f,  -1.0f, 0.0f ),
            glm::vec3(  1.0f,   1.0f, 0.0f ),
            
        };
        
        for( auto& vertex: vertices )
            vertex = vertex * size;
        
        noOfElements = static_cast<int>(vertices.size());
        
        vector<glm::vec3> normals = {
            glm::vec3(  0.0f,  0.0f,  1.0f ),
            glm::vec3(  0.0f,  0.0f,  1.0f ),
            glm::vec3(  0.0f,  0.0f,  1.0f ),
            glm::vec3(  0.0f,  0.0f,  1.0f ),
            glm::vec3(  0.0f,  0.0f,  1.0f ),
            glm::vec3(  0.0f,  0.0f,  1.0f ),
        };
        
        vector<glm::vec2> uvs {
            glm::vec2( 0.0f, 1.0f ),
            glm::vec2( 0.0f, 0.0f ),
            glm::vec2( 1.0f, 0.0f ),
            
            glm::vec2( 0.0f, 1.0f ),
            glm::vec2( 1.0f, 0.0f ),
            glm::vec2( 1.0f, 1.0f ),
        };
        
        vector<glm::vec3> tangents, bitangents;
        computeTangentBasis( vertices, uvs, normals, tangents, bitangents );
        
        glGenBuffers( 5, bufferIds );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * vertices.size(), &vertices[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[1] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * normals.size(), &normals[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[2] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec2 ) * uvs.size(), &uvs[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[3] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * tangents.size(), &tangents[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[4] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * bitangents.size(), &bitangents[0], GL_STATIC_DRAW );
        
        return true;
    }
    
    void Plane::draw() {
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
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[2] );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
        
        /* Tangents */
        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[3] );
        glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
        
        /* Bitangents */
        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[4] );
        glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
        
        glDrawArrays( GL_TRIANGLES, 0, noOfElements );
    }
}