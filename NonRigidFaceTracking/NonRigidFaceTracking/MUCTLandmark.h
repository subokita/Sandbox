//
//  MUCTLandmark.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 04/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __NonRigidFaceTracking__MUCTLandmark__
#define __NonRigidFaceTracking__MUCTLandmark__

#include "Header.h"

/**
 Just an internal representation for MUCT files (images and annotations)
 */
class MUCTLandmark {
public:
    string filename;
    string tag;
    vector<Point2f> points;
    
    MUCTLandmark();
    ~MUCTLandmark();
    
    bool initFromCSV( string csv_line );
    
    static vector<MUCTLandmark> readFromCSV( string filename );
    
    friend ostream& operator<<(ostream& os, const MUCTLandmark& obj);
};

#endif /* defined(__NonRigidFaceTracking__MUCTLandmark__) */
