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

#include "SphericalHarmonic.h"

SphericalHarmonic::SphericalHarmonic() : _order(-1),
_init(false),
_elevation_conv(0),
_phi(1111.f),
_theta(1111.f)
{
    
}


SphericalHarmonic::~SphericalHarmonic()
{
    
}



void SphericalHarmonic::Init(int order, bool n3d, bool elevation_conv)
{
    if (!_init || _order != order)
    {
        _init = false;
        // init
        Norm.Calc(order, n3d); // this has to be done just if order changes
        
        Legendre.Calc(order, sin(0.f), cos(0.f));
        
        Chebyshev.Calc(order, 0.f);
      
		Ymn = Eigen::VectorXd::Zero((order+1)*(order+1));
        _elevation_conv = elevation_conv;
        _order = order;
        _init = true;
    }
}


void SphericalHarmonic::Calc(double phi, double theta)
{
    if (_phi != phi && _theta != theta)
    {
        Eigen::VectorXd Nmn, Chb, Pmn;
        
        // sin -> ambix convention -> elevation
        // cos -> mtx_spherical_harmonics -> zenith
        
        if (!_elevation_conv)
            Legendre.Calc(_order, sin(theta), cos(theta));
        else
            Legendre.Calc(_order, cos(theta), sin(theta));
        
        Chebyshev.Calc(_order, phi);
        
        Norm.Get(Nmn);
        Legendre.Get(Pmn);
        Chebyshev.Get(Chb);
        
        Ymn = Nmn.cwiseProduct(Pmn).cwiseProduct(Chb);
    }
    
}


void SphericalHarmonic::Get(Eigen::VectorXd &GetYmn)
{
    GetYmn = Ymn;
}

