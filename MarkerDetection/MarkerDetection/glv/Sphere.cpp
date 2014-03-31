//
//  Sphere.cpp
//  OpenGLViewer
//
//  Created by Saburo Okita on 19/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Sphere.h"
#include <vector>

namespace glv {
    Sphere::Sphere() {
        
    }
    
    
    bool Sphere::init( unsigned int slices, unsigned int parallels, float size ) {
        if( size <= 0.0f )
            return false;
        
        float angle_step = (M_PI * 2.0) / slices ;
        
        vector<glm::vec3> vertices, normals;
        vector<glm::vec2> uvs;
        
        for( int i = 0; i < parallels + 1; i++ ) {
            for( int j = 0; j < slices + 1; j++ ) {
                glm::vec3 vertex;
                vertex.x = sinf( angle_step * i ) * sinf( angle_step * j );
                vertex.y = cosf( angle_step * i );
                vertex.z = sinf( angle_step * i ) * cosf( angle_step * j );
                
                glm::vec3 normal = glm::normalize(glm::vec3( vertex ));
                glm::vec2 uv( j * 1.0 / slices, 1.0 - i * 1.0 / parallels );
                
                uvs.push_back( uv );
                vertices.push_back( vertex );
                normals.push_back( normal );
            }
        }
        
        std::transform( vertices.begin(), vertices.end(), vertices.begin(),
                       [&](glm::vec3 vertex) {
                           return vertex * size;
                       }
        );
        
        vector<unsigned short> index( slices * parallels * 6 );
        int index_counter = 0;
        for( int i = 0; i < parallels; i++ ) {
            for( int j = 0; j < slices; j++ ) {
                index[index_counter++] = i       * (slices + 1) + j;
                index[index_counter++] = (i + 1) * (slices + 1) + j;
                index[index_counter++] = (i + 1) * (slices + 1) + (j + 1);
                
                
                index[index_counter++] = i       * (slices + 1) + j;
                index[index_counter++] = (i + 1) * (slices + 1) + (j + 1);
                index[index_counter++] = i       * (slices + 1) + (j + 1);
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
        
        return true;
    }
    
    bool Sphere::init(float size){
        if( size <= 0.0f )
            return false;
        
        this->init( 30, 15, size );
        
        return true;
    }
    

};