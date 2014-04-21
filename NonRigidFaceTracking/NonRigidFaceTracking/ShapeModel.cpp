//
//  ShapeModel.cpp
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 10/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "ShapeModel.h"
#include "SimilarityTransform.h"


int ShapeModel::noOfPoints() {
    return shapeBasis.rows / 2;
}

void ShapeModel::save( string filename ) {
    FileStorage fs( filename, FileStorage::WRITE );
    fs << "parameterVec"    << parameterVec;
    fs << "shapeBasis"      << shapeBasis;
    fs << "paramVariance"   << paramVariance;
    fs.release();
}

void ShapeModel::load( string filename ) {
    FileStorage fs( filename, FileStorage::READ );
    fs["parameterVec"]   >> parameterVec;
    fs["shapeBasis"]     >> shapeBasis;
    fs["paramVariance"]  >> paramVariance;
    fs.release();
}

void ShapeModel::setIdentityParams() {
    parameterVec = Mat::zeros( paramVariance.rows,1,CV_32F);
    parameterVec.at<float>(0.0) = 1.0; /* First parameter is the scale */
}


vector<Point2f> ShapeModel::calcShape( const float width ) {
    if( parameterVec.empty() ) {
        setIdentityParams();
        parameterVec.at<float>(0) = calcScale(shapeBasis.col(0), width);
    }
    
    Mat shape_mat = shapeBasis * parameterVec;
    vector<Point2f> shape_points;
    shape_mat.reshape( 2 ).copyTo( shape_points );
    
    return shape_points;
}

/**
 * Apply generalized procrustes analysis to align all points to canonical shape
 */
Mat ShapeModel::procrustes(vector<Mat>& points_matrices, const int itol, const float ftol ) {
    /* Recenter the points */
    for( Mat& mat : points_matrices )
        SimilarityTransform::recenter( mat );
    
    Mat P, C_old;
    hconcat( points_matrices, P );
    
    const Mat div = Mat::ones( P.cols, 1, CV_32FC2 );
    
    for( int iter = 0; iter < itol; iter++ ) {
        /* Calculate the mean shape of all the points */
        Mat C = P * div / P.cols;
        normalize( C, C );
        
        if( iter > 0 && norm( C, C_old ) < ftol )
            break;
        
        /* well, remember the previous canonical shape in order for comparison in the next loop */
        C_old = C.clone();
        
        /* For each set of points (columns), rotate them to the current canonical shape */
        for( int i = 0; i < P.cols; i++ ){
            Mat R = SimilarityTransform::rotateScaleAlign( P.col(i), C );
            cv::transform( P.col(i), P.col(i), R );
        }
    }
    
    return P;
}


/**
 Perform Gram Schmidt orthonormalization
 */
Mat ShapeModel::gramSchmidt( const Mat& A ) {
    Mat res = A.clone();
    
    for( int i = 0; i < res.cols; i++ ) {
        Mat vi = res.col(i);
        normalize( vi, vi );
        
        for( int j = i+1; j < res.cols; j++ ) {
            Mat vj = res.col( j );
            vj = vj - vi * (vj.dot( vi ));
        }
    }
    return res;
}

/**
 Flatten a 2 channels matrix where each channel corresponds to x and y coordinates
 respectively, into a single channel matrix, where both x and y coordinates are position next to each
 other in a single row
 */
Mat ShapeModel::flatten( Mat& mat ){
    if( mat.channels() == 1 )
        return mat.clone();
    
    Mat res( mat.rows * 2, mat.cols, CV_32FC1 );
    for( int i = 0; i < mat.rows; i++ ) {
        float * ptr_temp_x  = res.ptr<float>(i * 2);
        float * ptr_temp_y  = res.ptr<float>(i * 2 + 1);
        Point2f * ptr_trans = mat.ptr<Point2f>(i);
        
        for( int j = 0; j < mat.cols; j++ ) {
            ptr_temp_x[j] = ptr_trans[j].x;
            ptr_temp_y[j] = ptr_trans[j].y;
        }
    }
    
    return res;
}

