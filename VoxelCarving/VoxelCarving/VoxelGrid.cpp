//
//  VoxelGrid.cpp
//  VoxelCarving
//
//  Created by Saburo Okita on 13/08/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "VoxelGrid.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace cv;

VoxelGrid::VoxelGrid(){}

VoxelGrid::VoxelGrid( int x_div, int y_div, int z_div, float voxel_size, cv::Point3f voxel_origin ) :
    voxelSize(voxel_size),
    grid  ( x_div * y_div * z_div, Point3f(0.0f, 0.0f, 0.0f) ),
    colors( x_div * y_div * z_div  ),
    depths( x_div * y_div * z_div, FLT_MAX )
{
    initVoxelGrid( x_div, y_div, z_div, voxel_origin  );
}

VoxelGrid::~VoxelGrid() {
    
}

vector<cv::Point3f>& VoxelGrid::getGrid() {
    return this->grid;
}
vector<float>& VoxelGrid::getDepths() {
    return this->depths;
}
vector<cv::Vec3b>& VoxelGrid::getColors() {
    return this->colors;
}

unsigned int VoxelGrid::getSize() {
    return static_cast<unsigned int>(grid.size());
}

/**
 * Init grid of empty voxels
 */
void VoxelGrid::initVoxelGrid( int x_div, int y_div, int z_div, cv::Point3f& voxel_origin ) {
    int k = 0;
    for( int x = 0; x < x_div; x++ ) {
        for( int y = 0; y < y_div; y++ ) {
            for( int z = 0; z < z_div; z++ ) {
                grid[k++] = voxel_origin + Point3f( x, y, z ) * voxelSize;
            }
        }
    }
}

/*
 * Perform visibility carving for several different camera viewpoints
 */
void VoxelGrid::carve(std::vector<cv::Mat>& images, std::vector<cv::Mat>& masks,
                      std::vector<cv::Mat>& projection_matrices ){
    
    Mat projected_coord (3, 1, CV_32FC1);
    Mat voxel_coord     (4, 1, CV_32FC1);
    
    float * voxel_coord_ptr     = voxel_coord.ptr<float>();
    float * projected_coord_ptr = projected_coord.ptr<float>();
    
    for( int k = 0; k < projection_matrices.size(); k++ ) {
        vector<cv::Point3f> new_grid;
        vector<cv::Vec3b> new_colors;
        vector<float> new_depths;
        
        Mat& image = images[k];
        Mat& mask  = masks[k];
        Mat& projection_mat = projection_matrices[k];
        
        for( int i = 0; i < grid.size(); i++ ) {
            Point3f& voxel = grid[i];
            voxel_coord_ptr[0] = voxel.x;
            voxel_coord_ptr[1] = voxel.y;
            voxel_coord_ptr[2] = voxel.z;
            voxel_coord_ptr[3] = 1.0;
            
            
            /* Project the voxel coord back using corresponding projection matrix */
            projected_coord = projection_mat * voxel_coord;
            
            /* Then divide by the w component */
            int x = projected_coord_ptr[0] / projected_coord_ptr[2];
            int y = projected_coord_ptr[1] / projected_coord_ptr[2];
            
            /* Try to clear up the voxel first */
            if( x < 0 || x >= mask.cols || y < 0 || y >= mask.rows )
                continue;
            
            /* Only retain voxel that's in the foreground */
            if( mask.at<uchar>(y, x) ) {
                if( projected_coord_ptr[2] < depths[i] ) {
                    depths[i] = projected_coord_ptr[2];
                    colors[i] = image.at<Vec3b>(y, x);
                }
                
                new_depths.push_back( depths[i] );
                new_colors.push_back( colors[i] );
                new_grid.push_back( voxel );
            }
        }
        
        /* Update our voxel grid, colors and depth information */
        this->grid = new_grid;
        this->colors = new_colors;
        this->depths = new_depths;
    }
}

/**
 * Subdivide the existing grid into higher resolution coords, and perform carving on them
 */
