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

#include "AmbixEncoder.h"

#include "ambi_weight_lookup.h"
// #include "SphericalHarmonic/tools.h"

#define _USE_MATH_DEFINES
#include <math.h>

AmbixEncoder::AmbixEncoder() :
azimuth(0.5f),
elevation(0.5f),
size(0.f),
_azimuth(0.1f),
_elevation(0.1f),
_size(0.1f)
{
    // reset size
    ambi_gain.resize(AMBI_CHANNELS);
    _ambi_gain.resize(AMBI_CHANNELS);

    sph_h.Init(AMBI_ORDER);

    calcParams();
}

AmbixEncoder::~AmbixEncoder()
{

}

void AmbixEncoder::calcParams()
{
  // save last status
  _ambi_gain = ambi_gain;

  if (_azimuth != azimuth || _elevation != elevation || _size != size)
  {

    sph_h.Calc((azimuth-0.5f)*2*(float)M_PI, (elevation-0.5f)*2*(float)M_PI);


    //std::cout << "ambi_gain size: " << ambi_gain.size() << " Ymn size: " << Ymn.size() << " AMBI_CHANNELS: " << AMBI_CHANNELS << std::endl;


    for (int i=0; i < AMBI_CHANNELS; ++i) {
      ambi_gain.set(i, (float)sph_h.Ymn(i));
    }

    ///////////////////////////
    // apply size -> scaling of higher orders -> this is not really correct! more for sounddesign!
    if (size > 0.f) {
      Array<float> ambi_weight;

      ambi_weight.add(1.f); // w

      for (int i=1; i <= AMBI_ORDER; i++) {

        int id = (int) (size * (i * 128 / AMBI_ORDER + 128));

        id = (std::min)(128, id);
        id = (std::max)(0, id);

        // std::cout << "id: " << id << " weight: " << ambi_weight_lookup[id] << std::endl;

        ambi_weight.add(ambi_weight_lookup[id]);
      }

      for (int i = 1; i < AMBI_CHANNELS; i++) // don't touch w
      {
        int l = (int)sqrt(i);
        //std::cout << "l: " << l << std::endl;
        ambi_gain.set(i, ambi_gain[i] * ambi_weight.getUnchecked(l));
      }
    }

  }

  _azimuth = azimuth;
  _elevation = elevation;
  _size = size;

}
