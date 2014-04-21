//
//  PatchModels.cpp
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 11/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "PatchModels.h"
#include "SimilarityTransform.h"


vector<Point2f> PatchModels::calcPeaks( const Mat& image, const vector<Point2f>& points, const Size ssize ) {
    int n = static_cast<int>(points.size());
    
    Mat points_mat = Mat(points).reshape( 1, 2 * n );
    Mat similarity = calcSimilarityTransform( points, reference );
    Mat inversed_similarity = inverseSimilarityTransform( similarity );
    
    vector<Point2f> trans_points = applySimilarity( inversed_similarity, points );
    
    tbb::parallel_for( 0, n, 1, [&]( int i ) {
        Size window_size = ssize + patches[i].patchSize();
    
        Mat A( 2, 3, CV_32F );
        similarity.colRange(0, 2).copyTo( A.colRange(0, 2) );
        
        A.at<float>(0, 2) = points[i].x - (A.at<float>(0, 0) * (window_size.width  - 1)
                                        +  A.at<float>(0, 1) * (window_size.height - 1)) / 2;
    
        A.at<float>(1, 2) = points[i].y - (A.at<float>(1, 0) * (window_size.width  - 1)
                                        +  A.at<float>(1, 1) * (window_size.height - 1)) / 2;
        

        Mat warped;
        warpAffine( image, warped, A, window_size, INTER_LINEAR + WARP_INVERSE_MAP );
        
        
        Mat response = patches[i].calcResponse( warped, true );
        Point max_loc;
        minMaxLoc( response, 0, 0, 0, &max_loc );
        trans_points[i] = Point2f( trans_points[i].x + max_loc.x - 0.5 * ssize.width,
                                   trans_points[i].y + max_loc.y - 0.5 * ssize.height);
        
    } );
    
    return applySimilarity( similarity, trans_points );
}


void PatchModels::save( string filename ) {
    FileStorage fs( filename, FileStorage::WRITE );
    fs << "reference"    << reference;
    fs << "no of patches" << static_cast<int>(patches.size());
    
    char temp[32];
    for( int i = 0; i < patches.size(); i++ ) {
        sprintf( temp, "patch-%04d", i );
        fs << temp << patches[i].P ;
    }
    fs.release();
}

void PatchModels::load( string filename ) {
    FileStorage fs( filename, FileStorage::READ );
    fs["reference"]   >> reference;
    
    int no_of_patches = 0;
    fs["no of patches"] >> no_of_patches;

    patches = vector<PatchModel>( no_of_patches );
    
    char temp[32];
    for( int i = 0; i < no_of_patches; i++ ){
        sprintf( temp, "patch-%04d", i );
        fs[temp] >> patches[i].P;
    }
    
    fs.release();
}



vector<Point2f> PatchModels::applySimilarity( const Mat& S, const vector<Point2f>& points ) {
    vector<Point2f> transformed_points ;
    Mat points_mat = Mat( points ).clone();
    
    cv::transform( points_mat, points_mat, S );
    points_mat.copyTo( transformed_points );
    
    return transformed_points;
}


Mat PatchModels::inverseSimilarityTransform( const Mat& S ) {
    Mat inversed( 2, 3, CV_32F );
    
    /* Inverse the rotation */
    Mat SR       = S.colRange( 0, 2 );
    Mat rotation = inversed.colRange(0, 2);
    cv::invert( SR, rotation );
    
    Mat translation = -rotation * S.col( 2 );
    translation.copyTo( inversed.col(2) );
    
    return inversed;
}

/**
 * Calculate the affine transformation matrix from the set of points to the previously found
 * canonical shape
 */
Mat PatchModels::calcSimilarityTransform( const vector<Point2f>& points, Mat& ref ) {
    Mat mat         = Mat( points ).clone();
    Scalar mu       = SimilarityTransform::recenter( mat );
    Mat rotation    = SimilarityTransform::rotateScaleAlign( ref, mat );
    
    /* Concatenate the rotation and translation to obtain rigid transformation matrix */
    Mat rigid_transform;
    
    float a = rotation.at<float>(0, 0);
    float b = rotation.at<float>(1, 0);
    
    /* since a = k cos(theta), and b = k sin(theta) */
    /* k^2 cos^2(theta) + k^2 sin^2(theta) = k^2 ( cos^2(theta) + sin^2(theta) ) */
    /*                                     = k^2 */
    float k             = sqrt( a * a + b * b );
    float theta         = atan2(b, a);
    float kcos_theta    = k * cosf( theta );
    float ksin_theta    = k * sinf( theta );
    
    rigid_transform = (Mat_<float>(2, 3) <<
                       kcos_theta, -ksin_theta, mu[0],
                       ksin_theta,  kcos_theta, mu[1]);

//    Mat translation = (Mat_<float>(2, 1) << mu[0], mu[1]);
//    hconcat( rotation, translation, rigid_transform );
    
    return rigid_transform;
}

/**
 * Return the number of patches used to train the Patch Models
 */
int PatchModels::noOfPatches() {
    return static_cast<int>(patches.size());
}

