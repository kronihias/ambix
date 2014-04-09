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

#ifndef __ambix_matrix_test__SphericalHarmonic__
#define __ambix_matrix_test__SphericalHarmonic__

#include <iostream>
#include "ShNorm.h"
#include "ShLegendre.h"
#include "ShChebyshev.h"
#include <Eigen/Eigen>

class SphericalHarmonic {
    
public:
    SphericalHarmonic();
    ~SphericalHarmonic();
    
    void Init(int order, bool n3d=false, bool elevation_conv=false);
    
    void Calc(double phi, double theta);
    
    void Get(Eigen::VectorXd &GetYmn);
    
    Eigen::VectorXd Ymn;
    
private:
    
    int _order;
    bool _init;

    bool _elevation_conv; // elevation = false , zenith = true
    
    double _phi, _theta;
    
    ShNorm Norm;
    ShLegendre Legendre;
    ShChebyshev Chebyshev;
    
};
#endif /* defined(__ambix_matrix_test__SphericalHarmonic__) */
