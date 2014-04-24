//
//  main.cpp
//  EfficientGraphBasedImageSegmentation
//
//  Created by Saburo Okita on 23/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include "EGBS.h"

/**
 * Read more about it from the original literature:
 * http://cs.brown.edu/~pff/segment/
 */
int main(int argc, const char * argv[]) {
    Mat image = imread( "/Users/saburookita/Desktop/Blog stuff/QSI4E.jpg" );
    
    float sigma             = 0.5;      /* For internal gaussian blurring usage only */
    float threshold         = 1500;     /* Bigger threshold means bigger clusters */
    int min_component_size  = 20;       /* Weed out clusters that are smaller than this size */

    EGBS egbs;
    egbs.applySegmentation( image, sigma, threshold, min_component_size );
    
    Mat random_color  = egbs.recolor(true);
    Mat average_color = egbs.recolor(false);
    
    
    imshow( "original", image );
    imshow( "average color", average_color );
    imshow( "random color", random_color );
    waitKey();
    
    return 0;
}

