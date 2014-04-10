//
//  main.cpp
//  Procrustes
//
//  Created by Saburo Okita on 06/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include "Procrustes.h"

using namespace std;
using namespace cv;

vector<Mat> generateTestData( int size ) {
//    srand(static_cast<unsigned int>(time(NULL)));
    
    vector<Mat> result;
    
    /* First create random points X, centered at (250, 250) with stddev of 80 */
    Mat X(40, 1, CV_32FC2 );
    RNG rng;
    rng.fill( X, RNG::NORMAL, Scalar( 250, 250 ), Scalar( 80, 80 ) );
    result.push_back( X );
    
    for( int i = 1; i < size; i++ ) {
        float scale = ((rand() % 100) + 25) / 100.0;
        Scalar translation( (rand() % 600) + 100, (rand() % 300) );
        
        /* Transform Y so that it's rotated and translated version of X */
        float angle = (rand() % 90) * 180.0 / M_PI;
        Mat S = (Mat_<float>(2,2) << cosf(angle), -sinf(angle), sinf(angle), cosf( angle) );
        Mat Y;
        cv::transform( scale * X, Y, S );
        Y += translation;
        
        /* Jitter the Y points a bit, so that it's not exactly transformed version of X */
        Mat jitter( Y.size(), Y.type() );
        rng.fill( jitter, RNG::NORMAL, Scalar(0, 0), Scalar( 5, 5 ));
        Y += jitter;
        
        result.push_back( Y );
    }
    
    return result;
}

/**
 * A simple helper class to plot the points
 **/
void plot(Mat& img, Mat& points, Scalar color ) {
    vector<Point2f> vec;
    points.copyTo( vec );
    
    for( Point2f p: vec )
        circle( img, p, 2, color, 2 );
}

void procrustesAnalysisTest() {
    namedWindow("");
    moveWindow( "", 0, 0 );
    
    vector<Mat> points = generateTestData( 2 );
    
    Mat img(600, 900, CV_8UC3, Scalar(255, 255, 255) );
    
    /* Plot X */
    plot( img, points[0], Scalar(0, 200, 0));
    
    /* Plot Y */
    plot( img, points[1], Scalar(200, 0, 0));
    
    imshow( "", img );
    waitKey();
    
    /*  Perform procrustes analysis, to obtain approximate transformed points from Y to X */
    Procrustes proc;
    proc.procrustes( points[0], points[1] );
    vector<Point2f> Y_prime = proc.yPrimeAsVector();
    for( Point2f point : Y_prime )
        circle( img, point, 3, Scalar(0, 0, 255), 2);
    
    imshow( "", img );
    waitKey();
    
    /* Output the squared error, and scale, rotation and translation values involved in acquiring Y prime */
    cout << proc.error << endl;
    cout << proc.scale << endl;
    cout << proc.rotation << endl;
    cout << proc.translation << endl;
}

void generalizedProcrustesTest() {
    namedWindow("");
    moveWindow( "", 0, 0 );
    
    /* First we generate 6 set of points */
    vector<Mat> points = generateTestData( 6 );
    
    /* Colors to differentiate each set of points */
    const vector<Scalar> colors = {
        Scalar( 255, 0, 0 ),
        Scalar( 0, 255, 0 ),
        Scalar( 0, 0, 255 ),
        Scalar( 255, 255, 0 ),
        Scalar( 0, 255, 255 ),
        Scalar( 255, 0, 255 ),
    };
    
    /* Plot the points out */
    Mat img(600, 900, CV_8UC3, Scalar(255, 255, 255) );
    for( int i = 0; i < 6; i++ )
        plot( img, points[i], colors[i] );
    
    imshow( "", img );
    waitKey();
    
    /* Get a sub region of the original image, to plot our mean / canonical shape */
    Mat temp = Mat( img, Rect(580, 20, 300, 300) );
    temp     = Scalar( 220, 220, 220 );
    
    /* Apply general procrustes analysis to get the mean shape */
    Mat mean_mat;
    Procrustes proc;
    points = proc.generalizedProcrustes( points, mean_mat );
    
    /* The mean shape is normalized, thus in order to view it, we scale it and translate it a bit */
    vector<Point2f> mean_shape;
    mean_mat *= 600;
    mean_mat += Scalar( 150, 150 );
    mean_mat.reshape(2).copyTo( mean_shape );
    
    /* Plot out our image */
    for( Point2f point : mean_shape )
        circle( temp, point, 3, Scalar(0, 0, 255), 2);
    
    imshow( "", img );
    waitKey();

}

int main(int argc, const char * argv[]) {
    generalizedProcrustesTest();
    
    return 0;
}

