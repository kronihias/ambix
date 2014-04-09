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

#include "SphFilter.h"


SphFilter::SphFilter()
{
}

SphFilter::~SphFilter()
{
}

double SphFilter::GetWeight(Eigen::Vector2d* sph_coord, Eigen::Vector3d carth_coord, Eigen::Vector2d *sph_center, Eigen::Vector3d carth_center, int shape, float width, float height, double gain, bool window, float transition)
{
    double weight = 1.f;
    
    if (shape == 0) // circular shape
    {
        float angle = (float)carth_coord.dot(carth_center);
        angle = acos(angle);
        
        if (angle < width)
        {
            weight *= gain;
        } else {
            if (window)
                weight = 0.f;
        }
    } else {
        // rectangular shape
        
        // angular distance azimuth
        
        Eigen::Vector2d sph_az((*sph_coord)(0), 0.f); // map coordinate to el=0
        // convert to carth
        Eigen::Vector3d carth_az;
        carth_az(0) = cos(sph_az(0))*cos(sph_az(1)); // x
        carth_az(1) = sin(sph_az(0))*cos(sph_az(1)); // y
        carth_az(2) = sin(sph_az(1)); // z
        
        Eigen::Vector2d sph_c_az((*sph_center)(0), 0.f); // map center coordinate to el=0
        // convert to carth
        Eigen::Vector3d carth_c_az;
        carth_c_az(0) = cos(sph_c_az(0))*cos(sph_c_az(1)); // x
        carth_c_az(1) = sin(sph_c_az(0))*cos(sph_c_az(1)); // y
        carth_c_az(2) = sin(sph_c_az(1)); // z
        
        float angle_az = acosf((float)carth_az.dot(carth_c_az));
        
        // angular distance elevation
        
        Eigen::Vector2d sph_el((*sph_coord)(1), 0.f); // map coordinate to az=0
        // convert to carth
        Eigen::Vector3d carth_el;
        carth_el(0) = cos(sph_el(0))*cos(sph_el(1)); // x
        carth_el(1) = sin(sph_el(0))*cos(sph_el(1)); // y
        carth_el(2) = sin(sph_el(1)); // z
        
        Eigen::Vector2d sph_c_el((*sph_center)(1), 0.f); // map center coordinate to az=0
        // convert to carth
        Eigen::Vector3d carth_c_el;
        carth_c_el(0) = cos(sph_c_el(0))*cos(sph_c_el(1)); // x
        carth_c_el(1) = sin(sph_c_el(0))*cos(sph_c_el(1)); // y
        carth_c_el(2) = sin(sph_c_el(1)); // z
        
        float angle_el = acosf((float)carth_el.dot(carth_c_el));
        
        if (angle_az < width && angle_el < height)
        {
            weight *= gain;
        } else {
            if (window)
                weight = 0.f;
        }
    }
    
    
    return weight;
}