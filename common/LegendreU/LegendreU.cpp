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

#include "LegendreU.h"

LegendreU::LegendreU() : _nmax(-1),
_arg(0.)
{
    
}


LegendreU::~LegendreU()
{
    
}

void LegendreU::Calc(int nmax, double arg)
{
    if (nmax < 0)
        nmax = 0;
    
    // Recompute whenever either input changed - with && a new nmax at an
    // unchanged arg (or vice versa) silently kept the stale Pn.
    if (_arg != arg || _nmax != nmax)
    {
        // init the vector
        Pn = Eigen::VectorXd::Zero(nmax+1);

        // known values for Pn(0) and Pn(1)
        Pn(0) = 1.;
        if (nmax >= 1) // at nmax == 0 there is no Pn(1) - writing it ran one
            Pn(1) = arg; // double past the vector and corrupted the heap

        for (int i=2; i < nmax+1; i++) {
            // recursion formula
            Pn(i) = ((2*(i-1)+1)*arg*Pn(i-1)-(i-1)*Pn(i-2))/(double)i;
        }
        _nmax = nmax;
        _arg = arg;
    }
    
}


Eigen::VectorXd LegendreU::Get()
{
    return Pn;
}

