//
//  Shader.h
//  glfwTutorial
//
//  Created by Saburo Okita on 10/4/13.
//  Copyright (c) 2013 Saburo Okita. All rights reserved.
//

#ifndef __glfwTutorial__Shader__
#define __glfwTutorial__Shader__

#include <iostream>
#include <map>

#include "GLVHeader.h"

using namespace std;

namespace glv {
    /**
     * A helper class to parse shader files, and acts as a facade(?) pattern
     */
    class Shader {
        
    private:
        GLuint vertexShaderId;
        GLuint fragmentShaderId;
        GLuint programId;
        map<string, int> uniforms;
        
        void getUniform( const string uniform_name );
        string readShader( const char * filename );
        static void getShaderLog( const char * message, ostream& os, GLuint shader_id );
        static void getProgramLog( ostream& os, GLuint program_id );
        GLint getStructLocation( const char * struct_name, const char * variable );
        
    public:
        Shader();
        Shader( const char * vert_filename, const char * frag_filename );
        ~Shader();
        
        void init( const char * vert_filename, const char * frag_filename );
        void bind();
        void unbind();
        unsigned int getId();
        
        GLint getAttribute( const char * attribute_name );
        void setUniform( const char * uniform_name, GLint v0 );
        void setUniform( const char * uniform_name, GLfloat v0 );
        void setUniform( const char * uniform_name, glm::mat3 matrix );
        void setUniform( const char * uniform_name, glm::mat4 matrix );
        void setUniform( const char * uniform_name, glm::vec3 vector );
        void setUniform( const char * uniform_name, glm::vec4 vector );
        void setUniform( const char * struct_name, const char * variable, GLfloat v0 );
        void setUniform( const char * struct_name, const char * variable, glm::vec3 vector );
        void setUniform( const char * struct_name, const char * variable, glm::vec4 vector );
        
    };
};

#endif /* defined(__glfwTutorial__Shader__) */
