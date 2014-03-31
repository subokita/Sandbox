//
//  main.cpp
//  Spectral Clustering in OpenCV
//
//  Created by Saburo Okita on 31/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <vector>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

vector<Point2f> createCircles( int center_x, int center_y, float radius, int n_samples ){
    vector<Point2f> result;
    Point2f center ( center_x, center_y );
    
    RNG rng;
    
    for( int i = 0; i < n_samples; i++ ) {
        float x = rng.gaussian( radius ) *  (rand() % 2 == 0 ? 1.0 : -1.0 );
        float y = rng.gaussian( radius ) *  (rand() % 2 == 0 ? 1.0 : -1.0 );
        
        result.push_back( center + Point2f(x, y) );
    }
    
    return result;
}

void plot( Mat& img, vector<Point2f>& points, Scalar color = Scalar(0) ) {
    for( Point2f point: points)
        circle( img, point, 2, color, 2 );
}

/**
 * Plot points and color it based on the given labels
 **/
void plot( Mat& img, Mat& points, Mat& labels ) {
    int total = points.rows;

    if( total < 1 )
        return;
    
    static Scalar colors[] = {
        Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255),
        Scalar(255, 255, 0), Scalar(255, 0, 255), Scalar(0, 255, 255)
    };
    
    int * labels_ptr = labels.ptr<int>(0);
    Point2f * points_ptr = points.ptr<Point2f>( 0 );
    
    for( int i = 0; i < total; i++ )
        circle( img, points_ptr[i], 2, colors[labels_ptr[i]], 2 );
}

/**
 * Create a degree matrix out from the given adjacency matrix
 **/
Mat degreeMatrix( Mat& adjacency ) {
    int cols = adjacency.cols;
    
    Mat degree( 1, cols, CV_32FC1 );
    for( int col = 0; col < cols; col++ )
        degree.at<float>(0, col) = sum(adjacency.col( col ))[0];
    
    return degree;
}

/**
 * Create an adjacency matrix based on the gaussian distance between the points
 **/
Mat gaussianDistance( vector<Point2f> points, float sigma = 1.0, float division_factor = 1.0 ) {
    int m = static_cast<int>(points.size());
    Mat adjacency(m, m, CV_32FC1, Scalar(0.0));

    for( int i = 0; i < m; i++ ) {
        float * ptr = adjacency.ptr<float>(i);
        for( int j = 0; j < m; j++ ) {
            if( i >= j )
                continue;
            /* Yeah, this is because when the original coordinates is used, it's bound to overflow in later part */
            Point2f p1(points[i].x / division_factor, points[i].y / division_factor);
            Point2f p2(points[j].x / division_factor, points[j].y / division_factor);
            ptr[j] = (p1 - p2).dot(p1 - p2);
        }
    }
    
    completeSymm( adjacency );
    
    adjacency = -adjacency / (2.0 * sigma * sigma);
    exp( adjacency, adjacency );
    adjacency = adjacency - Mat::eye(m, m, CV_32FC1 );
    
    return adjacency;
}


int main(int argc, const char * argv[]) {
    /* Create 2 blobs of points */
    vector<Point2f> points1 = createCircles( 150, 150, 50.0f, 500 );
    vector<Point2f> points2 = createCircles( 450, 450, 40.0f, 500 );
    vector<Point2f> points;
    points.insert( points.end(), points1.begin(), points1.end() );
    points.insert( points.end(), points2.begin(), points2.end() );
    
    /* Create adjacency and degree matrices */
    Mat adjacency = gaussianDistance( points, 0.1f, 500.0f );
    Mat degree = degreeMatrix( adjacency );
    
    /* Create laplacian matrix */
    Mat L = Mat::diag( degree ) - adjacency;
    Mat degree_05;
    pow( degree, -0.5, degree_05 );
    L = (degree_05 * L) * degree_05;
    
    /* Perform eigen decompositions */
    Mat eigenvalues, eigenvectors;
    eigen( L, eigenvalues, eigenvectors );
    
    /* Since it's automatically sorted in descending order, take the last two entries of eigenvectors */
    eigenvectors = eigenvectors.rowRange( eigenvectors.rows - 2, eigenvectors.rows ).t();
    
    /* Perform K-means on eigenvectors */
    Mat labels;
    kmeans( eigenvectors, 2, labels, TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 1000, 1e-5), 2, KMEANS_RANDOM_CENTERS );
    
    /* Plot it out */
    Mat img(600, 600, CV_8UC3, Scalar(255, 255, 255) );
    Mat points_mat( points );
    plot( img, points_mat, labels );
    
    namedWindow( "Spectral Clustering on OpenCV" );
    moveWindow( "Spectral Clustering on OpenCV", 0, 0 );
    imshow( "Spectral Clustering on OpenCV", img );
    waitKey();
    
    return 0;
}

