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

// This class computers the unassociated Legendre Polynoms

#ifndef __ambix_matrix_test__LegendreU__
#define __ambix_matrix_test__LegendreU__

#include <iostream>
#include <Eigen/Eigen>

class LegendreU {
    
public:
    // Fixed capacity, so recomputing for another order (maxre does it on the
    // audio thread) never touches the heap. Orders above kMaxOrder are clamped.
    static constexpr int kMaxOrder = 63;
    using Vector = Eigen::Matrix<double, Eigen::Dynamic, 1, 0, kMaxOrder + 1, 1>;

    LegendreU();
    ~LegendreU();
  
    void Calc(int nmax, double arg);
    
    const Vector& Get() const;
    
    Vector Pn;
    
private:
    
    int _nmax;

    double _arg;
  
};
#endif /* defined(__ambix_matrix_test__LegendreU__) */
