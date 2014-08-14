//
//  VoxelGrid.h
//  VoxelCarving
//
//  Created by Saburo Okita on 13/08/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __VoxelCarving__VoxelGrid__
#define __VoxelCarving__VoxelGrid__

#include <iostream>
#include <opencv2/opencv.hpp>

class VoxelGrid {
public:
    VoxelGrid();
    VoxelGrid( int x_div, int y_div, int z_div, float voxel_size, cv::Point3f voxel_origin );
    ~VoxelGrid();
    
    std::vector<cv::Point3f>& getGrid();
    std::vector<float>& getDepths();
    std::vector<cv::Vec3b>& getColors();
    unsigned int getSize();
    
    void carve(std::vector<cv::Mat>& images, std::vector<cv::Mat>& masks, std::vector<cv::Mat>& projection_matrices );
    void subDivideAndRefine( int sub_division, std::vector<cv::Mat>& images, std::vector<cv::Mat>& masks,
                             std::vector<cv::Mat>& projection_matrices );
    void normalize();
    
    void saveAsPLY( const std::string filename );
    
protected:
    void initVoxelGrid( int x_div, int y_div, int z_div, cv::Point3f& voxel_origin );
    
protected:
    float voxelSize;
    
    std::vector<cv::Point3f> grid;
    std::vector<float> depths;
    std::vector<cv::Vec3b> colors;
};

#endif /* defined(__VoxelCarving__VoxelGrid__) */
