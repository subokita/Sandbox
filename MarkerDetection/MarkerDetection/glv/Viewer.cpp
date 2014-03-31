//
//  Viewer.cpp
//  OpenGLViewer
//
//  Created by Saburo Okita on 13/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Viewer.h"
#include <sstream>

using namespace std;

namespace glv {
    Viewer * Viewer::instance = NULL;
    
    Viewer::Viewer(){
        vertexArrayId = -1;
    }
    
    Viewer::~Viewer() {
        if( vertexArrayId != -1 )
            glDeleteVertexArrays( 1, &vertexArrayId );
        
        glfwDestroyWindow( window );
        window   = NULL;
        instance = NULL;
        glfwTerminate();
    }
  
    
    bool Viewer::init( const string title, GLuint window_width, GLuint window_height ) {
        
        if( !glfwInit() ) {
            cerr << "Unable to initialize glfw" << endl;
            return false;
        }
        
        /* Tell GLFW to use OpenGL 4.1 */
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
        glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
        glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
        
        this->windowWidth  = window_width;
        this->windowHeight = window_height;
        this->window       = glfwCreateWindow( windowWidth, windowHeight, title.c_str(), NULL, NULL );
        
        if ( !window ) {
            cerr << "Unable to create glfw window" << endl;
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent( window );
        glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );
        
        cout << "OpenGL Ver: " << glGetString( GL_VERSION ) << endl;
        
        glGenVertexArrays( 1, &vertexArrayId );
        glBindVertexArray( vertexArrayId );
        
        instance = this;
        
        
        glfwSetErrorCallback          ( Viewer::errorCallbackProxy );
        glfwSetScrollCallback         ( window, Viewer::scrollCallbackProxy );
        glfwSetKeyCallback            ( window, Viewer::keyCallbackProxy );
        glfwSetFramebufferSizeCallback( window, Viewer::frameBufferSizeCallbackProxy );
        glfwSetCursorPosCallback      ( window, Viewer::cursorCallbackProxy );
        glfwSetMouseButtonCallback    ( window, Viewer::mouseButtonCallbackProxy );
        
        arcball.init( windowWidth, windowHeight, 2.0f );
        
        cameraDistance = 5.0f;
        
        return true;
    }
    
    void Viewer::run() {
        if (!preLoop() ){
            cerr << "Pre loop function ended with failure" << endl;
            return;
        }
        
        double last_time = glfwGetTime();
        unsigned int no_frames = 0;
        unsigned int fps_int = 0;
        
        double last_frame_time = last_time;
        
        while( !glfwWindowShouldClose( window ) ) {
            double current_time = glfwGetTime();
            no_frames++;
            if( current_time - last_time >= 1.0 ) {
                fps_int = no_frames;
                no_frames = 0;
                last_time += 1.0;
            }
            
            loop(last_frame_time, current_time);
            last_frame_time = current_time;
//            
//            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
//            glViewport(0, 0, windowWidth, windowHeight );
//            
//            stringstream ss;
//            ss << "FPS: ";
//            ss << fps_int;
//            
            glfwSwapBuffers( window );
            glfwPollEvents();
        }
        
        if (!postLoop() ){
            cerr << "Post loop function ended with failure" << endl;
            return;
        }
    }
    
    
    void Viewer::errorCallback( int error, const char * desc ) {
        fputs( desc, stderr );
    }
    
    void Viewer::errorCallbackProxy( int error, const char * desc ) {
        instance->errorCallback( error, desc );
    }
    void Viewer::keyCallbackProxy(GLFWwindow *window, int key, int scancode, int action, int mod) {
        instance->keyCallback( window, key, scancode, action, mod );
    }
    void Viewer::frameBufferSizeCallbackProxy( GLFWwindow *window, int width, int height ) {
        instance->frameBufferSizeCallback( window, width, height );
    }
    void Viewer::scrollCallbackProxy( GLFWwindow *window, double x, double y ) {
        instance->scrollCallback( window, x, y );
    }
    void Viewer::cursorCallbackProxy( GLFWwindow *window, double x, double y ) {
        instance->cursorCallback( window, x, y );
    }
    void Viewer::mouseButtonCallbackProxy( GLFWwindow * window, int button, int action, int mods ) {
        instance->mouseButtonCallback( window, button, action, mods );
    }
    
    void Viewer::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mod) {
        if( action == GLFW_PRESS ) {
            switch ( key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose( window, GL_TRUE );
                    return;
                    
                default:
                    break;
            }
        }
    }
    
    void Viewer::frameBufferSizeCallback( GLFWwindow *window, int width, int height ) {
        glViewport( 0, 0, width, height );
    }
    
    void Viewer::scrollCallback( GLFWwindow *window, double x, double y ) {
        cameraDistance = glm::clamp( static_cast<float>(cameraDistance + y * 0.1), 0.1f, 20.0f);
    }
    
    
    void Viewer::mouseButtonCallback( GLFWwindow * window, int button, int action, int mods ){
        arcball.mouseButtonCallback( window, button, action, mods );
    }
    
    void Viewer::cursorCallback( GLFWwindow *window, double x, double y ) {
        arcball.cursorCallback( window, x, y );
    }
};