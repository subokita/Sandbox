//
//  FaceDetector.cpp
//  NonRigidFaceTracking
//
//  Created by Saburo Okita on 17/04/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#include "FaceDetector.h"

void FaceDetector::save( string filename ) {
    FileStorage fs( filename, FileStorage::WRITE );
    fs << "reference"           << reference;
    fs << "detectorOffset"      << detectorOffset;
    fs << "cascadeXMLFilename"  << cascadeXMLFilename;
    fs.release();
}

void FaceDetector::load( string filename ) {
    FileStorage fs( filename, FileStorage::READ );
    fs["reference"     ]        >> reference;
    fs["detectorOffset"]        >> detectorOffset;
    fs["cascadeXMLFilename"]    >> cascadeXMLFilename;
    fs.release();
}

bool FaceDetector::enoughBoundedPoints( const vector<Point2f>& points, const Rect rect, const float frac ) {
    float count = 1.0f * count_if( points.begin(), points.end(), [=](Point2f point){
        return rect.contains( point );
    });
    return (count / points.size()) >= frac;
}

Point2f FaceDetector::centerOfMass( const vector<Point2f>& points ) {
    int n = static_cast<float>(points.size());
    Point2f total = accumulate( points.begin(), points.end(), Point2f(0, 0), [=](Point2f a, Point2f b){
        return a + b;
    });
    
    return Point2f( total.x / n, total.y / n );
}

float FaceDetector::calcScale( const vector<Point2f>& points, const Point2f center ) {
    Mat mat = Mat(points).clone();
    mat -= Scalar(center.x, center.y);
    mat = mat.reshape( 1, 2 * mat.rows );
    return reference.dot( mat ) / reference.dot( reference );
}


vector<vector<Point2f>> FaceDetector::detect( Mat& grayscale, vector<Rect>& faces, const float scale_factor, const int min_neighbors, const Size min_size) {
    if( classifier.empty() )
        classifier.load( cascadeXMLFilename );
    
    vector<vector<Point2f>> points;
    
    Mat equalized;
    equalizeHist( grayscale, equalized );
    
    classifier.detectMultiScale( equalized, faces, scale_factor, min_neighbors,
                                CV_HAAR_SCALE_IMAGE, min_size );

    if( faces.empty() )
        return points;
    
    for( Rect face : faces ) {
        Vec3f scale = detectorOffset * face.width;
        
        /* Apply scaling and translation transformation to our reference shape */
        Mat transform = (Mat_<float>(2, 3) <<
                         scale[2], 0, face.x + 0.5 * face.width  + scale[0],
                         0, scale[2], face.y + 0.5 * face.height + scale[1] );
        
        Mat mat;
        cv::transform( reference.reshape( 2 ), mat, transform );
        
        vector<Point2f> temp;
        mat.copyTo( temp );
        points.push_back( temp );
    }
    
    return points;
}

void FaceDetector::train( vector<MUCTLandmark> data, const string cascade_xml_filename,
                         const Mat& ref, const float frac, const float scale_factor,
                         const int min_neighbors, const Size min_size, bool visualize ) {
    
    reference = ref.clone();
    cascadeXMLFilename = cascade_xml_filename;
    classifier.load( cascadeXMLFilename );
    
    if(classifier.empty()) {
        cerr << "Unable to load cascade classifier" << endl;
        abort();
    }

    
    vector<float> offsets_x, offsets_y, offsets_z;
    
    tbb::tick_count begin = tbb::tick_count::now();

    int counter = 0;
    for( MUCTLandmark muct: data ){
        cout << counter++ << "/" << data.size() << endl;
        
        Mat image = imread( DIR_PATH + muct.filename + ".jpg", CV_LOAD_IMAGE_GRAYSCALE );
        Mat equalized;
        equalizeHist( image, equalized );
        
        vector<Rect> objects;
        /* Can't parallelize unless we create a new CascadeClassifier object, and load the xml file */
        /* which seems to be slower */
        classifier.detectMultiScale( equalized, objects, scale_factor, min_neighbors,
                                     CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, min_size );

        if( !objects.empty() ) {
            bool enough_bounded_points = enoughBoundedPoints( muct.points, objects[0], frac );
            
            if( enough_bounded_points ){
                Point2f center = centerOfMass( muct.points );
                float x = (center.x - (objects[0].x + 0.5 * objects[0].width)) / objects[0].width;
                float y = (center.y - (objects[0].y + 0.5 * objects[0].height)) / objects[0].height;
                float z = calcScale( muct.points, center ) / objects[0].width;
                
                offsets_x.push_back( x );
                offsets_y.push_back( y );
                offsets_z.push_back( z );
            }
            
            if( visualize ){
                namedWindow("");
                moveWindow("", 0, 0 );
                cvtColor( image, image, CV_GRAY2BGR );
                for( Rect object : objects )
                    rectangle( image, object, Scalar(0, 0, 255), 2);
                
                if( enough_bounded_points ) {
                    for( Point2f point : muct.points )
                        circle( image, point, 2, Scalar(0, 255, 0), 2 );
                }
                
                imshow( "", image );
                waitKey(10);
            }
            
        }
    }
    
    tbb::tick_count end = tbb::tick_count::now();
    cout << (end - begin).seconds() << " seconds elapsed"  << endl;
    
    /* Find the median value */
    stable_sort( offsets_x.begin(), offsets_x.end() );
    stable_sort( offsets_y.begin(), offsets_y.end() );
    stable_sort( offsets_z.begin(), offsets_z.end() );

    detectorOffset[0] = offsets_x[offsets_x.size()/2];
    detectorOffset[1] = offsets_y[offsets_y.size()/2];
    detectorOffset[2] = offsets_z[offsets_z.size()/2];
}