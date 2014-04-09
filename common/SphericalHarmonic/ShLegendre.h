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

#ifndef __ambix_matrix_test__ShLegendre__
#define __ambix_matrix_test__ShLegendre__

#include <iostream>
#include <Eigen/Eigen>

class ShLegendre {
  
public:
    
    ShLegendre();
    ~ShLegendre();
    
    void Calc(int n_max, double arg, double arg_2); // calculate all legendre coefficients till given order
    // arg_2 is sqrt(1-arg^2) -> to make it behave like expected for elevations beneath the poles
  
    void Get(Eigen::VectorXd &GetLegendre);
    
    Eigen::VectorXd Legendre;
    
    
private:
    int _n;
    double _arg; // in case of ambix this would be sin(theta) because of using elevation (zero degree at equator)
    
};

#endif /* defined(__ambix_matrix_test__ShLegendre__) */
