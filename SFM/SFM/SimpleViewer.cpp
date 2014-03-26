//
//  SimpleViewer.cpp
//  SFM
//
//  Created by Saburo Okita on 26/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "SimpleViewer.h"

SimpleViewer::SimpleViewer(){
    
}

SimpleViewer::~SimpleViewer(){
    
}

void SimpleViewer::setVertexData(std::vector<glm::vec3> &points) {
    noOfPoints = static_cast<int>(points.size() );
    
    glGenBuffers( 1, &vertexBufferId );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBufferId );
    glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3) * points.size(), &points[0], GL_STATIC_DRAW );
}

bool SimpleViewer::preLoop(){
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );
    
    shader.init("objectPass.vsh", "objectPass.fsh" );
    
    float scale = 10.0f;
    model       = glm::scale(-scale, scale, -scale) * glm::mat4(1.0);
    projection  = glm::perspective(70.0f, 4.0f/3.0f, 0.1f, 100.0f );
    view        = glm::lookAt( glm::vec3(0.0f, 0.0f, cameraDistance), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f) );
    
    shader.bind();
    
    sphere.init( 1.0f );
    
    return true;
}

void SimpleViewer::loop( double last_time, double current_time ){
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glm::mat4 rotated_view = view * arcball.createViewRotationMatrix();
    
    glm::vec3 translation_vec( rotated_view[3][0], rotated_view[3][1], rotated_view[3][2] );
    translation_vec = glm::normalize( translation_vec ) * cameraDistance;
    
    rotated_view[3][0] = translation_vec.x;
    rotated_view[3][1] = translation_vec.y;
    rotated_view[3][2] = translation_vec.z;
    
    glm::mat4 mvp = projection * rotated_view * model;
    shader.setUniform( "mvp", mvp );
    
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBufferId );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
    
    glDrawArrays( GL_POINTS, 0, noOfPoints);
}

bool SimpleViewer::postLoop() {
    shader.unbind();
    
    return true;
}