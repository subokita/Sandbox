//
//  Header.h
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 18/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef NonRigidFaceTracking_Header_h
#define NonRigidFaceTracking_Header_h

/* A very lazy way to include headers, please don't do this in production */

/* STL headers */
#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <fstream>

/* OpenCV headers */
#include <opencv2/opencv.hpp>

/* Intel TBB headers */
#include <tbb/tbb.h>

/* Boost headers */
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

static const std::string CASCADE_FILE = "/usr/local/Cellar/opencv/2.4.7.1/share/OpenCV/haarcascades/haarcascade_frontalface_default.xml";

static const std::string DIR_PATH = "/Users/saburookita/Sandbox/NonRigidFaceTracking/muct/jpg/";

static const std::string path = "/Users/saburookita/Sandbox/NonRigidFaceTracking/muct/";

using namespace cv;
using namespace std;

#endif
