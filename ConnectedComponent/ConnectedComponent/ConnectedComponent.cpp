//
//  ConnectedComponent.cpp
//  RobustTextDetection
//
//  Created by Saburo Okita on 06/06/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "ConnectedComponent.h"
#include <stdexcept>

using namespace std;
using namespace cv;

ConnectedComponent::ConnectedComponent( int max_component )
: maxComponent( max_component ){
}

ConnectedComponent::~ConnectedComponent(){
}

/**
 * Apply connected component labeling
 * it only works for predefined maximum no of connected components
 * and currently treat black color as background
 */
Mat ConnectedComponent::apply( const Mat& image ) {
    CV_Assert( image.type() == CV_8UC1 );
    
    Mat result = image.clone();
    result.convertTo( result, CV_32SC1 );
    
    /* First pass, labeling the regions incrementally */
    nextLabel = 1;
    vector<int> linked(maxComponent);
    
    int * prev_ptr = NULL;
    for( int y = 0; y < result.rows; y++ ) {
        int * curr_ptr = result.ptr<int>(y);
        
        for( int x = 0; x < result.cols; x++ ) {
            
            if( curr_ptr[x] != 0 ) {
                vector<int> neighbors = getNeighbors( curr_ptr, prev_ptr, x, y, result.cols );
                
                if( neighbors.empty() ) {
                    curr_ptr[x] = nextLabel;
                    nextLabel++;
                    
                    if( nextLabel >= maxComponent ) {
                        stringstream ss;
                        ss  << "Current label count [" << (int) nextLabel
                            << "] exceeds maximum no of components [" << maxComponent << "]";
                        throw std::runtime_error( ss.str() );
                    }
                }
                else {
                    /* Use the minimum label out from the neighbors */
                    int min_index = (int) (min_element( neighbors.begin(), neighbors.end() ) - neighbors.begin());
                    curr_ptr[x]   = neighbors[min_index];
                    
                    for( int neighbor: neighbors )
                        disjointUnion( curr_ptr[x], neighbor, linked );
                }
            }
        }
        prev_ptr = curr_ptr;
    }
    
    /* Second pass merge the equivalent labels */
    nextLabel = 1;
    vector<int> temp, labels_set(maxComponent);
    for( int y = 0; y < result.rows; y++ ) {
        int * curr_ptr = result.ptr<int>(y);
        
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
void ConnectedComponent::disjointUnion( int a, int b, vector<int>& parent  ) {
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
int ConnectedComponent::disjointFind( int a, vector<int>& parent, vector<int>& labels ) {
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
vector<int> ConnectedComponent::getNeighbors( int * curr_ptr, int * prev_ptr, int x, int y, int cols ) {
    vector<int> neighbors;
    
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
    vector<int> result;
    if( !neighbors.empty() ) {
        std::sort( neighbors.begin(), neighbors.end() );
        std::unique_copy( neighbors.begin(), neighbors.end(), std::back_inserter( result ) );
    }
    
    return result;
}
