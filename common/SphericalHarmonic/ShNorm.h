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

// Generate Spherical Harmonics Normalization Vector in ACN order

#ifndef __ambix_matrix_test__sh_normalization__
#define __ambix_matrix_test__sh_normalization__

#include <iostream>

#include <Eigen/Eigen>

using namespace std;

class ShNorm {
    
public:
    ShNorm();
    ~ShNorm();
    
    void Calc(int n_max, bool n3d=false); // calculate all normalization constants for given order
    // n3d = false -> sn3d, n3d= true -> n3d
    
    void Get(Eigen::VectorXd &GetNmn);

    Eigen::VectorXd Nmn;
    
private:
    
    int _n; // n
    

};

#endif /* defined(__ambix_matrix_test__sh_normalization__) */