void PatchModels::train(const vector<MUCTLandmark>& data,
                        const vector<Point2f>& ref,
                        const Size patch_size,
                        const Size search_window_size,
                        const bool mirror,
                        const float variance,
                        const float lambda,
                        const float mu_init,
                        const int n_samples,
                        const bool visualize) {
    
    int n = static_cast<int>(ref.size());
    
    reference = Mat( ref );
    
    Size window_size = patch_size + search_window_size;
    patches.resize( n );
    
    
    int no_of_samples = static_cast<int>(data.size());
    
    
    tbb::tick_count begin = tbb::tick_count::now();
    tbb::task_group task;
    for( int i = 0; i < n; i++) {
        cout << "Training patch no: " << i << "/" << n << endl;
        
        vector<Mat> images( no_of_samples );
        tbb::parallel_for( 0, no_of_samples, 1, [&](int j) {

            Mat image   = imread( DIR_PATH + data[j].filename + ".jpg", CV_LOAD_IMAGE_GRAYSCALE );
            Mat A       = calcSimilarityTransform( data[j].points, reference );


            A.at<float>(0, 2) = data[j].points[i].x - (A.at<float>(0, 0) * (window_size.width  - 1)
                                                    +  A.at<float>(0, 1) * (window_size.height - 1)) / 2;
            
            A.at<float>(1, 2) = data[j].points[i].y - (A.at<float>(1, 0) * (window_size.width  - 1)
                                                    +  A.at<float>(1, 1) * (window_size.height - 1)) / 2;
            
            warpAffine( image, images[j], A, window_size, INTER_LINEAR + WARP_INVERSE_MAP );
        });
    
        task.run([=]() {
            patches[i].train( images, patch_size, variance, lambda, mu_init, n_samples, visualize );
        });
    }
    task.wait();
    
    tbb::tick_count end = tbb::tick_count::now();
    cout << (end - begin).seconds() << " seconds elapsed"  << endl;
}



/* Below is the original visualization code from
 * "Mastering OpenCV with Practical Computer Vision Projects"
 *   Copyright Packt Publishing 2012. */
#pragma visualize

void
draw_string(Mat img,                       //image to draw on
            const string text)             //text to draw
{
    Size size = getTextSize(text,FONT_HERSHEY_COMPLEX,0.6f,1,NULL);
    putText(img,text,Point(0,size.height),FONT_HERSHEY_COMPLEX,0.6f,
            Scalar::all(0),1,CV_AA);
    putText(img,text,Point(1,size.height+1),FONT_HERSHEY_COMPLEX,0.6f,
            Scalar::all(255),1,CV_AA);
}

float                                      //scaling factor
calc_scale(const Mat &X,                   //scaling basis vector
           const float width)              //width of desired shape
{
    int n = X.rows/2; float xmin = X.at<float>(0),xmax = X.at<float>(0);
    for(int i = 0; i < n; i++){
        xmin = min(xmin,X.at<float>(2*i));
        xmax = max(xmax,X.at<float>(2*i));
    }return width/(xmax-xmin);
}

int
calc_height(const Mat &X,
            const float scale)
{
    int n = X.rows/2;
    float ymin = scale*X.at<float>(1),ymax = scale*X.at<float>(1);
    for(int i = 0; i < n; i++){
        ymin = min(ymin,scale*X.at<float>(2*i+1));
        ymax = max(ymax,scale*X.at<float>(2*i+1));
    }return int(ymax-ymin+0.5);
}

void PatchModels::visualize() {
    //compute scale factor
    int width = 200;
    
    int no_of_patches = noOfPatches();
    
    float scale = calc_scale( reference, width );
    int height = calc_height( reference, scale );
    
    //compute image width
    int max_width = 0,max_height = 0;
    for(int i = 0; i < no_of_patches; i++){
        Size size   = patches[i].patchSize();
        max_width   = max(max_width,int(scale*size.width));
        max_height  = max(max_width,int(scale*size.height));
    }
    //create reference image
    Size image_size(width+4*max_width,height+4*max_height);
    Mat image(image_size.height,image_size.width,CV_8UC3);
    image = Scalar::all(255);
    
    vector<Point> points( no_of_patches );
    vector<Mat> P( no_of_patches );
    
    for(int i = 0; i < no_of_patches; i++){
        Mat I1, I2, I3;
        normalize( patches[i].P,I1,0,255,NORM_MINMAX);
        resize(I1,I2,Size(scale*I1.cols,scale*I1.rows));
        I2.convertTo(I3,CV_8U);
        cvtColor(I3,P[i],CV_GRAY2RGB);
        
        points[i] = Point(scale * reference.at<float>(2*i  ) +
                          image_size.width /2 - P[i].cols/2,
                          scale * reference.at<float>(2*i+1) +
                          image_size.height/2 - P[i].rows/2);
        Mat I = image(Rect(points[i].x,points[i].y,P[i].cols,P[i].rows));
        P[i].copyTo(I);
    }
    //animate
    namedWindow("patch model");
    int i = 0;
    
    while(1){
        Mat img = image.clone();
        Mat I = img(Rect(points[i].x,points[i].y,P[i].cols,P[i].rows));
        P[i].copyTo(I);
        rectangle(img,points[i],Point(points[i].x+P[i].cols,points[i].y+P[i].rows), CV_RGB(255,0,0),2,CV_AA);
        
        char str[256];
        sprintf(str,"patch %d",i);
        Size size = getTextSize(str,FONT_HERSHEY_COMPLEX,0.6f,1,NULL);
        putText( img, str, Point(0,size.height), FONT_HERSHEY_COMPLEX, 0.6f, Scalar::all(0), 1, CV_AA );

        
        
        imshow("patch model",img);
        int c = waitKey(0);
        if(c == 'q')break;
        else if(c == 'p')i++;
        else if(c == 'o')i--;
        if(i < 0)i = 0; else if(i >= no_of_patches)i = no_of_patches - 1;
    }
    
    destroyWindow("patch model");
}

#pragma end