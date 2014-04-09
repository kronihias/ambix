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

#include "ShLegendre.h"


ShLegendre::ShLegendre() : _n(-1), _arg(-4)
{
    
}


ShLegendre::~ShLegendre()
{
    
}


void ShLegendre::Calc(int n_max, double arg, double arg_2)
{
    
    if (_n != n_max || _arg != arg)
    {
        Legendre.resize((n_max+1)*(n_max+1));
        
        Legendre.setZero();
        
        Legendre(0) = 1; // P_0^0
        
        
        // computes the legendre functions P_n^m(arg) for m=n
        // from P_0^0
        // Zotter Diss page 19
        
        for (int n=1; n <= n_max; n++)
        {
            // P_n^m with m=n //correct!!
            Legendre((n+1)*(n+1)-1) = -(2*n-1) * Legendre(n*n-1) * arg_2;
            
            // P_n^m with m=n-1
            Legendre((n+1)*(n+1)-2) = (2*n-1) * arg * Legendre(n*n-1);
        }
        
        
        // rest P_n^m with 3<=n<=n_max and 0 <= m <= n-2
        for (int n=2; n <= n_max; n++)
        {
            for (int m=0; m <= n-2; m++)
            {
                if (m+2 <= n) {
                    Legendre(n*(n+1)+m) = ((2*n-1)*arg*Legendre((n)*(n-1)+m)-(n+m-1)*Legendre((n-2)*(n-1)+m))/(n-m);
                } else
                {
                    Legendre(n*(n+1)+m) = ((2*n-1)*arg*Legendre((n)*(n-1)+m))/(n-m);
                }
                
            }
        }
        
        // mirror coefficients for m<0
        for (int n=1; n <= n_max; n++)
        {
            int n0 = n*(n+1); // index of m=0 coefficients
            
            for (int m=1; m <= n; m++)
            {
                Legendre(n0-m) = Legendre(n0+m);
            }
        }
        
        // std::cout << "Legendre vector size: " << Legendre.size() << std::endl << "Vector: " << std::endl << Legendre << std::endl;
        _arg = arg;
        _n = n_max;
    }
    
}


void ShLegendre::Get(Eigen::VectorXd &GetLegendre)
{
    GetLegendre = Legendre;
}