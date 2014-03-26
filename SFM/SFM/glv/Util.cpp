//
//  Util.cpp
//  GL4Demo
//
//  Created by Saburo Okita on 16/12/13.
//  Copyright (c) 2013 Saburo Okita. All rights reserved.
//

#include "Util.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <FreeImagePlus.h>

using namespace std;

void computeTangentBasis( vector<glm::vec3>& vertices, vector<glm::vec2>& uvs, vector<glm::vec3>& normals, vector<glm::vec3>& tangents, vector<glm::vec3>& bitangents ) {
    for( int i = 0; i < vertices.size(); i+= 3 ) {
        glm::vec3 & v0 = vertices[i];
        glm::vec3 & v1 = vertices[i+1];
        glm::vec3 & v2 = vertices[i+2];
        
        glm::vec2 & uv0 = uvs[i];
        glm::vec2 & uv1 = uvs[i+1];
        glm::vec2 & uv2 = uvs[i+2];
        
        glm::vec3 delta_pos_1 = v1 - v0;
        glm::vec3 delta_pos_2 = v2 - v0;
        
        glm::vec2 delta_uv_1 = uv1 - uv0;
        glm::vec2 delta_uv_2 = uv2 - uv0;
        
        float r = 1.0f / ( delta_uv_1.x * delta_uv_2.y - delta_uv_1.y * delta_uv_2.x );
        glm::vec3 tangent   = ( delta_pos_1 * delta_uv_2.y - delta_pos_2 * delta_uv_1.y ) * r;
        glm::vec3 bitangent = ( delta_pos_2 * delta_uv_2.x - delta_pos_1 * delta_uv_1.x ) * r;
        
        tangents.push_back( tangent );
        tangents.push_back( tangent );
        tangents.push_back( tangent );
        
        bitangents.push_back( bitangent );
        bitangents.push_back( bitangent );
        bitangents.push_back( bitangent );
    }
    
    for (int i = 0; i < vertices.size(); i++ ) {
        glm::vec3 & normal      = normals[i];
        glm::vec3 & tangent     = tangents[i];
        glm::vec3 & bitangent   = bitangents[i];
        
        tangent = glm::normalize( tangent - normal * glm::dot(normal, tangent) );
        
        if( glm::dot( glm::cross( normal, tangent ), bitangent ) < 0.0f )
            tangent *= -1.0f;
    }
}


GLuint loadCubeMap( vector<const char *> filenames ) {
    GLuint cube_map_id;
    
    glGenTextures(1, &cube_map_id);
    glBindTexture( GL_TEXTURE_CUBE_MAP, cube_map_id );
    
    const static GLenum cube_map_enums[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
    };
    
    int counter = 0;
    for( const char * filename: filenames ) {
        FIBITMAP * bitmap =  FreeImage_Load( FreeImage_GetFileType( filename ), filename  );
        
        glTexImage2D( cube_map_enums[counter++], 0, GL_RGB, FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*) FreeImage_GetBits( bitmap ));
        
        FreeImage_Unload( bitmap );
    }
    
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    
    return cube_map_id;
}

void* loadImage( const char * filename ) {
    FIBITMAP * bitmap =  FreeImage_Load( FreeImage_GetFileType( filename ), filename  );
    return FreeImage_GetBits( bitmap );
}

GLuint loadTexture( const char * filename )  {
    FIBITMAP * bitmap =  FreeImage_Load( FreeImage_GetFileType( filename ), filename  );
    
    GLuint texture_id;
    
    glGenTextures( 1, &texture_id );
    glBindTexture( GL_TEXTURE_2D, texture_id );
    
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*) FreeImage_GetBits( bitmap ));
    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture( GL_TEXTURE_2D, 0 );
    
    FreeImage_Unload( bitmap );
    
    return texture_id;
}

string& rtrim( string& s ) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

void print( glm::vec3 vector ) {
    cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")" << endl;
}

void print( glm::mat4 matrix ) {
    for( int j = 0; j < 4; j++ ) {
        for( int i = 0; i < 4; i++ )
            printf( "%+02.2f ", matrix[i][j] );
        printf( "\n" );
    }
}

void print( glm::mat3 matrix ) {
    for( int j = 0; j < 3; j++ ) {
        for( int i = 0; i < 3; i++ )
            printf( "%02.2f ", matrix[i][j] );
        printf( "\n" );
    }
}