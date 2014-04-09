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

#include "ShNorm.h"

ShNorm::ShNorm() : _n(-1)
{
    
}

ShNorm::~ShNorm()
{
    
}

void ShNorm::Calc(int n_max, bool n3d)
{
    const double sqrt2 = sqrt(2);
    
    if (n_max != _n)
    {
        Nmn.resize((n_max+1)*(n_max+1));
        Nmn.setZero();
        
        Nmn(0) = 1;
        
        for (int n=1; n <= n_max; n++) { // order
            
            int n0 = n*(n+1); // index of m=0 coefficients
            
            if (!n3d)
                Nmn(n0) = 1; // SN3D
            else 
                Nmn(n0) = sqrt(2*n+1); // N3D
            
            
            // sqrt2 factor is not included in the recursion!
            Nmn(n0+1) = - sqrt2 * Nmn(n0) / sqrt((n+1)*n);
            
            Nmn(n0-1) = Nmn(n0+1); // mirror coefficient
            
            
            for (int m=2; m<=n; m++) { // degree
                
                Nmn(n0+m) = - Nmn(n0+m-1) / sqrt((n+m)*(n-m+1)); // recursion
                
                // set m<0 to the same symmetric value m>0 (N-mn = Nmn)
                Nmn(n0-m) = Nmn(n0+m);
            }
            
        }
        
        // std::cout << "Normalization vector size: " << Nmn.size() << std::endl << "Vector: " << std::endl << Nmn << std::endl;
        
        _n = n_max;
    }
    
}

void ShNorm::Get(Eigen::VectorXd &GetNmn)
{
    GetNmn = Nmn;
}