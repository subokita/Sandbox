//
//  FaceDetector.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 17/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __NonRigidFaceTracking__FaceDetector__
#define __NonRigidFaceTracking__FaceDetector__

#include "Header.h"
#include "MUCTLandmark.h"

class FaceDetector{
public:
    CascadeClassifier classifier;
    string cascadeXMLFilename;
    Mat reference;
    Vec3f detectorOffset;
    
    vector<Point2f> detect( Mat& grayscale, vector<Rect>& faces, const float scale_factor = 1.2,
                            const int min_neighbors = 4, const Size min_size = Size(30, 30));
    
    void train( vector<MUCTLandmark> data, const string cascade_xml_filename, const Mat& ref,
               const float frac = 0.8, const float scale_factor = 1.1, const int min_neighbors = 2,
               const Size min_size = Size(30, 30), bool visualize = false );
    
    void save( string filename );
    void load( string filename );
    
protected:
    bool enoughBoundedPoints( const vector<Point2f>& points, const Rect rect, const float frac );
    Point2f centerOfMass( const vector<Point2f>& points );
    float calcScale( const vector<Point2f>& points, const Point2f center );
};

#endif /* defined(__NonRigidFaceTracking__FaceDetector__) */
