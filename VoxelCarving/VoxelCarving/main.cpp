//
//  main.cpp
//  VoxelCarving
//
//  Created by Saburo Okita on 12/08/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <sstream>
#include <fstream>

#include "VoxelGrid.h"

using namespace std;
using namespace cv;


void replaceAll(std::string& str, const std::string& from, const std::string& to);
vector<Mat> readCalibFile( const string path, const string filename );

int main(int argc, const char * argv[])
{
    const string path = "/Users/saburookita/Sandbox/VoxelCarving/";
    namedWindow("");
    moveWindow("", 0, 0);

    vector<Mat> projection_matrices;

    const vector<string> calib_filenames = { "camera0.m", "camera1.m", "camera2.m", };
    for( string filename: calib_filenames ) {
        vector<Mat> temp = readCalibFile( path + "morpheus/calib", filename );
        projection_matrices.insert(projection_matrices.end(), temp.begin(), temp.end() );
    }
    
    vector<Mat> images, masks;
    
    char temp_str[255];
    for( int k = 0; k < projection_matrices.size(); k++ ) {
        sprintf(temp_str, (path + "morpheus/visualize/%02d.jpg").c_str(), k );
        Mat img = imread(temp_str);
        Mat mask = img <= 220;

        /* Show the image and mask */
        Mat appended( img.rows / 4, img.cols / 2, CV_8UC3 );
        Mat temp_img;
        resize(img, temp_img, Size(img.cols / 4, img.rows / 4));
        temp_img.copyTo(Mat(appended, Rect(0, 0, temp_img.cols, temp_img.rows)));
        resize(mask, temp_img, Size(img.cols / 4, img.rows / 4));
        temp_img.copyTo(Mat(appended, Rect(temp_img.cols, 0, temp_img.cols, temp_img.rows)));
        
        imshow("", appended);
        waitKey(50);
        
        /* Dilate the mask to close all the holes */
        cvtColor(mask, mask, CV_BGR2GRAY );
        dilate(mask, mask, Mat());
        
        images.push_back( img );
        masks.push_back( mask );
    }
    
    cout << "Performing voxel carving" << endl;
    VoxelGrid grid(100, 100, 100, 800.0 / 100.0, Point3f( -100.0, -100.0, -100.0) );
    grid.carve( images, masks, projection_matrices );
    grid.subDivideAndRefine(2, images, masks, projection_matrices );
    grid.normalize();
    
    cout << "Saving to PLY" << endl;
    grid.saveAsPLY( path + "morpheus/morpheus.ply");
    
    cout << "DONE" << endl;
    return 0;
}


/*
 * String replace, taken from http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
 */
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

/**
 * Read the projection matrices from the camera.m matlab files
 */
vector<Mat> readCalibFile( const string path, const string filename ) {
    ifstream stream( path + "/" + filename );
    
    vector<Mat> projection_matrices;
    
    string line;
    if ( stream.is_open() ) {
        while( getline( stream, line )) {
            if( line.length() == 0 )
                continue;
            
            /* Find the line that starts with 'proj_' */
            if( line.find("proj") == 0 ) {
                size_t start = line.find("[") + 2;
                size_t end   = line.find("]") - start;
                line = line.substr(start, end);
                
                replaceAll(line, ";", "");
                
                /* and convert them into 3x4 projection matrix */
                vector<float> tokens;
                istringstream iss( line );
                std::copy(istream_iterator<float>(iss), istream_iterator<float>(), back_inserter(tokens));
                
                Mat proj_mat = (Mat_<float>(3, 4) << tokens[0], tokens[1], tokens[2], tokens[3],
                                tokens[4], tokens[5], tokens[6], tokens[7],
                                tokens[8], tokens[9], tokens[10], tokens[11]);
                
                projection_matrices.push_back( proj_mat );
            }
        }
    }
    else {
        throw runtime_error("Unable to open " + path + "/" + filename );
    }
    
    stream.close();
    
    return projection_matrices;
}