//
//  MUCTLandmark.cpp
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 04/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "MUCTLandmark.h"

MUCTLandmark::MUCTLandmark() {
    
}

MUCTLandmark::~MUCTLandmark() {
    points.clear();
}


bool MUCTLandmark::initFromCSV( string csv_line ) {
    namespace bs = boost;
    static bs::char_separator<char> separator(",");
    bs::tokenizer<bs::char_separator<char> > tokenizer( csv_line, separator );
    bs::tokenizer<bs::char_separator<char>>::iterator itr = tokenizer.begin();
    
    filename    = string(*itr++);
    tag         = string(*itr++);
    
    if( filename[1] == 'r' )
        return false;
    
    if ( filename[filename.length() - 4] == 'e' || filename[filename.length() - 4] == 'd' )
        return false;
    
    
    while ( itr != tokenizer.end() ) {
        float x = atof( string( *itr++ ).c_str() );
        float y = atof( string( *itr++ ).c_str() );
        points.push_back( Point2f( x, y ) );
    }
    
    return true;
}

vector<MUCTLandmark> MUCTLandmark::readFromCSV( string filename ) {
    vector<MUCTLandmark> landmarks;
    
    fstream file( filename.c_str(), ios::in );
    
    if( file ) {
        string line;
        getline(file, line);
        
        while( getline(file, line) ) {
            MUCTLandmark muct;
            if ( muct.initFromCSV( line ) )
                landmarks.push_back( muct );
        }
    }
    
    int n = static_cast<int>(landmarks[0].points.size());
    int N = static_cast<int>(landmarks.size());
    
    for( int i = 1; i < N; i++ )
        n = MAX( n, static_cast<int>(landmarks[i].points.size()) );
    
    for( int i = 0; i < landmarks.size(); i++ ) {
        if( landmarks[i].points.size() != n ){
            landmarks.erase( landmarks.begin() + i );
            i--;
        }
        else {
            int j = 0;
            for( ; j < n; j++ ) {
                if( landmarks[i].points[j].x <= 0 || landmarks[i].points[j].y <= 0 )
                    break;
            }
            
            if( j < n ) {
                landmarks.erase( landmarks.begin() + i );
                i--;
            }
        }
    }
    
    return landmarks;
}


ostream& operator<<( ostream& os, const MUCTLandmark& obj ) {
    os << obj.filename << " " << obj.tag << endl;
    os << "[";
    for ( Point2f point : obj.points )
        os << point << ", ";
    os << "]";
    return os;
}
