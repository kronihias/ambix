/*
 ==============================================================================
 
 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com
 
 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)
 
 Details of these licenses can be found at: www.gnu.org/licenses
 
 ambix is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 
 ==============================================================================
 */

#ifndef __ambix_directional_loudness__SphFilter__
#define __ambix_directional_loudness__SphFilter__

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
#include <Eigen/Eigen>

class SphFilter {

public:
    SphFilter();
    ~SphFilter();
    
    double GetWeight(Eigen::Vector2d *sph_coord, Eigen::Vector3d carth_coord, Eigen::Vector2d *sph_center, Eigen::Vector3d carth_center, int shape, float width, float height, double gain, bool window = false, float transition = 0.f);
    
    /*
     
     int shape; // 0: circular, 1: rectangular
     float width; // for circular and rectangular shape (in rad)
     float height; // only for rectangular shape (in rad)
     double gain; // gain returned to points inside shape
     bool window; // true: set points outside to zero
     float transition; // steepness of filter 0: rectagular, 1: raised cosine
     */
    
private:

    
};

#endif /* defined(__ambix_directional_loudness__SphFilter__) */
