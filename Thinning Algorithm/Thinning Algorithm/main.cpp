//
//  main.cpp
//  Thinning Algorithm
//
//  Created by Saburo Okita on 22/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include "ZhangSuenThinning.h"

using namespace std;
using namespace cv;

int main(int argc, const char * argv[]) {
    namedWindow( "" );
    moveWindow( "", 0, 0 );
    
    vector<string> filenames = {
        "/Users/saburookita/Desktop/Hand #01.png",
        "/Users/saburookita/Desktop/Hand #02.png",
        "/Users/saburookita/Desktop/Demon.png"
    };
    
    
    for( string filename: filenames ) {
        Mat image = imread( filename );
        
        Mat gray = ZhangSuenThinning::apply( image );
        
        /* Since the output is in 0 and 1s, scale it to 255 so that it's visible */
        gray *= 255;
        cvtColor( gray, gray, CV_GRAY2BGR );
        
        /* Append the original image with thinned image together */
        Mat appended( gray.rows, gray.cols * 2, CV_8UC3 );
        image.copyTo( Mat(appended, Rect(0, 0, image.cols, image.rows)) );
        gray.copyTo( Mat(appended, Rect(image.cols, 0, gray.cols, gray.rows)) );
        
        resize( appended, appended, Size(), 0.75, 0.75 );
        
        imshow(  "", appended );
        waitKey();
    }
    
    return 0;
}

