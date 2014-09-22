//
//  ARViewer.h
//  MarkerDetection
//
//  Created by Saburo Okita on 29/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __MarkerDetection__ARViewer__
#define __MarkerDetection__ARViewer__

#include <iostream>

#include <opencv2/opencv.hpp>

#include "glv/Viewer.h"
#include "glv/Shader.h"
#include "glv/Cube.h"
#include "glv/Sphere.h"

#include "MarkerDetector.h"
#include "Calibration.h"

class ARViewer: public glv::Viewer {
public:
    ARViewer();
    ~ARViewer();
    
protected:
    string videoFilename;
    Marker marker;
    MarkerDetector detector;
    Calibration calib;
    
    cv::VideoCapture cap;
    cv::Mat frame, prevFrame, output;
    cv::Mat rvec, tvec;
    glv::Shader backgroundShader, objectShader;
    glv::Cube cube;
    glm::mat4 ortho, CVToGLMat, modelView;
    GLuint vertexBufferId, uvBufferId, textureId;
    
    
    void drawScene( cv::Mat& frame );
    
    bool preLoop();
    void loop( double last_time, double current_time );
    bool postLoop();
    
    glm::mat4 createProjectionMatrix( Mat& cam_matrix );
    static void drawOutline( Marker& marker, cv::Mat& output, cv::Mat& rvec, cv::Mat& tvec, cv::Mat& cam_matrix, cv::Mat& dist_coeff, cv::Scalar outline_color );
    static glm::mat4 asGLMMatrix( cv::Mat& rvec, cv::Mat& tvec );
};


#endif /* defined(__MarkerDetection__ARViewer__) */
