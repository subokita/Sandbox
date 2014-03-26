//
//  main.cpp
//  SFM
//
//  Created by Saburo Okita on 26/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>

#include "SimpleViewer.h"

using namespace std;
using namespace cv;

/**
 * Structure from motion from 2 cameras, using farneback optical flow as the 'features'
 * No, this doesn't work on more than 2 cams, because that requires bundle adjustment, which
 * I'm still searching if there's an OpenCV implementation of it
 */
Mat sfm( Mat& img1, Mat& img2, Mat& cam_matrix, Mat& dist_coeff ) {
    Mat gray1, gray2;
    cvtColor( img1, gray1, CV_BGR2GRAY );
    cvtColor( img2, gray2, CV_BGR2GRAY );
    
    /*  Find the optical flow using farneback dense algorithm
        Note that you might need to tune the parameters, especially window size.
        Smaller window size param, means more ambiguity when calculating the flow.
     */
    Mat flow_mat;
    calcOpticalFlowFarneback( gray1, gray2, flow_mat, 0.5, 3, 12, 3, 5, 1.2, 0 );
    
    vector<Point2f> left_points, right_points;
    for ( int y = 0; y < img1.rows; y+=6 ) {
        for ( int x = 0; x < img1.cols; x+=6 ) {
            /* Flow is basically the delta between left and right points */
            Point2f flow = flow_mat.at<Point2f>(y, x);
            
            /*  There's no need to calculate for every single point, 
                if there's not much change, just ignore it
             */
            if( fabs(flow.x) < 0.1 && fabs(flow.y) < 0.1 )
                continue;
            
            left_points.push_back(  Point2f( x, y ) );
            right_points.push_back( Point2f( x + flow.x, y + flow.y ) );
        }
    }
    
    /* Undistort the points based on intrinsic params and dist coeff */
    undistortPoints( left_points, left_points, cam_matrix, dist_coeff );
    undistortPoints( right_points, right_points, cam_matrix, dist_coeff );
    
    /* Try to find essential matrix from the points */
    Mat fundamental = findFundamentalMat( left_points, right_points, FM_RANSAC, 3.0, 0.99 );
    Mat essential   = cam_matrix.t() * fundamental * cam_matrix;
    
    /* Find the projection matrix between those two images */
    SVD svd( essential );
    static const Mat W = (Mat_<double>(3, 3) <<
                         0, -1, 0,
                         1, 0, 0,
                         0, 0, 1);
    
    static const Mat W_inv = W.inv();
    
    Mat_<double> R1 = svd.u * W * svd.vt;
    Mat_<double> T1 = svd.u.col( 2 );
    
    Mat_<double> R2 = svd.u * W_inv * svd.vt;
    Mat_<double> T2 = -svd.u.col( 2 );
    
    static const Mat P1 = Mat::eye(3, 4, CV_64FC1 );
    Mat P2 =( Mat_<double>(3, 4) <<
             R1(0, 0), R1(0, 1), R1(0, 2), T1(0),
             R1(1, 0), R1(1, 1), R1(1, 2), T1(1),
             R1(2, 0), R1(2, 1), R1(2, 2), T1(2));
    
    /*  Triangulate the points to find the 3D homogenous points in the world space
        Note that each column of the 'out' matrix corresponds to the 3d homogenous point
     */
    Mat out;
    triangulatePoints( P1, P2, left_points, right_points, out );
    
    /* Since it's homogenous (x, y, z, w) coord, divide by w to get (x, y, z, 1) */
    vector<Mat> splitted = {
        out.row(0) / out.row(3),
        out.row(1) / out.row(3),
        out.row(2) / out.row(3)
    };
    
    merge( splitted, out );
    
    return out;
}

int main(int argc, const char * argv[]) {
    Mat img1 = imread( "/Users/saburookita/Downloads/visualize/00.jpg" );
    Mat img2 = imread( "/Users/saburookita/Downloads/visualize/01.jpg" );
    
    Mat cam_matrix = (Mat_<double>(3, 3) <<
                      6624.070862,           0, 1008.853968,
                      0, 6624.995786, 1132.158299,
                      0,           0, 1);
    
    Mat dist_coeff = (Mat_<double>(1, 5) << -0.159685, 0.037437, -0.000708, -0.000551, 0.000000);
    
    Mat out = sfm( img1, img2, cam_matrix, dist_coeff );

    /* Convert the matrix of homogenous coords to our glm::vec3 representation */
    vector<glm::vec3> points;
    for ( int i = 0; i < out.cols; i++ ) {
        Mat m = out.col( i );
        float x = m.at<float>(0);
        float y = m.at<float>(1);
        float z = m.at<float>(2);
        
        points.push_back( glm::vec3(x, y, z) );
    }
    
    /*  This is a silly hack to shift the image to the origin coord (0, 0, 0) 
        by applying K-mean cluster (in this case, 1 cluster), to get the cluster center ...
     */
    Mat labels, center;
    kmeans( out.t(), 1, labels, TermCriteria( CV_TERMCRIT_ITER, 1000, 1e-5), 1, KMEANS_RANDOM_CENTERS, center );
    
    /*  ... and shift all the points based on the cluster center */
    for( glm::vec3& point: points ) {
        point.x -= center.at<float>(0, 0);
        point.y -= center.at<float>(0, 1);
        point.z -= center.at<float>(0, 2);
    }
    
    
    /* Briefly see how our original images look like */
    namedWindow( "" );
    moveWindow( "", 0, 0 );
    
    Mat appended( img1.rows, img1.cols + img2.cols, img1.type() );
    img1.copyTo( Mat( appended, Rect(0, 0, img1.cols, img1.rows) ) );
    img2.copyTo( Mat( appended, Rect(img1.cols, 0, img2.cols, img2.rows) ) );
    resize( appended, appended, Size(), 0.3f, 0.3f );
    
    imshow( "", appended );
    waitKey();
    
    SimpleViewer viewer;
    if( viewer.init( "Simple Point Cloud Viewer", 1066, 600 ) ) {
        /* Pass the coordinates to our simple viewer */
        viewer.setVertexData( points );
        viewer.run();
    }
    
    return 0;
}

