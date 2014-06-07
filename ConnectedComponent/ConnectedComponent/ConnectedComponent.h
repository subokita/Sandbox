//
//  ConnectedComponent.h
//  ConnectedComponent
//
//  Created by Saburo Okita on 06/06/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __RobustTextDetection__ConnectedComponent__
#define __RobustTextDetection__ConnectedComponent__

#include <iostream>
#include <opencv2/opencv.hpp>

/**
 * Structure that describes the property of the connected component
 */
struct ComponentProperty {
    int labelID;
    int area;
    float eccentricity;
    cv::Point2f centroid;

    friend std::ostream &operator <<( std::ostream& os, const ComponentProperty & prop ) {
        os << "     Label ID: " << prop.labelID      << "\n";
        os << "         Area: " << prop.area         << "\n";
        os << "     Centroid: " << prop.centroid     << "\n";
        os << " Eccentricity: " << prop.eccentricity << "\n";
        return os;
    }
};


/**
 * Connected component labeling using 8-connected neighbors, based on
 * http://en.wikipedia.org/wiki/Connected-component_labeling
 *
 * with disjoint union and find functions adapted from :
 * https://courses.cs.washington.edu/courses/cse576/02au/homework/hw3/ConnectComponent.java
 */
class ConnectedComponent {
public:
    ConnectedComponent( int max_component = 1000 );
    virtual ~ConnectedComponent();
    
    cv::Mat apply( const cv::Mat& image );
    
    int getComponentsCount();
    const std::vector<ComponentProperty>& getComponentsProperties();
    
protected:
    float calculateBlobEccentricity( const cv::Moments& moment );
    cv::Point2f calculateBlobCentroid( const cv::Moments& moment );
    
    void disjointUnion( int a, int b, std::vector<int>& parent  );
    int disjointFind( int a, std::vector<int>& parent, std::vector<int>& labels  );
    std::vector<int> getNeighbors( int * curr_ptr, int * prev_ptr, int x, int y, int cols );
    
private:
    int maxComponent;
    int nextLabel;
    std::vector<ComponentProperty> properties;
};

#endif /* defined(__RobustTextDetection__ConnectedComponent__) */
