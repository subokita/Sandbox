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
//  DisjointSetForest.h
//  EfficientGraphBasedImageSegmentation
//
//  Created by Saburo Okita on 24/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __EfficientGraphBasedImageSegmentation__SegmentGraph__
#define __EfficientGraphBasedImageSegmentation__SegmentGraph__

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

/**
 * Set within the disjoint forest
 */
struct SetNode {
    int rank;
    int parent;
    int size;
};

/**
 * Represents a weighted edge
 */
struct Edge {
    int a;
    int b;
    float weight;
    
    bool operator<( const Edge& other ) {
        return weight < other.weight;
    }
};


/**
 * Class to represent Disjoint Set Forest, more can be read over here:
 * http://en.wikipedia.org/wiki/Disjoint-set_data_structure
 */
class DisjointSetForest{
public:
    DisjointSetForest();
    DisjointSetForest( int no_of_elements );
    ~DisjointSetForest();
    
    void init( int no_of_elements );
    int find( int x );
    void join( int x, int y );
    int size( int x );
    int noOfElements();
    
    void segmentGraph( int no_of_vertices, int no_of_edges, vector<Edge>& edges, float c );
    
private:
    vector<SetNode> elements;
    int num;
};

#endif /* defined(__EfficientGraphBasedImageSegmentation__SegmentGraph__) */
