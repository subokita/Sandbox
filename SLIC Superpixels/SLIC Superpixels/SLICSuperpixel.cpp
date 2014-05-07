//
//  SLICSuperpixel.cpp
//  SLICSuperpixel Superpixels
//
//  Created by Saburo Okita on 22/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "SLICSuperpixel.h"
#include <tbb/tbb.h>
#include <numeric>

SLICSuperpixel::SLICSuperpixel() {
    
}

SLICSuperpixel::SLICSuperpixel( Mat& src, int no_of_superpixels, int m, int max_iterations ) {
    init( src, no_of_superpixels, m, max_iterations );
}

/**
 * Initialize the clusters centers to nearest lowest gradient in 3x3 neighborhood
 * and initiliaze the values for the cluster labels and the distances
 **/
void SLICSuperpixel::init(Mat& src, int no_of_superpixels, int m, int max_iterations ) {
    this->clear();
    
    /* Grid interval (S) = sqrt( N / k ) */
    this->S             = int(sqrt( (1.0 * src.rows * src.cols) / no_of_superpixels ));
    this->K             = no_of_superpixels;
    this->m             = m;
    this->maxIterations = max_iterations;
    
    cvtColor( src, image, CV_BGR2Lab );
    
    /* Initialize cluster centers Ck and move them to the lowest gradient position in 3x3 neighborhood */
    for( int y = S; y < image.rows - S / 2; y += S ) {
        for( int x = S; x < image.cols - S / 2; x += S ) {
            Point2i minimum = findLocalMinimum( image, Point2i(x, y));
            Vec3b color = image.at<Vec3b>( minimum.y, minimum.x );
            centers.push_back( ColorRep( color, minimum ) );
        }
    }
    
    /* Set labels to -1 and distances to infinity */
    clusters  = Mat( image.size(), CV_32SC1, Scalar(-1));
    distances = Mat( image.size(), CV_64FC1, Scalar(std::numeric_limits<float>::max()) );
    
    centerCounts = vector<int>( centers.size(), 0 );
}

/**
 * Clear everything
 */
void SLICSuperpixel::clear() {
    centers.clear();
    centerCounts.clear();
    
    if( !image.empty() )
        image.release();
}

/**
 * Apply the superpixel algorithm in order to obtain cluster centers for each
 * superpixel
 */
void SLICSuperpixel::generateSuperPixels() {
    /* Repeat until we hit max iterations (or certain threshold in literature) */
    for( int iter = 0; iter < this->maxIterations; iter++ ) {
        
        /* Reset distances */
        distances = Scalar(std::numeric_limits<float>::max());
        
        /* For each cluster centers Ck */
        tbb::parallel_for( 0, static_cast<int>(centers.size()), 1, [&](int k) {
            ColorRep center = centers[k];
            
            /* For each 2 x Steps region around Ck */
            for( int y = center.y - S; y < center.y + S; y++ ){
                Vec3b * ptr         = image.ptr<Vec3b>(y);
                int * clust_ptr     = clusters.ptr<int>(y);
                double * dist_ptr   = distances.ptr<double>(y);
                
                for( int x = center.x - S; x < center.x + S; x++ ){
                    if( withinRange(x, y) ){
                        Vec3b color = ptr[x];
                        
                        /* Compute and retain the smaller distance */
                        double distance = calcDistance( center, color, x, y );
                        if( distance < dist_ptr[x] ) {
                            dist_ptr[x] = distance;
                            clust_ptr[x]  = k;
                        }
                    }
                }
            }
        });
        
        centers.assign( centers.size(), ColorRep() );
        centerCounts.assign( centerCounts.size(), 0 );
        
        /* Update new cluster centers ... */
        for( int y = 0; y < image.rows; y++ ) {
            int * clust_ptr = clusters.ptr<int>(y);
            
            for( int x = 0; x < image.cols; x++ ) {
                int cluster_id = clust_ptr[x];
                if( cluster_id > -1 ) {
                    Vec3b color = image.at<Vec3b>(y, x);
                    centers[cluster_id].add( color, x, y );
                    centerCounts[cluster_id]++;
                }
            }
        }
        
        /* ... average them */
        tbb::parallel_for( 0, static_cast<int>(centers.size()), 1, [&](int i) {
            centers[i].div( centerCounts[i] );
        });
    }
    
}


vector<ColorRep> SLICSuperpixel::getCenters() {
    return vector<ColorRep>( centers );
}

/**
 * Return to cluster centers for each superpixel
 */
vector<Point2i> SLICSuperpixel::getClusterCenters() {
    vector<Point2i> result( centers.size() );
    
    tbb::parallel_for( 0, static_cast<int>(centers.size()), 1, [&](int i) {
        result[i].x = centers[i].x;
        result[i].y = centers[i].y;
    });
    
    return result;
}

