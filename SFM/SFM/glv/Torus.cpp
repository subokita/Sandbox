//
//  Torus.cpp
//  OpenGLViewer
//
//  Created by Saburo Okita on 25/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Torus.h"
#include <vector>

namespace glv {
    
    Torus::Torus() {
        
    }
    bool Torus::init( float inner_radius, float outer_radius, unsigned int slices, unsigned int stacks ) {
        if( inner_radius <= 0.0f || outer_radius <= 0.0f )
            return false;
        
        if( inner_radius >= outer_radius )
            return false;
        
        float s_increment = 1.0f / slices,
              t_increment = 1.0f / stacks;
        
        
        vector<glm::vec3> vertices, normals;
        vector<glm::vec2> uvs;
        
        float center_radius = outer_radius - inner_radius;
        float torus_radius = center_radius / 2.0f;
        
        float s = 0.0f;
        for( int i = 0; i <= slices; ++i, s+= s_increment ) {
            float cos_2pi_s = cosf( 2.0f * M_PI * s );
            float sin_2pi_s = sinf( 2.0f * M_PI * s );
            
            float t = 0.0f;
            for( int j = 0; j <= stacks; ++j, t+= t_increment ){
                float cos_2pi_t = cosf( 2.0f * M_PI * t );
                float sin_2pi_t = sinf( 2.0f * M_PI * t );
                
                glm::vec3 vertex;
                vertex.x = (center_radius + torus_radius * cos_2pi_t) * cos_2pi_s;
                vertex.y = (center_radius + torus_radius * cos_2pi_t) * sin_2pi_s;
                vertex.z =                  torus_radius * sin_2pi_t;
                
                glm::vec3 normal;
                normal.x = cos_2pi_t * cos_2pi_s;
                normal.y = cos_2pi_t * sin_2pi_s;
                normal.z = sin_2pi_t;
                
                glm::vec2 uv(s, t);
                vertices.push_back( vertex );
                normals.push_back( normal );
                uvs.push_back( uv );
            }
        }
        
        
        vector<unsigned short> index( stacks * slices * 2 * 3 );
        int index_counter = 0;
        for( int i = 0; i < slices; i++ ) {
            for( int j = 0; j < stacks; j++ ) {
                unsigned int v0 = (i       * (stacks + 1)) + j;
                unsigned int v1 = ((i + 1) * (stacks + 1)) + j;
                unsigned int v2 = ((i + 1) * (stacks + 1)) + (j + 1);
                unsigned int v3 = (i       * (stacks + 1)) + (j + 1);
                
                index[index_counter++] = v0;
                index[index_counter++] = v1;
                index[index_counter++] = v2;
                
                index[index_counter++] = v0;
                index[index_counter++] = v2;
                index[index_counter++] = v3;
            }
        }
        
        glGenBuffers( 4, bufferIds );
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[0] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * vertices.size(), &vertices[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[1] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * normals.size(), &normals[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ARRAY_BUFFER, bufferIds[3] );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec2 ) * uvs.size(), &normals[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bufferIds[2] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned short ) * index.size(), &index[0], GL_STATIC_DRAW );
        
        this->noOfElements = static_cast<int>(index.size());

        
        return true ;
    }
    
    bool Torus::init( float size ) {
        return init( size / 2.0, size, 30, 15 );
    }
};