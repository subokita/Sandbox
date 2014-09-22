//
//  main.cpp
//  MarkerDetection
//
//  Created by Saburo Okita on 27/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//


#include <iostream>
#include <opencv2/opencv.hpp>

#include "MarkerDetector.h"
#include "ARViewer.h"
#include "Calibration.h"

using namespace std;
using namespace cv;


int main(int argc, const char * argv[]) {
    ARViewer viewer;
    if( viewer.init( "", 540, 960 )) {
        viewer.run();
    }
    
    return 0;
}