/**
 * Calculate the rigid basis transformation matrix for each point set
 */
Mat ShapeModel::calcRigidBasis( Mat& X ) {
    int N = X.cols;
    
    Mat R( X.rows, 4, CV_32FC2  );
    Mat mean_mat = X * Mat::ones( N, 1, CV_32FC2 ) / N;
    
    for( int i = 0; i < X.rows; i++ ) {
        Point2f mean_entry    = mean_mat.at<Point2f>( i );

        R.at<Point2f>( i, 0 ) = mean_entry;
        R.at<Point2f>( i, 1 ) = Point2f( -mean_entry.y, mean_entry.x );
        R.at<Point2f>( i, 2 ) = Point2f( 1.0, 0.0 );
        R.at<Point2f>( i, 3 ) = Point2f( 0.0, 1.0 );
    }
    
    /* Apply gram schmidt in order to get the orthonormalized R, i.e. R.t() * R = I  */
    return gramSchmidt( R );
}

/**
 * Train shape model by:
 * -finding the canonical shape, 
 * -rigid transformation basis, 
 * -applying PCA to reduce dimensionality
 * -calculating variance for each parameters
 */
void ShapeModel::train(const vector<MUCTLandmark> &landmarks, const int kmax, const float frac) {
    int no_of_samples = static_cast<int>(landmarks.size());
    int no_of_points  = static_cast<int>(landmarks[0].points.size());
    
    vector<Mat> temp( no_of_samples );
    for( int i = 0; i < temp.size(); i++ )
        temp[i] = Mat( landmarks[i].points ).clone();

    Mat X;
    hconcat( temp, X );
    X = flatten( X );

    /* Apply general procrustes analysis */
    Mat Y = procrustes( temp );
    Mat R = calcRigidBasis( Y );
    
    Y = flatten( Y );
    R = flatten( R );
    
    
    /* Perform SVD on the covariance matrix for PCA purposes */
    Mat dY = Y - R * R.t() * Y;
    SVD svd( dY * dY.t() );
    
    /* Find k numbers of eigenvalues / vectors to preserve */
    int m = MIN( MIN( kmax, no_of_samples - 1), no_of_points - 1 );
    
    /* ... first take m maximum number of eigenvalues to consider ... */
    float vsum = cv::sum( svd.w.rowRange(0, m ))[0];
    float v = 0;
    int k = 0;
    
    /* ... Then find K eigenvalues that covers around frac% (should be 95%) ... */
    for( k = 0; k < m; k++ ) {
        v += svd.w.at<float>(k);
        if( v / vsum >= frac ) {
            k++;
            break;
        }
    }
    
    if( k > m )
        k = m;

    /* Only extract the first k eigenvectors */
    Mat D = svd.u.colRange(0, k);
    

    /* Append the rigid and non rigid transformation to a single matrix */
    shapeBasis.create( 2 * no_of_points, 4 + k, CV_32F );
    R.copyTo( Mat(shapeBasis, Rect(0, 0, 4, 2 * no_of_points)) );
    D.copyTo( Mat(shapeBasis, Rect(4, 0, k, 2 * no_of_points)) );
    
    
    /* Calculate the variance */
    Mat Q = shapeBasis.t() * X;
    for( int i = 0; i < no_of_samples; i++ )
        Q.col(i) /= Q.at<float>(0, i);
    pow( Q, 2, Q );
    
    /* Store the variance, but only for the non rigid transformation derived from PCA */
    paramVariance = Mat( 4 + k, 1, CV_32F, Scalar(-1) );
    for( int i = 4; i < paramVariance.rows; i++ )
        paramVariance.at<float>(i) = cv::sum(Q.row(i))[0] / (no_of_samples-1);
}


/**
 * Calculate the parameter vector
 */
