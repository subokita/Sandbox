//
//  SuperpixelSegmentation.h
//  SLICSuperpixelsAndSpectralCluster
//
//  Created by Saburo Okita on 07/05/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __SLICSuperpixelsAndSpectralCluster__SuperpixelSegmentation__
#define __SLICSuperpixelsAndSpectralCluster__SuperpixelSegmentation__

#include <iostream>
#include <opencv2/opencv.hpp>
#include <tbb/tbb.h>

#include "SLICSuperpixel.h"

using namespace std;
using namespace cv;

class SuperpixelSegmentation {
public:
    SuperpixelSegmentation();
    SuperpixelSegmentation( Size image_size, float sigma = 1.0 );
    void init( Size image_size, float sigma = 1.0 );
    
    void calculateEigenvectors( vector<ColorRep>& clusters_centers, int slic_s, int slic_m );
    Mat applySegmentation( int no_of_clusters, Mat& clusters_index );
    Mat getClusterMask();
    Mat createAdjacency( vector<ColorRep>& points, int slic_s, int slic_m );
    
protected:
    Mat degreeMatrix( Mat& adjacency );
    float sigma;
    Mat labels;
    Mat eigenvectors;
    Mat clusterMask;
};

#endif /* defined(__SLICSuperpixelsAndSpectralCluster__SuperpixelSegmentation__) */
