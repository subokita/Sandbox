//
//  Shader.cpp
//  glfwTutorial
//
//  Created by Saburo Okita on 10/4/13.
//  Copyright (c) 2013 Saburo Okita. All rights reserved.
//


#include "Shader.h"
#include <fstream>
#include <sstream>
#include <vector>
#include "Util.h"

namespace glv {
    Shader::Shader() {

    }

    Shader::Shader( const char * vert_filename, const char * frag_filename ) {
        this->init( vert_filename, frag_filename );
    }

    Shader::~Shader() {
        if( vertexShaderId != 0 ) {
            glDetachShader( programId, vertexShaderId );
            glDeleteShader( vertexShaderId );
        }
        
        if( fragmentShaderId != 0 ) {
            glDetachShader( programId, fragmentShaderId );
            glDeleteShader( fragmentShaderId );
        }
        
        if( programId != 0 )
            glDeleteProgram( programId );
        
        uniforms.clear();
    }


    void Shader::init( const char * vert_filename, const char * frag_filename ) {
        vertexShaderId          = glCreateShader( GL_VERTEX_SHADER );
        fragmentShaderId        = glCreateShader( GL_FRAGMENT_SHADER );
        
        string vertex_code      = Shader::readShader( vert_filename );
        string fragment_code    = Shader::readShader( frag_filename );
        
        if( vertex_code.empty() || fragment_code.empty() ) {
            cerr << "Unable to open vertex or fragment shader file" << endl;
            exit( 1 );
        }
        
        const char * vcs = vertex_code.c_str();
        glShaderSource( vertexShaderId, 1, &vcs, 0 );
        glCompileShader( vertexShaderId );
        Shader::getShaderLog( vert_filename, cout, vertexShaderId );
        
        const char * fcs = fragment_code.c_str();
        glShaderSource( fragmentShaderId, 1, &fcs, 0 );
        glCompileShader( fragmentShaderId );
        Shader::getShaderLog( frag_filename, cout, fragmentShaderId );
        
        programId = glCreateProgram();
        glAttachShader( programId, vertexShaderId );
        glAttachShader( programId, fragmentShaderId );
        glLinkProgram( programId );
        Shader::getProgramLog( cout, programId );
        
        for( auto & key_value : uniforms )
            getUniform( key_value.first );
    }

    unsigned int Shader::getId() {
        return this->programId;
    }

    void Shader::bind() {
        glUseProgram( programId );
    }

    void Shader::unbind() {
        glUseProgram( 0 );
    }

    void Shader::getUniform( const string uniform_name ) {
        if( uniforms[uniform_name] != -1 )
            cout << uniform_name  << " already generated, using the old one" << endl;
        else {
            uniforms[uniform_name] = glGetUniformLocation( programId, uniform_name.c_str() );
        }
    }

    GLint Shader::getStructLocation( const char * struct_name, const char * variable ){
        stringstream ss;
        ss << struct_name << "." << variable;
        GLint location = glGetUniformLocation( programId, ss.str().c_str() );
        if( location == -1 ){
            cerr << "Unable to link to uniform variable " << ss.str() << endl;
            throw -1;
        }
        
        return location;
    }
    
    void Shader::setUniform( const char * struct_name, const char * variable, GLfloat v0 ) {
        GLint location = getStructLocation( struct_name, variable );
        glUniform1f( location, v0 );
    }
    
    void Shader::setUniform( const char * struct_name, const char * variable, glm::vec3 vector ) {
        GLint location = getStructLocation( struct_name, variable );
        glUniform3f( location, vector.x, vector.y, vector.z );
    }
    
    void Shader::setUniform( const char * struct_name, const char * variable, glm::vec4 vector ) {
        GLint location = getStructLocation( struct_name, variable );
        glUniform4f( location, vector.x, vector.y, vector.z, vector.w );
    }
    
    void Shader::setUniform( const char * uniform_name, glm::vec4 vector ) {
        if( uniforms.count( uniform_name ) == 0  ) {
            cerr << uniform_name << " was not generated previously" << endl;
            throw -1;
        }
        glUniform4f( uniforms[uniform_name], vector.x, vector.y, vector.z, vector.w );
    }

    
    void Shader::setUniform( const char * uniform_name, glm::vec3 vector ) {
        if( uniforms.count( uniform_name ) == 0  ) {
            cerr << uniform_name << " was not generated previously" << endl;
            throw -1;
        }
        glUniform3f( uniforms[uniform_name], vector.x, vector.y, vector.z );
    }

    void Shader::setUniform( const char * uniform_name, GLint v0 ) {
        if( uniforms.count( uniform_name ) == 0 ) {
            cerr << uniform_name << " was not generated previously" << endl;
            throw -1;
        }
        glUniform1i( uniforms[uniform_name], v0 );
    }

    void Shader::setUniform( const char * uniform_name, GLfloat v0 ) {
        if( uniforms.count( uniform_name ) == 0 ) {
            cerr << uniform_name << " was not generated previously" << endl;
            throw -1;
        }
        glUniform1f( uniforms[uniform_name], v0 );
    }

    void Shader::setUniform( const char * uniform_name, glm::mat3 matrix ) {
        if( uniforms.count( uniform_name ) == 0 ) {
            cerr << uniform_name << " was not generated previously" << endl;
            throw -1;
        }
        glUniformMatrix3fv( uniforms[uniform_name], 1, GL_FALSE, glm::value_ptr( matrix ));
    }

    void Shader::setUniform( const char * uniform_name, glm::mat4 matrix ) {
        if( uniforms.count( uniform_name ) == 0 ) {
            cerr << uniform_name << " was not generated previously" << endl;
            throw -1;
        }
        glUniformMatrix4fv( uniforms[uniform_name], 1, GL_FALSE, glm::value_ptr(matrix));
    }

    string Shader::readShader( const char * filename ) {
        stringstream text;
        string line;
        ifstream stream( filename );
        
        if( stream.is_open() ) {
            while( getline( stream, line ) ) {
                text << "\n" << line;
                
                vector<string> tokens;
                istringstream iss( line );
                copy( istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens) );
                if( tokens.size() >= 3 && tokens[0].compare( "uniform" ) == 0 ) {
                    string var_name     = rtrim( tokens[2] );
                    var_name            = var_name.substr( 0, var_name.length() - 1 );
                    uniforms[var_name]  = -1;
                }
            }
        }
        stream.close();
        return text.str();
    }


    void Shader::getShaderLog( const char * message, ostream& os, GLuint shader_id ) {
        int info_log_length = 0;
        glGetShaderiv( shader_id, GL_INFO_LOG_LENGTH, &info_log_length );
        
        if( info_log_length > 0 ) {
            char * info_log_buffer = new char[info_log_length];
            int info_log_read = 0;
            glGetShaderInfoLog( shader_id, info_log_length, &info_log_read, info_log_buffer );
            
            os << message << " info log: " << info_log_buffer << endl;
            
            delete [] info_log_buffer;
        }
        else {
            os << message << ": OK" << endl;
        }
    }


    void Shader::getProgramLog( ostream &os, GLuint program_id ) {
        int info_log_length = 0;
        glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &info_log_length );
        
        if( info_log_length > 0 ) {
            char * info_log_buffer = new char[info_log_length];
            int info_log_read = 0;
            glGetProgramInfoLog( program_id, info_log_length, &info_log_read, info_log_buffer );
            
            os << "Shader info log: " << info_log_buffer << endl;
            
            delete [] info_log_buffer;
        }
        else {
            //os << "Shader info log: OK" << endl;
        }
    }
};