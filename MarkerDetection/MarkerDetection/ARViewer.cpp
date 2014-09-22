//
//  ARViewer.cpp
//  MarkerDetection
//
//  Created by Saburo Okita on 29/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "ARViewer.h"

#include <thread>
#include <chrono>

#include "glv/Util.h"


using namespace std;
using namespace cv;

ARViewer::ARViewer() {
    detector.init( 100.0f, 6, 6 );
}

ARViewer::~ARViewer() {
    
}

glm::mat4 ARViewer::createProjectionMatrix( Mat& cam_matrix ) {
    float alpha    = calib.camMatrix.at<float>(0, 0);
    float beta     = calib.camMatrix.at<float>(1, 1);
    float cx       = calib.camMatrix.at<float>(0, 2);
    float cy       = calib.camMatrix.at<float>(1, 2);
    float near     = 0.1f;
    float far      = 1000.0f;
    glm::mat4 projection(0.0f);
    
//    /* 1st col */
//    projection[0][0] = alpha / cx;
//    
//    /* 2nd col */
//    projection[1][1] = beta / cy;
//    
//    /* 3rd col */
//    projection[2][2] = -(far + near) / (far - near);
//    projection[2][3] = -1.0f;
//    
//    /* 4th col */
//    projection[3][2] = -(2.0f * far * near) / (far - near );
    
    /* 1st col */
    projection[0][0] = 2.0f * alpha / windowWidth;
    
    /* 2nd col */
    projection[1][1] = 2.0f * beta / windowHeight;
    
    /* 3rd col */
    projection[2][0] =  (2.0 * cx / windowWidth) - 1.0f;
    projection[2][1] =  (2.0 * cy / windowHeight) - 1.0f;
    projection[2][2] = -(far + near) / (far - near);
    projection[2][3] = -1.0f;
    
    /* 4th col */
    projection[3][2] = -(2.0f * far * near) / (far - near );
    
    
    return projection;
}

bool ARViewer::preLoop(){
    calib = Calibration::load("/Users/saburookita/Desktop/calib.yml");
    
    calib.camMatrix.convertTo( calib.camMatrix, CV_32FC1 );
    calib.distCoeff.convertTo( calib.distCoeff, CV_32FC1 );
    
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    
    backgroundShader.init( "/Users/saburookita/Sandbox/MarkerDetection/MarkerDetection/shaders/backgroundPass.vsh",
                           "/Users/saburookita/Sandbox/MarkerDetection/MarkerDetection/shaders/backgroundPass.fsh" );
    
    
    objectShader.init( "/Users/saburookita/Sandbox/MarkerDetection/MarkerDetection/shaders/objectPass.vsh",
                       "/Users/saburookita/Sandbox/MarkerDetection/MarkerDetection/shaders/objectPass.fsh" );
    
    videoFilename = "/Users/saburookita/Sandbox/MarkerDetection/IMG_0316.MOV";
    
    /* Open our video */
    cap.open(videoFilename);
    
    /* Background scene is a normal ortographic projection */
    ortho = glm::ortho( 0.0f, windowWidth * 1.0f, 0.0f, windowHeight * 1.0f );
    
    /* Prepare to draw the background scene */
    glGenBuffers( 1, &vertexBufferId );
    glGenBuffers( 1, &uvBufferId );
    
    vector<glm::vec3> vertices = {
        glm::vec3(  0.0, 0.0, 0.0 ),
        glm::vec3(  windowWidth, 0.0, 0.0 ),
        glm::vec3(  0.0,  windowHeight, 0.0 ),
        glm::vec3(  windowWidth, windowHeight, 0.0 ),
    };
    
    vector<glm::vec2> uvs = {
        glm::vec2(  0.0,  1.0 ),
        glm::vec2(  1.0,  1.0 ),
        glm::vec2(  0.0,  0.0 ),
        glm::vec2(  1.0,  0.0 ),
    };
    
    glBindBuffer( GL_ARRAY_BUFFER, vertexBufferId );
    glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec3 ), &vertices[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, uvBufferId );
    glBufferData( GL_ARRAY_BUFFER, uvs.size() * sizeof( glm::vec2 ), &uvs[0], GL_STATIC_DRAW );
    
    /* Prepare the texture */
    glGenTextures( 1, &textureId );
    glBindTexture( GL_TEXTURE_2D, textureId );
    
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, 0 );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glShadeModel( GL_SMOOTH );
    
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    
    cube.init( 0.5f );
    
    modelView   = glm::mat4( 1.0 );
    projection  = createProjectionMatrix( calib.camMatrix );
    
    CVToGLMat = glm::mat4( 1.0 );
    CVToGLMat[1][1] = -1.0f;
    CVToGLMat[2][2] = -1.0f;
    
    return true;
}


