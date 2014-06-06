//
//  ConnectedComponent.cpp
//  RobustTextDetection
//
//  Created by Saburo Okita on 06/06/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "ConnectedComponent.h"

using namespace std;
using namespace cv;

ConnectedComponent::ConnectedComponent(){
}

ConnectedComponent::~ConnectedComponent(){
}

/**
 * Apply connected component labelling
 * it only works for maximum 255 connected components
 * and currently treat black color as background
 */
Mat ConnectedComponent::apply( const Mat& image ) {
    CV_Assert( image.type() == CV_8UC1 );
    
    Mat result = image.clone();

    /* First pass, labelling the regions incrementally */
    nextLabel = 1;
    vector<uchar> linked(255);

    uchar * prev_ptr = NULL;
    for( int y = 0; y < result.rows; y++ ) {
        uchar * curr_ptr = result.ptr<uchar>(y);
        
        for( int x = 0; x < result.cols; x++ ) {
            
            if( curr_ptr[x] != 0 ) {
                vector<uchar> neighbors = getNeighbors( curr_ptr, prev_ptr, x, y, result.cols );
                
                if( neighbors.empty() ) {
                    curr_ptr[x] = nextLabel;
                    nextLabel++;
                    
                    CV_Assert( nextLabel < 255 );
                }
                else {
                    /* Use the minimum label out from the neighbors */
                    int min_index = (int) (min_element( neighbors.begin(), neighbors.end() ) - neighbors.begin());
                    curr_ptr[x]   = neighbors[min_index];
                    
                    for( uchar neighbor: neighbors )
                        disjointUnion( curr_ptr[x], neighbor, linked );
                }
            }
        }
        prev_ptr = curr_ptr;
    }
    
    /* Second pass merge the equivalent labels */
    nextLabel = 1;
    vector<uchar> temp, labels_set(255);
    for( int y = 0; y < result.rows; y++ ) {
        uchar * curr_ptr = result.ptr<uchar>(y);
        
        for( int x = 0; x < result.cols; x++ ) {
            if( curr_ptr[x] != 0 ) {
                curr_ptr[x] = disjointFind( curr_ptr[x], linked, labels_set );
                temp.push_back( curr_ptr[x] );
            }
        }
    }
    
    
    labels.clear();
    if( !temp.empty() ) {
        std::sort( temp.begin(), temp.end() );
        std::unique_copy( temp.begin(), temp.end(), std::back_inserter( labels ) );
    }
    
    return result;
}

/**
 * Returns the number of connected components found
 */
int ConnectedComponent::getComponentsCount() {
    return static_cast<int>(labels.size());
}


/**
 * Disjoint set union function, taken from
 * https://courses.cs.washington.edu/courses/cse576/02au/homework/hw3/ConnectComponent.java
 */
void ConnectedComponent::disjointUnion( uchar a, uchar b, vector<uchar>& parent  ) {
    while( parent[a] > 0 )
        a = parent[a];
    while( parent[b] > 0 )
        b = parent[b];
    
    if( a != b ) {
        if( a < b )
            parent[a] = b;
        else
            parent[b] = a;
    }
}

/**
 * Disjoint set find function, taken from
 * https://courses.cs.washington.edu/courses/cse576/02au/homework/hw3/ConnectComponent.java
 */
uchar ConnectedComponent::disjointFind( uchar a, vector<uchar>& parent, vector<uchar>& labels ) {
    while( parent[a] > 0 )
        a = parent[a];
    if( labels[a] == 0 )
        labels[a] = nextLabel++;
    return labels[a];
}

/**
 * Get the labels of 8 point neighbors from the given pixel
 *   | 2 | 3 | 4 |
 *   | 1 | 0 | 5 |
 *   | 8 | 7 | 6 |
 *
 * returns a vector of that contains unique neighbor labels
 */
vector<uchar> ConnectedComponent::getNeighbors( uchar * curr_ptr, uchar * prev_ptr, int x, int y, int cols ) {
    vector<uchar> neighbors;
    
    /* Actually we only consider pixel 1, 2, 3, and 4 */
    /* At this point of time, the logic hasn't traversed thru 5, 6, 7, 8 */
    if( prev_ptr != NULL ) {
        if( prev_ptr[x] != 0 )
            neighbors.push_back( prev_ptr[x] );
        
        if( x > 0 && prev_ptr[x-1] != 0 )
            neighbors.push_back( prev_ptr[x-1] );
        
        if( x < cols - 1 && prev_ptr[x+1] != 0 )
            neighbors.push_back( prev_ptr[x+1] );
    }
    
    if( x > 0 && curr_ptr[x-1] != 0 )
        neighbors.push_back( curr_ptr[x-1] );
    
    
    /* Reduce to unique labels */
    /* This is because I'm not using set (it doesn't have random element access) */
    vector<uchar> result;
    if( !neighbors.empty() ) {
        std::sort( neighbors.begin(), neighbors.end() );
        std::unique_copy( neighbors.begin(), neighbors.end(), std::back_inserter( result ) );
    }
    
    return result;
}
