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

#ifndef __ambix_encoder__AmbixEncoder__
#define __ambix_encoder__AmbixEncoder__

#include <iostream>
#include "JuceHeader.h"

#include "SphericalHarmonic/SphericalHarmonic.h"

#include <Eigen/Eigen>

class AmbixEncoder {
    
public:
    AmbixEncoder();
    ~AmbixEncoder();
    
    
    void calcParams();
    
    
    // from 0.0 ..... 1.0   .... -180 -> +180
    float azimuth;
    float elevation;
    
    // from 0.0 .... 1.0   0: no scaling on HOA ...... 1: just W channel
    float size;
	
    Array<float> ambi_gain;  // actual gain
    Array<float> _ambi_gain; // buffer for gain ramp (last status)
  
    SphericalHarmonic sph_h;
  
private:
    
    float _azimuth, _elevation, _size; // buffer to realize changes
    
};

#endif /* defined(__ambix_encoder__AmbixEncoder__) */
