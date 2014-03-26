//
//  Viewer.h
//  OpenGLViewer
//
//  Created by Saburo Okita on 13/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __OpenGLViewer__Viewer__
#define __OpenGLViewer__Viewer__

#include <iostream>
#include "GLVHeader.h"
#include "Arcball.h"
#include "Shader.h"

namespace glv{
    class Viewer {
        public:
            Viewer();
            virtual ~Viewer();
            virtual bool init( const std::string title, GLuint window_width, GLuint window_height );
            void run();
        
        protected:
            static Viewer * instance;
            Arcball arcball;
        
            GLuint vertexArrayId;
            GLuint windowWidth;
            GLuint windowHeight;
        
            GLFWwindow * window;
            GLfloat cameraDistance;
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        
            virtual bool preLoop() = 0;
            virtual void loop( double last_time, double current_time ) = 0;
            virtual bool postLoop() = 0;
        
            /* Callback functions required by GFLW */
            virtual void mouseButtonCallback    ( GLFWwindow * window, int button, int action, int mods );
            virtual void errorCallback          ( int error, const char * desc );
            virtual void keyCallback            ( GLFWwindow *window, int key, int scancode, int action, int mod);
            virtual void frameBufferSizeCallback( GLFWwindow *window, int width, int height );
            virtual void scrollCallback         ( GLFWwindow *window, double x, double y );
            virtual void cursorCallback         ( GLFWwindow *window, double x, double y );
        
            static void mouseButtonCallbackProxy    ( GLFWwindow * window, int button, int action, int mods );
            static void errorCallbackProxy          ( int error, const char * desc );
            static void keyCallbackProxy            ( GLFWwindow *window, int key, int scancode, int action, int mod);
            static void frameBufferSizeCallbackProxy( GLFWwindow *window, int width, int height );
            static void scrollCallbackProxy         ( GLFWwindow *window, double x, double y );
            static void cursorCallbackProxy         ( GLFWwindow *window, double x, double y );
    };
};

#endif /* defined(__OpenGLViewer__Viewer__) */
