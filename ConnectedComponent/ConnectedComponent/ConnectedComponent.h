//
//  ConnectedComponent.h
//  RobustTextDetection
//
//  Created by Saburo Okita on 06/06/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __RobustTextDetection__ConnectedComponent__
#define __RobustTextDetection__ConnectedComponent__

#include <iostream>
#include <opencv2/opencv.hpp>


/**
 * Connected component labelling using 8-connected neighbors, based on
 * http://en.wikipedia.org/wiki/Connected-component_labeling
 *
 * with disjoint union and find functions adapted from :
 * https://courses.cs.washington.edu/courses/cse576/02au/homework/hw3/ConnectComponent.java
 */
class ConnectedComponent {
public:
    ConnectedComponent();
    virtual ~ConnectedComponent();
    cv::Mat apply( const cv::Mat& image );
    int getComponentsCount();
    
protected:
    void disjointUnion( uchar a, uchar b, std::vector<uchar>& parent  );
    uchar disjointFind( uchar a, std::vector<uchar>& parent, std::vector<uchar>& labels  );
    std::vector<uchar> getNeighbors( uchar * curr_ptr, uchar * prev_ptr, int x, int y, int cols );

private:
    uchar nextLabel;
    std::vector<uchar> labels;
};

#endif /* defined(__RobustTextDetection__ConnectedComponent__) */
