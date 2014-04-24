/*
 Copyright (C) 2006 Pedro Felzenszwalb
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

//
//  EGBS.h
//  EfficientGraphBasedImageSegmentation
//
//  Created by Saburo Okita on 23/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __EfficientGraphBasedImageSegmentation__EGBS__
#define __EfficientGraphBasedImageSegmentation__EGBS__

#include <iostream>
#include <opencv2/opencv.hpp>
#include "DisjointSetForest.h"

using namespace cv;
using namespace std;


class EGBS {
public:
    EGBS();
    ~EGBS();
    
    int applySegmentation( Mat& image, float sigma, float threshold, int min_component_size );
    Mat recolor( bool random_color = false );
    int noOfConnectedComponents();
    
protected:
    Mat image;
    Size imageSize;
    DisjointSetForest forest;
    inline float diff( Mat& rgb, int x1, int y1, int x2, int y2 );
};

#endif /* defined(__EfficientGraphBasedImageSegmentation__EGBS__) */
