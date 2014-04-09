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

#include "ShChebyshev.h"

#include "tools.h"


ShChebyshev::ShChebyshev() : _n(-1), _phi (-5)
{
    
    
}

ShChebyshev::~ShChebyshev()
{
    
    
}

void ShChebyshev::Calc(int n_max, double phi)
{
    if (_n != n_max || _phi != phi)
    {
        Eigen::VectorXd sinus(n_max+1); // temporary storage
        Eigen::VectorXd cosinus(n_max+1);
        
        sinus.setZero();
        cosinus.setZero();
        
        // start values
        sinus(0) = 0.f;
        cosinus(0) = 1.f;
        
        sinus(1) = -sin(phi); // for mtx_sph_h compatibility use -sin
        cosinus(1) = cos(phi);
        
        // chebyshev recursion for multiples of argument
        for (int i = 2; i <= n_max; i++) {
            
            cosinus(i) = 2 * cosinus(1) * cosinus(i-1) - cosinus(i-2);
            sinus(i) = 2 * cosinus(1) * sinus(i-1) - sinus(i-2);
            
        }
        
        // std::cout << "Sinus vector size: " << sinus.size() << std::endl << "Vector: " << std::endl << sinus << std::endl;
        
        // std::cout << "Cosinus vector size: " << cosinus.size() << std::endl << "Vector: " << std::endl << cosinus << std::endl;
        
        
        // put those into our ACN vector
        int size = (n_max+1)*(n_max+1);
        
        Chebyshev.resize(size);
        
        for (int acn=0; acn < size; acn++)
        {
            int n = 0; // sometimes also l
            int m = 0;
            
            ACNtoLM(acn, n, m);
            
            if (m < 0)
            {
                Chebyshev(acn) = sinus(-m);
            } else {
                Chebyshev(acn) = cosinus(m);
            }
            
        }
        
        // std::cout << "Chebyshev vector size: " << Chebyshev.size() << std::endl << "Vector: " << std::endl << Chebyshev << std::endl;
        
        _phi = phi;
        _n = n_max;
    }
    
}

void ShChebyshev::Get(Eigen::VectorXd &GetChebyshev)
{
    GetChebyshev = Chebyshev;
}