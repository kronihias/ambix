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

#ifndef __ambix_matrix_test__ShChebyshev__
#define __ambix_matrix_test__ShChebyshev__

#include <iostream>

#include <Eigen/Eigen>



using namespace std;

class ShChebyshev {
    
public:
    ShChebyshev();
    ~ShChebyshev();
    
    void Calc(int n_max, double phi); // calculate all sinus/cosinus components for given order
    
    void Get(Eigen::VectorXd &GetChebyshev);
    
    Eigen::VectorXd Chebyshev;
    
private:
    
    int _n; // n
    double _phi;
    
};

#endif /* defined(__ambix_matrix_test__ShChebyshev__) */
