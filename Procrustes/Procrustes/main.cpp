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


int main(int argc, const char * argv[]) {
    namedWindow("");
    moveWindow("", 0, 0);
    
    
    /* First create random points X, centered at (250, 250) with stddev of 80 */
    Mat X(40, 1, CV_32FC2 );
    RNG rng;
    rng.fill( X, RNG::NORMAL, Scalar( 250, 250 ), Scalar( 80, 80 ) );

    /* Transform Y so that it's rotated and translated version of X */
    float angle = 10.0 * 180.0 / M_PI;
    Mat S = (Mat_<float>(2,2) << cosf(angle), -sinf(angle), sinf(angle), cosf( angle) );
    Mat Y;
    cv::transform( 0.5 * X, Y, S );
    Y += Scalar( 600, 100 );
    
    /* Jitter the Y points a bit, so that it's not exactly transformed version of X */
    Mat jitter( Y.size(), Y.type() );
    rng.fill( jitter, RNG::NORMAL, Scalar(0, 0), Scalar( 5, 5 ));
    Y += jitter;
    
    /* Plot X */
    vector<Point2f> X_points;
    X.copyTo( X_points );
    Mat img(450, 700, CV_8UC3, Scalar(255, 255, 255) );
    for( Point2f p: X_points )
        circle( img, p, 2, Scalar(0, 200, 0), 2);
    
    /* Plot Y */
    vector<Point2f> Y_points;
    Y.copyTo( Y_points );
    for( Point2f p: Y_points )
        circle( img, p, 2, Scalar(200, 0, 0), 2);
    
    imshow( "", img );
    waitKey();
    
    /*  Perform procrustes analysis, to obtain approximate transformed points from Y to X */
    Procrustes proc;
    proc.procrustes( X_points, Y_points );
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
    
    return 0;
}

