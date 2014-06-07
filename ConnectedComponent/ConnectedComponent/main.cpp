//
//  main.cpp
//  ConnectedComponent
//
//  Created by Saburo Okita on 06/06/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "ConnectedComponent.h"

using namespace std;
using namespace cv;

const static vector<Vec3b> COLORMAP = {
    Vec3b(  0,   0,   0),
    Vec3b(255,   0,   0),
    Vec3b(  0, 255,   0),
    Vec3b(  0,   0, 255),
    Vec3b(255, 255,   0),
    Vec3b(  0, 255, 255),
    Vec3b(255,   0, 255),
    Vec3b(255, 255, 255),
};

/* First test is based on the wikipedia example image */
/* You can find it here: http://en.wikipedia.org/wiki/Connected-component_labeling */
void test1() {
    Mat image = (Mat_<uchar>( 9, 17 ) <<
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
                 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0,
                 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
                 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0,
                 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0,
                 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0,
                 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
    
    /* Apply connected component labelling */
    ConnectedComponent conn_comp;
    Mat labels = conn_comp.apply( image );
    
    /* Recolor the labels */
    Mat colored( image.size(), CV_8UC3, Scalar::all(0) );
    for( int y = 0; y < labels.rows; y++ ) {
        Vec3b * ptr     = colored.ptr<Vec3b>(y);
        int * label_ptr = labels.ptr<int>(y);
        
        for( int x = 0; x < labels.cols; x++ )
            ptr[x] = COLORMAP[ label_ptr[x] % COLORMAP.size() ];
    }
    
    
    resize( colored, colored, Size(), 30.0, 30.0, CV_INTER_NN );
    
    /* Convert our original image into appropriate size and type */
    image = image * 255;
    resize( image, image, Size(), 30.0, 30.0, CV_INTER_NN );
    cvtColor( image, image, CV_GRAY2BGR );
    
    /* Append the two images together side by side */
    Mat appended( image.rows, image.cols * 2, CV_8UC3 );
    image.copyTo  ( Mat( appended, Rect(0, 0, image.cols, image.rows) ) );
    colored.copyTo( Mat( appended, Rect(image.cols, 0, image.cols, image.rows) ) );
    
    imshow("", appended);
    while( waitKey(10) != 'q' );
}

void test2() {
    Mat image = imread( "/Users/saburookita/Sandbox/ConnectedComponent/Example 1.png", CV_LOAD_IMAGE_GRAYSCALE );
    
    /* Binary threshold and invert it, since it's black on white background */
    Mat binary;
    threshold( image, binary, 200.0, 1.0, CV_THRESH_BINARY_INV );
    
    ConnectedComponent conn_comp;
    Mat labels = conn_comp.apply( binary );

    /* Recolor the labels */
    Mat colored( image.size(), CV_8UC3, Scalar::all(0) );
    for( int y = 0; y < labels.rows; y++ ) {
        Vec3b * ptr     = colored.ptr<Vec3b>(y);
        int * label_ptr = labels.ptr<int>(y);
        
        for( int x = 0; x < labels.cols; x++ )
            ptr[x] = COLORMAP[ label_ptr[x] % COLORMAP.size() ];
    }
    
    
    vector<ComponentProperty> props = conn_comp.getComponentsProperties();
    for( ComponentProperty prop : props )
        cout << prop << endl;
    
    cvtColor( image, image, CV_GRAY2BGR );
    
    /* Append the two images together side by side */
    Mat appended( image.rows, image.cols * 2, CV_8UC3 );
    image.copyTo  ( Mat( appended, Rect(0, 0, image.cols, image.rows) ) );
    colored.copyTo( Mat( appended, Rect(image.cols, 0, image.cols, image.rows) ) );
    
    imshow( "", appended );
    while( waitKey(10) != 'q' );
}

int main(int argc, const char * argv[])
{
    test1();
    test2();
    
    return 0;
}

