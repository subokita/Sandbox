//
//  main.cpp
//  ColorTransfer
//
//  Created by Saburo Okita on 19/08/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char * argv[])
{
    namedWindow("");
    moveWindow("", 0, 0);
    
    /* Load our images */
    const string path = "/Users/saburookita/Sandbox/ColorTransfer/images/";
    Mat source_img = imread( path + "Wind Waker.png" );
    Mat target_img = imread( path + "Phantom Pain.png" );
    
    
    /* Now convert them into CIE Lab color space */
    Mat source_img_cie,
        target_img_cie;
    
    cvtColor(source_img, source_img_cie, CV_BGR2Lab );
    cvtColor(target_img, target_img_cie, CV_BGR2Lab );
    
    
    /* Split into individual l a b channels */
    vector<Mat> source_channels,
                target_channels;
    
    split( source_img_cie, source_channels );
    split( target_img_cie, target_channels );
    
    /* For each of the l, a, b, channel ... */
    for( int i = 0; i < 3; i++ ) {
        /* ... find the mean and standard deviations */
        /* ... for source image ... */
        Mat temp_mean, temp_stddev;
        meanStdDev(source_channels[i], temp_mean, temp_stddev);
        double source_mean     = temp_mean.at<double>(0);
        double source_stddev   = temp_stddev.at<double>(0);
        
        /* ... and for target image */
        meanStdDev(target_channels[i], temp_mean, temp_stddev);
        double target_mean     = temp_mean.at<double>(0);
        double target_stddev   = temp_stddev.at<double>(0);
    
        /* Fit the color distribution from target LAB to our source LAB */
        target_channels[i].convertTo( target_channels[i], CV_64FC1 );
        target_channels[i] -= target_mean;
        target_channels[i] *= (target_stddev / source_stddev);
        target_channels[i] += source_mean;
        target_channels[i].convertTo( target_channels[i], CV_8UC1 );
    }
    
    
    /* Merge the lab channels back into a single BGR image */
    Mat output_img;
    merge(target_channels, output_img);
    cvtColor(output_img, output_img, CV_Lab2BGR );
    
    
    /* Append all the images together so that it looks like a triptych */
    int max_rows = MAX(source_img.rows, target_img.rows);
    Mat append( max_rows, source_img.cols + target_img.cols + output_img.cols, CV_8UC3, Scalar(0, 0, 0) );
    source_img.copyTo( Mat(append, Rect( Point2i(0, 0),                                 source_img.size() )) );
    target_img.copyTo( Mat(append, Rect( Point2i(source_img.cols, 0),                   target_img.size() )) );
    output_img.copyTo( Mat(append, Rect( Point2i(source_img.cols + target_img.cols, 0), output_img.size() )) );
    
    
    /* Uhh, try to fit my screen */
    Size screen_size(1366 * 0.98, 768 * 0.98);
    float resize_ratio = MIN( 1.0 * screen_size.width / append.cols, 1.0 * screen_size.height / append.rows  );
    resize( append, append, Size(), resize_ratio, resize_ratio );
    
    imshow("", append);
    while( waitKey(10) != 27 );
    
    return 0;
}