/**
 * Get contour separating each clusters
 */
vector<Point2i> SLICSuperpixel::getContours() {
    const int dx[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
    const int dy[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
    
    vector<vector<bool>> taken;
    for( int y = 0; y < image.rows; y++ )
        taken.push_back ( vector<bool>( image.cols, false ) );
    
    vector<Point2i> contours;
    for( int y = 0; y < image.rows; y++ ){
        int * clust_ptr = clusters.ptr<int>(y);
        
        for( int x = 0; x < image.cols; x++ ) {
            int nr_p = 0;
            
            for(int k = 0; k < 8; k++ ) {
                int nx = x + dx[k];
                int ny = y + dy[k];
                
                if( withinRange( nx, ny ) ){
                    if( !taken[ny][nx] && clust_ptr[x] != clusters.at<int>(ny, nx) ) {
                        nr_p++;
                        
                        if( nr_p > 1 )
                            break;
                    }
                }
            }
            
            if( nr_p > 1 ) {
                contours.push_back( Point2i(x, y) );
                taken[y][x] = true;
            }
        }
    }
    
    return contours;
}


/**
 * Check if x and y are inside the image
 */
inline bool SLICSuperpixel::withinRange( int x, int y ) {
    return x >= 0 && y >= 0 && x < image.cols && y < image.rows;
}

/**
 * Find local minimum within 3x3 region from the center position
 */
Point2i SLICSuperpixel::findLocalMinimum( Mat& image, Point2i center ) {
    Point2i minimum( center.x, center.y );
    float min_gradient = std::numeric_limits<float>::max();
    
    for( int y = center.y - 1; y < center.y + 2; y++ ) {
        for( int x = center.x - 1; x < center.x + 2; x++ ) {
            Vec3b lab    = image.at<Vec3b>( y  , x   );
            Vec3b lab_dy = image.at<Vec3b>( y+1, x   );
            Vec3b lab_dx = image.at<Vec3b>( y  , x+1 );
            
            float diff = fabs( lab_dy[0] - lab[0] ) + fabs( lab_dx[0] - lab[0] );
            if( diff < min_gradient ) {
                min_gradient = diff;
                minimum.x = x;
                minimum.y = y;
            }
        }
    }
    
    return minimum;
}

/**
 * Calculate distance measure by:
 * Dlab = L2 norm( lab color space )
 * Dxy  = L2 norm( coordinates )
 * Ds   = sqrt( (Dlab / m)^2 + (Dlab / grid interval)^2 )
 */
double SLICSuperpixel::calcDistance( ColorRep& c, Vec3b& p, int x, int y ) {
    double d_lab = ( (c.l - p[0]) * (c.l - p[0])
                    +   (c.a - p[1]) * (c.a - p[1])
                    +   (c.b - p[2]) * (c.b - p[2]) );
    
    double d_xy  = ( (c.x - x) * (c.x - x)
                    +   (c.y - y) * (c.y - y)  );
    
    return sqrt( d_lab + d_xy / (S * S) * (m * m) );
}


int SLICSuperpixel::getS() {
    return this->S;
}

int SLICSuperpixel::getM() {
    return this->m;
}

/**
 * Returns a copy of clusters index mapping for each pixel
 * i.e. each cell in the matrix shows which cluster index it is
 * assigned to
 */
Mat SLICSuperpixel::getClustersIndex() {
    return clusters.clone();
}

/**
 * Return the CIELab color space image
 */
Mat SLICSuperpixel::getImage() {
    return image.clone();
}

/**
 * Recolor the cluster within the image, based on the average color within the cluster
 */
Mat SLICSuperpixel::recolor() {
    Mat temp = image.clone();
    
    vector<Vec3f> colors( centers.size() );
    
    /* Accumulate the colors for each cluster */
    for( int y = 0; y < temp.rows; y++ ) {
        int * clusters_ptr = clusters.ptr<int>(y);
        Vec3b * ptr = temp.ptr<Vec3b>(y);
        
        for( int x = 0; x < temp.cols; x++ )
            colors[clusters_ptr[x]] += ptr[x];
    }
    
    /* Get the average of the colors */
    tbb::parallel_for( 0, static_cast<int>(colors.size()), 1, [&](int i) {
        colors[i] /= centerCounts[i];
    });
    
    /* Recolor the original CIELab image with the average color for each clusters */
    tbb::parallel_for( 0, temp.rows, 1, [&](int y) {
        Vec3b * ptr = temp.ptr<Vec3b>(y);
        int * clusters_ptr = clusters.ptr<int>(y);
        
        for( int x = 0; x < temp.cols; x++ ) {
            int cluster_index = clusters_ptr[x];
            Vec3b color = colors[cluster_index];
            ptr[x] = Vec3b( color[0], color[1], color[2] );
        }
    });
    
    return temp;
}