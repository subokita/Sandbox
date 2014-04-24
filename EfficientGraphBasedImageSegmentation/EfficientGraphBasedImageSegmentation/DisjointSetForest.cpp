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
//  SegmentGraph.cpp
//  EfficientGraphBasedImageSegmentation
//
//  Created by Saburo Okita on 24/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "DisjointSetForest.h"
#include <algorithm>


DisjointSetForest::DisjointSetForest() {
}

DisjointSetForest::DisjointSetForest( int no_of_elements ) {
    init( no_of_elements );
}

/**
 * Initialize the forest
 */
void DisjointSetForest::init( int no_of_elements ) {
    this->elements.clear();
    this->elements.reserve( no_of_elements );
    this->num = no_of_elements;
    
    for( int i = 0; i < no_of_elements; i++ ) {
        elements[i].rank    = 0;
        elements[i].size    = 1;
        elements[i].parent  = i;
    }
}

DisjointSetForest::~DisjointSetForest() {
}

/**
 * Find a given set inside the forest
 */
int DisjointSetForest::find( int x ) {
    int y = x;
    while( y != elements[y].parent )
        y = elements[y].parent;
    elements[x].parent = y;
    return y;
}

/**
 * Join two sets together
 */
void DisjointSetForest::join( int x, int y ) {
    if ( elements[x].rank > elements[y].rank ) {
        elements[y].parent  = x;
        elements[x].size    += elements[y].size;
    }
    else {
        elements[x].parent  = y;
        elements[y].size    += elements[x].size;
        
        if( elements[x].rank == elements[y].rank )
            elements[y].rank++;
    }
    num--;
}

/**
 * Returns the size of the set
 */
int DisjointSetForest::size( int x ) {
    return elements[x].size;
}

/**
 * Returns the total number of unique sets inside the forest
 */
int DisjointSetForest::noOfElements() {
    return num;
}

/**
 * Segment the graph based on the weight of each edges
 */
void DisjointSetForest::segmentGraph( int no_of_vertices, int no_of_edges, vector<Edge>& edges, float c ) {
    init( no_of_vertices );
    
    sort( edges.begin(), edges.end(), []( Edge& a, Edge& b){
        return a.weight < b.weight;
    });
    
    vector<float> thresholds( no_of_vertices, c );

    for( Edge& edge: edges ){
        int a = this->find( edge.a );
        int b = this->find( edge.b );
        
        if( a != b ) {
            
            /* If the weight is below respective threshold, union both sets together */
            if( edge.weight <= thresholds[a] && edge.weight <= thresholds[b] ) {
                this->join( a, b );
                a = this->find( a );
                thresholds[a] = edge.weight + c / this->size( a );
            }
        }
    }
}
