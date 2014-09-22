//
//  Calibration.cpp
//  MarkerDetection
//
//  Created by Saburo Okita on 01/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "Calibration.h"
//#define SHOW_CALIB_IMAGES

Calibration::Calibration(){}

Calibration::Calibration( Size pattern_size ) :
    patternSize( pattern_size ){
}

Calibration::~Calibration() {
    
}


void Calibration::calibrateFromVideo( string filename ) {
    VideoCapture cap( filename );
    Mat frame;
    
    vector<vector<Point2f>> found_centers;
    
#ifdef SHOW_CALIB_IMAGES
    vector<Mat> frames;
    namedWindow("");
    moveWindow("", 0, 0);
#endif
    
    int counter = 0;
    
    while( true ) {
        cap.set( CV_CAP_PROP_POS_MSEC, counter++ * 500 );
        cap >> frame;
        if( frame.empty() )
            break;
        
        resize( frame, frame, Size(), 0.5f, 0.5f );
        flip( frame.t(), frame, 1 );
        
        imageSize = frame.size();
        
        vector<Point2f> centers;
        bool found = findCirclesGrid(frame, patternSize, centers, CALIB_CB_ASYMMETRIC_GRID );
        if( found ) {
            found_centers.push_back( centers );
            
#ifdef SHOW_CALIB_IMAGES
            drawChessboardCorners( frame, patternSize, centers, found );
            imshow( "", frame );
            waitKey(10);
            frames.push_back( frame );
#endif
        }
    }
    
    imageCount = static_cast<int>(found_centers.size());
    
    vector<vector<Point3f> > pattern_points( 1 );
    float square_size = 1.0;
    for( int row = 0; row < patternSize.height; row++ )
        for( int col = 0; col < patternSize.width; col++ )
            pattern_points[0].push_back(Point3f(float((2 * col + row % 2)*square_size), float(row*square_size), 0));
    
    pattern_points.resize( imageCount, pattern_points[0] );
    
    double reproj_error = calibrateCamera( pattern_points, found_centers, imageSize, camMatrix, distCoeff, rvecs, tvecs );
    cout << "Reprojection error: " << reproj_error << endl;
    
    
#ifdef SHOW_CALIB_IMAGES
    for( int i = 0; i < imageCount; i++ ) {
        frame = frames[i];
        Mat projected_output;
        projectPoints( pattern_points[i], rvecs[i], tvecs[i], camMatrix, distCoeff, projected_output );
        
        drawChessboardCorners( frame, patternSize, projected_output, true );
        imshow( "", frame );
        waitKey(100);
    }
#endif
}

void Calibration::save( string filename ) {
    FileStorage fs( filename, FileStorage::WRITE );

    fs << "camMatrix"   << camMatrix;
    fs << "distCoeff"   << distCoeff;
    fs << "patternSize" << patternSize;
    fs << "imageSize"   << imageSize;
    fs << "imageCount"  << imageCount;;
    fs << "rvecs"       << rvecs;
    fs << "tvecs"       << tvecs;
    
    fs.release();
}

Calibration Calibration::load( string filename ) {
    Calibration calib;
    
    FileStorage fs( filename, FileStorage::READ );

    fs["camMatrix"   ] >> calib.camMatrix;
    fs["distCoeff"   ] >> calib.distCoeff;
    fs["patternSize" ] >> calib.patternSize;
    fs["imageSize"   ] >> calib.imageSize;
    fs["imageCount"  ] >> calib.imageCount;;
    fs["rvecs"       ] >> calib.rvecs;
    fs["tvecs"       ] >> calib.tvecs;
    
    fs.release();
    
    return calib;
}
