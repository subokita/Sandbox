//
//  SuperpixelSegmentation.cpp
//  SLICSuperpixelsAndSpectralCluster
//
//  Created by Saburo Okita on 07/05/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "SuperpixelSegmentation.h"


SuperpixelSegmentation::SuperpixelSegmentation() {
    
}

SuperpixelSegmentation::SuperpixelSegmentation( Size image_size, float sigma ) {
    this->init( image_size, sigma );
}

void SuperpixelSegmentation::init( Size image_size, float sigma ) {
    this->sigma         = sigma;
    this->clusterMask   = Mat( image_size, CV_8UC1, Scalar(0) );
}

/**
 * Create laplacian matrix out of the cluster centers of the superpixels
 * And apply eigen decomposition to obtain the eigenvectors
 *
 * The reason why those steps are here, so that later on when we apply
 * segmentation, we will just be looking at the eigenvectors and the result
 * of applying k-means on them
 */
void SuperpixelSegmentation::calculateEigenvectors( vector<ColorRep>& clusters_centers, int slic_s, int slic_m ) {
    /* Generate and retrieve the cluster centers for the super pixels */
    
    /* Create adjacency, degree, and then laplacian matrix */
    Mat adjacency   = createAdjacency( clusters_centers, slic_s, slic_m );
    Mat degree      = degreeMatrix( adjacency );
    
    Mat degree_05;
    pow( degree, -0.5, degree_05 );
    degree_05 = Mat::diag( degree_05 );
    
    Mat laplacian = Mat::diag( degree ) - adjacency;
    laplacian = (degree_05 * laplacian) * degree_05;
    
    /* Perform eigendecomposition on the laplacian */
    Mat eigenvalues;
    eigen( laplacian, eigenvalues, eigenvectors );
}

/**
 * Apply segmentation, and retrieve the mask that can be used to
 * separate the segments. Mask values range from 0 to no_of_clusters
 **/
Mat SuperpixelSegmentation::applySegmentation( int no_of_clusters, Mat& clusters_index ) {
    if( no_of_clusters < 1 )
        throw "No of clusters must be greater than 1";
    
    if( eigenvectors.empty() )
        throw "Please invoke generateSuperPixels() beforehand";
    
    /* Get the K eigenvectors whose corresponding eigenvalues are near to 0 */
    /* Since eigendecomposition in OpenCV returns result in descending order, just get the last K eigenvectors */
    Mat k_eigenvecs = eigenvectors.rowRange( eigenvectors.rows - no_of_clusters, eigenvectors.rows ).t();
    
    /* Perform k-means on the eigenvectors to get new cluster centers */
    Mat labels;
    kmeans( k_eigenvecs, no_of_clusters, labels,
            TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 1000, 1e-5), 2, KMEANS_RANDOM_CENTERS );
    
    /* Labels, transposed so that I can do label_ptr easier, not really important */
    labels = labels.t();
    int * labels_ptr = labels.ptr<int>();
    
    /* Map the superpixels cluster index to new labels, thus creating a mask for the segments */
    for( int y = 0; y < clusters_index.rows; y++ ) {
        int * cluster_ptr = clusters_index.ptr<int>(y);
        uchar * mask_ptr  = clusterMask.ptr<uchar>(y);
        
        for( int x = 0; x < clusters_index.cols; x++ )
            mask_ptr[x] = labels_ptr[ cluster_ptr[x] ];
    }
    
    return clusterMask.clone();
}

/**
 * Get cluster mask
 */
Mat SuperpixelSegmentation::getClusterMask() {
    return clusterMask.clone();
}

/**
 * Create degree matrix out from adjacency matrix
 */
Mat SuperpixelSegmentation::degreeMatrix( Mat& adjacency ) {
    int cols = adjacency.cols;
    
    Mat degree( 1, cols, CV_32FC1 );
    float * deg_ptr = degree.ptr<float>();
    
    for( int col = 0; col < cols; col++ )
        deg_ptr[col] = sum( adjacency.col( col ) )[0];
    
    return degree;
}


/**
 * Create adjacency matrix using the found cluster centers. 
 * Distance measured is basically gaussian applied to distance in CIELab + XY space.
 */
Mat SuperpixelSegmentation::createAdjacency( vector<ColorRep>& points, int slic_s, int slic_m ) {
    int size = static_cast<int>(points.size());
    Mat adjacency(size, size, CV_32FC1, Scalar(0.0));
    
    /* Refer to distance measured used in SLIC superpixels algorithm, this is the ratio so that */
    /* we can combine distance in CIELab space with cartesian coords */
    double ratio = 1.0 * (slic_m * slic_m) / (slic_s * slic_s);
    
    /* Gaussian normalization factor */
    double gauss_denominator = (2.0 * sigma * sigma);
    
    tbb::parallel_for( 0, size, 1, [&](int i) {
        float * ptr = adjacency.ptr<float>(i);
        for( int j = 0; j < size; j++ ) {
            if( i >= j )
                continue;
            
            double d_lab = points[i].colorDist( points[j] );
            double d_xy  = points[i].coordDist( points[j] );
            ptr[j] = -sqrt( d_lab + d_xy * ratio ) / gauss_denominator;
        }
    });
    
    completeSymm( adjacency );
    
    exp( adjacency, adjacency );
    adjacency = adjacency - Mat::eye(size, size, CV_32FC1 );
    
    return adjacency;
}
