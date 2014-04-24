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
//  EGBS.cpp
//  EfficientGraphBasedImageSegmentation
//
//  Created by Saburo Okita on 23/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "EGBS.h"
#include <map>


EGBS::EGBS() {
    
}

EGBS::~EGBS() {
    
}

/**
 * Calculate the difference between two (3 channels) pixels, by 
 * taking the L2 norm of their differences.
 */
float EGBS::diff( Mat& rgb, int x1, int y1, int x2, int y2 ) {
    Vec3f pix1 = rgb.at<Vec3f>(y1, x1);
    Vec3f pix2 = rgb.at<Vec3f>(y2, x2);
    return sqrt( (pix1 - pix2).dot((pix1 - pix2)) );
}

/**
 * Apply segmentation
 */
int EGBS::applySegmentation( Mat& image, float sigma, float threshold, int min_component_size ) {
    this->image = image.clone();
    this->imageSize = image.size();
    
    /* Apply gaussian blur to smoothen the image */
    Mat smoothed;
    image.convertTo( smoothed, CV_32FC1 );
    GaussianBlur( smoothed, smoothed, Size(5,5), sigma );
    
    vector<Edge> edges( imageSize.area() * 4 );
    int no_of_edges = 0;
    
    /* Create edges between each pixels, with the weight as the L2 norm between each color channels of the pixels */
    for( int y = 0; y < imageSize.height; y++ ) {
        for( int x = 0; x < imageSize.width; x++ ) {
            if( x < imageSize.width - 1 ){
                edges[no_of_edges].a        = y * imageSize.width + x;
                edges[no_of_edges].b        = y * imageSize.width + (x + 1);
                edges[no_of_edges].weight   = diff( smoothed, x, y, x + 1, y );
                no_of_edges++;
            }

            if( y < imageSize.height - 1 ) {
                edges[no_of_edges].a        = y       * imageSize.width + x;
                edges[no_of_edges].b        = (y + 1) * imageSize.width + x;
                edges[no_of_edges].weight   = diff( smoothed, x, y, x, y + 1 );
                no_of_edges++;
            }

            if( (x < imageSize.width - 1) && (y < imageSize.height - 1) ) {
                edges[no_of_edges].a        = y       * imageSize.width + x;
                edges[no_of_edges].b        = (y + 1) * imageSize.width + (x + 1);
                edges[no_of_edges].weight   = diff( smoothed, x, y, x + 1, y + 1 );
                no_of_edges++;
            }

            if( (x < imageSize.width - 1) && (y > 0) ) {
                edges[no_of_edges].a        = y       * imageSize.width + x;
                edges[no_of_edges].b        = (y - 1) * imageSize.width + (x + 1);
                edges[no_of_edges].weight   = diff( smoothed, x, y, x + 1, y - 1 );
                no_of_edges++;
            }
        }
    }
    
    /* Resize the vector, since we over-initialized it */
    edges.resize( no_of_edges );
    
    /* Apply segmentation on the edges */
    forest.segmentGraph( imageSize.height * imageSize.width, no_of_edges, edges, threshold );

    /* Union all the smaller sets */
    for( Edge& edge: edges ) {
        int a = forest.find( edge.a );
        int b = forest.find( edge.b );
        if( (a != b) && (( forest.size(a) < min_component_size) || (forest.size(b) < min_component_size)) ) {
            forest.join( a, b );
        }
    }
    
    return forest.noOfElements();
}


int EGBS::noOfConnectedComponents() {
    return forest.noOfElements();
}

/**
 * Recolor the image based on either average color of each cluster, or randomized color scheme
 */
Mat EGBS::recolor( bool random_color) {
    Mat result( imageSize, CV_8UC3, Scalar(0, 0, 0) );
    map<int, Vec3f> colors;
    
    if( !random_color ){
        map<int, int> count;
        
        /* If it's not random coloring, color based on the average of each clusters */
        for(int y = 0; y < imageSize.height; y++ ) {
            Vec3b * ptr = image.ptr<Vec3b>(y);
            
            for(int x = 0; x < imageSize.width; x++ ) {
                int component = forest.find( y * imageSize.width + x );
                colors[component] += ptr[x];
                count[component]++;
            }
        }
        
        /* Averaging the color */
        for( auto itr : colors )
            colors[itr.first] /= count[itr.first];
    }
    else {
        /* Else just randomize the colors */
        for(int y = 0; y < imageSize.height; y++ ) {
            for(int x = 0; x < imageSize.width; x++ ) {
                int component = forest.find( y * imageSize.width + x );
                if( colors.count( component ) == 0 )
                    colors[component] = Vec3f( rand() % 255, rand() % 255, rand() % 255 );
            }
        }
    }
    
    /* Recolor the image */
    for(int y = 0; y < imageSize.height; y++ ) {
        Vec3b * ptr = result.ptr<Vec3b>(y);
        
        for(int x = 0; x < imageSize.width; x++ ) {
            int component = forest.find( y * imageSize.width + x );
            Vec3f color = colors[component];
            ptr[x] = Vec3b( color[0], color[1], color[2] );
        }
    }


    return result;
}