void VoxelGrid::subDivideAndRefine( int sub_division, std::vector<cv::Mat>& images, std::vector<cv::Mat>& masks,
                                    std::vector<cv::Mat>& projection_matrices ) {
    
    /* Basically create a new voxel grid that contains the subdivided coordinates that weren't considered */
    /* by the original voxel grid */
    VoxelGrid new_grid;
    new_grid.voxelSize = voxelSize / sub_division;
    
    for( int i = 0; i < grid.size(); i++ ) {
        
        for( int x = 1; x < sub_division; x++ ) {
            for( int y = 1; y < sub_division; y++ ) {
                for( int z = 1; z < sub_division; z++ ) {
                    Point3f voxel = grid[i];
                    voxel.x += (x * new_grid.voxelSize);
                    voxel.y += (y * new_grid.voxelSize);
                    voxel.z += (z * new_grid.voxelSize);
                    
                    Vec3b color = colors[i];
                    new_grid.grid.push_back( voxel );
                    new_grid.colors.push_back( color );
                    new_grid.depths.push_back( FLT_MAX );
                }
            }
        }
    }
    
    /* Again, perform visibility carving */
    new_grid.carve( images, masks, projection_matrices );
    
    /* Save this newer voxel grid */
    this->grid.insert  (this->grid.end(),   new_grid.grid.begin(),   new_grid.grid.end());
    this->colors.insert(this->colors.end(), new_grid.colors.begin(), new_grid.colors.end());
    this->depths.insert(this->depths.end(), new_grid.depths.begin(), new_grid.depths.end());
}


/**
 * Normalize the voxel coordinates, so they stay between -1.0 an 1.0
 */
void VoxelGrid::normalize() {
    Point3f min_vec(FLT_MAX, FLT_MAX, FLT_MAX);
    Point3f max_vec(FLT_MIN, FLT_MIN, FLT_MIN);

    for( Point3f& voxel: grid ) {
        if( min_vec.x > voxel.x )
            min_vec.x = voxel.x;
        if( min_vec.y > voxel.y )
            min_vec.y = voxel.y;
        if( min_vec.z > voxel.z )
            min_vec.z = voxel.z;
        
        if( max_vec.x < voxel.x )
            max_vec.x = voxel.x;
        if( max_vec.y < voxel.y )
            max_vec.y = voxel.y;
        if( max_vec.z < voxel.z )
            max_vec.z = voxel.z;
    }
    
    Point3f length = (max_vec - min_vec) * 0.5;
    
    for( Point3f& voxel: grid ) {
        voxel.x = (voxel.x - min_vec.x) / length.x - 1.0f;
        voxel.y = (voxel.y - min_vec.y) / length.y - 1.0f;
        voxel.z = (voxel.z - min_vec.z) / length.z - 1.0f;
    }
}

/**
 * Save the carved voxel grid as a PLY model file
 */
void VoxelGrid::saveAsPLY( const std::string filename ) {
    ofstream output( filename );
    
    unsigned int size = static_cast<unsigned int>(grid.size());
    
    output << "ply" << endl;
    output << "format ascii 1.0" << endl;
    output << "element vertex " << size << endl;
    output << "property float x" << endl;
    output << "property float y" << endl;
    output << "property float z" << endl;
    output << "property uchar red" << endl;
    output << "property uchar green" << endl;
    output << "property uchar blue" << endl;
    output << "element face " << size / 3 << endl;
    output << "property list uchar int vertex_indices" << endl;
    output << "end_header" << endl;
    
    /* Write vertex and vertex color */
    for( unsigned int i = 0; i < size; i++ ) {
        Point3f& voxel = grid[i];
        output  << voxel.x << " "
                << voxel.y << " "
                << voxel.z << " ";
        
        Vec3b& color = colors[i];
        output  << static_cast<int>(color[2]) << " "
                << static_cast<int>(color[1]) << " "
                << static_cast<int>(color[0]) << endl;
    }
    
    /* Write the faces */
    for( int i = 0; i < size; i += 3 )  {
        output  << "3 " << i
                << " "  << (i + 1)
                << " "  << (i + 2) << endl;
    }
    
    output.close();
}