void ARViewer::loop( double last_time, double current_time ) {
    glClear( GL_COLOR_BUFFER_BIT);
    
    cap >> frame;
    
    if( frame.empty() ) {
        cap.open( videoFilename );
        cap >> frame;
    }
    
    resize( frame, frame, Size(), 0.5f, 0.5f );
    flip( frame.t(), frame, 1 );

    output = frame.clone();
    
    cvtColor( frame, frame, CV_BGR2GRAY );
    
    
    bool found = false;
    Scalar outline_color(255, 0, 0);
    
    /* Find candidate markers */
    vector<Marker> candidates = detector.findMarkerCandidates( frame );
    if( !candidates.empty() ) {
        marker          = candidates[0];
        found           = true;
        outline_color   = Scalar(255, 0, 0);
    }
    else {
        /* If we can't find marker from usual method, resort to optical flow */
        if( !marker.poly.empty() ) {
            found           = detector.opticalFlowPrediction( prevFrame, frame, marker );
            outline_color   = Scalar(0, 0, 255);
        }
        else
            marker.poly.clear();
    }
    
    if( !marker.poly.empty() ) {
        static float w = 0.5f;
        static vector<Point3f> target_corners = {
            Point3f( -w, -w, 0.0 ),
            Point3f(  w, -w, 0.0 ),
            Point3f(  w,  w, 0.0 ),
            Point3f( -w,  w, 0.0 ),
        };

        
        Mat rvec, tvec;
        solvePnP( target_corners, marker.poly, calib.camMatrix, calib.distCoeff, rvec, tvec );

        /* Convert from rodrigues' rotation vector to rotation matrix */
        Rodrigues( rvec, rvec );

        glm::mat4 rt = asGLMMatrix( rvec, tvec );
        
        modelView = ( CVToGLMat * rt );
        drawOutline( marker, output, rvec, tvec, calib.camMatrix, calib.distCoeff, outline_color );
    }
    
    prevFrame = frame.clone();
    
    drawScene( output );
    
    objectShader.bind();
    glm::mat4 mvp = projection * modelView;
    objectShader.setUniform( "light_position_w", glm::vec3(3.0, 7.0, 1.0) );
    objectShader.setUniform( "mv", modelView );
    objectShader.setUniform( "mvp", mvp );
    
    cube.draw();
    
    objectShader.unbind();

    
    if(waitKey(20) == 27 ) {
        glfwSetWindowShouldClose( window, true );
    }
    
    this_thread::sleep_for( chrono::milliseconds(20) );
}
bool ARViewer::postLoop() {

    return true;
}

void ARViewer::drawScene( Mat& image ) {
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, windowWidth, windowHeight, GL_BGR, GL_UNSIGNED_BYTE, image.ptr() );
    
    backgroundShader.bind();
    backgroundShader.setUniform( "ortho", ortho );
    
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBufferId );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
    
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, uvBufferId );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0 );
    
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    
    glDisableVertexAttribArray( 0 );
    glDisableVertexAttribArray( 1 );
    
    backgroundShader.unbind();
}

void ARViewer::drawOutline( Marker& marker, Mat& output, Mat& rvec, Mat& tvec, Mat& cam_matrix, Mat& dist_coeff, Scalar outline_color ) {
    static vector<Point3f> target_corners = {
        Point3f( -0.5, -0.5, 0.0 ),
        Point3f(  0.5, -0.5, 0.0 ),
        Point3f(  0.5,  0.5, 0.0 ),
        Point3f( -0.5,  0.5, 0.0 ),
        Point3f( -0.5, -0.5, -0.5 ),
    };
    
    /* Check if it's projecting back to correct points */
    vector<Point2f> points;
    projectPoints( target_corners, rvec, tvec, cam_matrix, dist_coeff, points );
    
    line( output, points[0], points[1], Scalar(0, 255, 0), 5 );
    line( output, points[1], points[2], outline_color, 2 );
    line( output, points[2], points[3], outline_color, 2 );
    line( output, points[3], points[0], outline_color, 5 );
    line( output, points[4], points[0], Scalar(0, 255, 255), 5 );
    
    /* Might as well, show on the left hand corner the marker that we've seen */
    float pscale = 15.0f;
    int rows = marker.matrix.rows;
    int cols = marker.matrix.cols;
    Mat temp( rows * pscale, cols * pscale, CV_8UC1, Scalar(0) );
    for( int y = 0; y < rows; y++ ) {
        uchar * ptr = marker.matrix.ptr(y);
        for( int x = 0; x < cols; x++ ) {
            temp.rowRange(y * pscale, y * pscale+pscale).colRange( x * pscale, x * pscale + pscale ) = ptr[x];
        }
    }
    
    vector<Mat> out = {temp, temp, temp};
    merge( out, temp );
    temp.copyTo( Mat(output, Rect(0, 0, temp.cols, temp.rows)) );
}


glm::mat4 ARViewer::asGLMMatrix( cv::Mat& rvec, cv::Mat& tvec ) {
    glm::mat4 glm_matrix(1.0);
    
    for( int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++)
            glm_matrix[i][j] = static_cast<float>(rvec.at<double>(j, i));
        glm_matrix[3][i] = static_cast<float>(tvec.at<double>(i));
    }
    
    return glm_matrix;
}