void ShapeModel::calcParams( const vector<Point2f>& points, const Mat weight, const float c_factor ){
    int n = static_cast<int>(points.size());
    
    if( weight.empty() ) {
        Mat points_mat = Mat(points).reshape( 1, 2 * n );
        
        /* Project the points to the shape basis to obtain the parameters */
        parameterVec = shapeBasis.t() * points_mat;
    }
    else {
        int K = shapeBasis.cols;
        Mat H = Mat::zeros( K, K, CV_32F );
        Mat g = Mat::zeros( K, 1, CV_32F );
        
        for( int i = 0; i < n; i++ ) {
            Mat v = shapeBasis( Rect(0, 2*i, K, 2) );
            float w = weight.at<float>(i);
            H += w * v.t() * v;
            g += w * v.t() * Mat(points[i]);
        }
        solve( H, g, parameterVec, DECOMP_SVD );
    }
    
    clamp( c_factor );
}

/**
 * Clamp the non rigid part of the parameters to 3 standard deviations 
 * (thus covering 99.7 of the normal distribution)
 */
void ShapeModel::clamp(const float c) {
    double scale = parameterVec.at<float>(0);
    
    
    for( int i = 0; i < paramVariance.rows; i++ ) {
        /* not clamping for rigid transformation obtained from Procrustes alignment */
        if( paramVariance.at<float>(i) < 0 )
            continue;
        
        float std_dev = c * sqrt( paramVariance.at<float>(i) );
        if( fabs( parameterVec.at<float>(i) / scale ) > std_dev ) {
            if( parameterVec.at<float>(i) > 0 )
                parameterVec.at<float>(i) = std_dev * scale;
            else
                parameterVec.at<float>(i) = -std_dev * scale;
        }
    }
}

#pragma visualize

float ShapeModel::calcScale(const Mat &X, const float width) {
    int n = X.rows/2;
    float xmin = X.at<float>(0),
          xmax = X.at<float>(0);
    
    for(int i = 0; i < n; i++){
        xmin = min( xmin, X.at<float>(2*i) );
        xmax = max( xmax, X.at<float>(2*i) );
    }
    
    return width/ ( xmax - xmin );
}

void ShapeModel::visualize() {
    int n = shapeBasis.rows / 2;
    
    float scale = calcScale( shapeBasis.col(0), 150 );
    float tranx = n * 150.0 / shapeBasis.col(2).dot( Mat::ones(2*n,1,CV_32F) );
    float trany = n * 150.0 / shapeBasis.col(3).dot( Mat::ones(2*n,1,CV_32F) );
    
    //generate trajectory of parameters
    vector<float> val;
    for(int i = 0; i < 50; i++)val.push_back(float(i)/50);
    for(int i = 0; i < 50; i++)val.push_back(float(50-i)/50);
    for(int i = 0; i < 50; i++)val.push_back(-float(i)/50);
    for(int i = 0; i < 50; i++)val.push_back(-float(50-i)/50);
    
    //visualise
    Mat img(300,300,CV_8UC3);
    namedWindow("shape model");
    while(1){
        
        for(int k = 4; k < shapeBasis.cols; k++){
            
            for(int j = 0; j < int(val.size()); j++){
                Mat param = Mat::zeros( shapeBasis.cols,1,CV_32F );
                param.at<float>(0) = scale;
                param.at<float>(2) = tranx;
                param.at<float>(3) = trany;
                param.at<float>(k) = scale * val[j] * 3.0 * sqrt(paramVariance.at<float>(k) );
                param.copyTo(parameterVec);
                
                img = Scalar::all(255);
                
                char str[256];
                sprintf(str,"mode: %d, val: %.5f sd",k-3,val[j]/3.0);
                Size size = getTextSize(str,FONT_HERSHEY_COMPLEX,0.6f,1,NULL);
                putText(img, str,Point(0,size.height),FONT_HERSHEY_COMPLEX,0.6f, Scalar::all(0),1, 1);
                
                vector<Point2f> shape = calcShape();
                for( Point2f point : shape )
                    circle(img, point, 2, Scalar( 0, 0, 0 ), 2 );
                
                imshow("shape model",img);
                if(waitKey(10) == 'q')
                    return;
            }
        }
    }
}
#pragma end