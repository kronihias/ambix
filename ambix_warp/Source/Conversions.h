/*
 ==============================================================================
 This file is part of the IEM plug-in suite.
 Author: Daniel Rudrich
 Copyright (c) 2018 - Institute of Electronic Music and Acoustics (IEM)
 https://iem.at

 The IEM plug-in suite is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 The IEM plug-in suite is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this software.  If not, see <https://www.gnu.org/licenses/>.
 ==============================================================================
 */

#pragma once

template <typename Type>
class Conversions
{
public:
    static Type radiansToDegrees (Type radians)
    {
        return radians * (Type (180) / Type (3.14159265358979323846264338327950288));
    }

    static Type degreesToRadians (Type degrees)
    {
        return degrees * (Type (3.14159265358979323846264338327950288) / Type (180));
    }

    static void cartesianToSpherical (const Type x,
                                      const Type y,
                                      const Type z,
                                      Type& azimuthInRadians,
                                      Type& elevationInRadians,
                                      Type& radius)
    {
        const float xSquared = x * x;
        const float ySquared = y * y;
        radius = sqrt (xSquared + ySquared + z * z);
        azimuthInRadians = atan2 (y, x);
        elevationInRadians = atan2 (z, sqrt (xSquared + ySquared));
    }

    static void cartesianToSpherical (const Type x,
                                      const Type y,
                                      const Type z,
                                      Type& azimuthInRadians,
                                      Type& elevationInRadians)
    {
        const float xSquared = x * x;
        const float ySquared = y * y;
        azimuthInRadians = atan2 (y, x);
        elevationInRadians = atan2 (z, sqrt (xSquared + ySquared));
    }

    static void cartesianToSpherical (const juce::Vector3D<Type> cartesian,
                                      Type& azimuthInRadians,
                                      Type& elevationInRadians)
    {
        cartesianToSpherical (cartesian.x,
                              cartesian.y,
                              cartesian.z,
                              azimuthInRadians,
                              elevationInRadians);
    }

    static juce::Vector3D<Type> cartesianToSpherical (juce::Vector3D<Type> cartvect)
    {
        const Type r = cartvect.length();
        return juce::Vector3D<Type> (
            r, // radius
            juce::radiansToDegrees (atan2 (cartvect.y, cartvect.x)), // azimuth
            juce::radiansToDegrees (
                atan2 (cartvect.z,
                       sqrt (cartvect.x * cartvect.x + cartvect.y * cartvect.y))) // elevation
        );
    }

    static juce::Vector3D<Type> sphericalToCartesian (const Type azimuthInRadians,
                                                      const Type elevationInRadians)
    {
        juce::Vector3D<Type> cartesian;
        sphericalToCartesian (azimuthInRadians,
                              elevationInRadians,
                              cartesian.x,
                              cartesian.y,
                              cartesian.z);
        return cartesian;
    }

    static juce::Vector3D<Type> sphericalToCartesian (const Type azimuthInRadians,
                                                      const Type elevationInRadians,
                                                      const Type radius)
    {
        return sphericalToCartesian (azimuthInRadians, elevationInRadians) * radius;
    }

    static void sphericalToCartesian (const Type azimuthInRadians,
                                      const Type elevationInRadians,
                                      const Type radius,
                                      Type& x,
                                      Type& y,
                                      Type& z)
    {
        Type _x;
        Type _y;
        Type _z;
        sphericalToCartesian (azimuthInRadians, elevationInRadians, _x, _y, _z);
        x = radius * _x;
        y = radius * _y;
        z = radius * _z;
    }

    static void sphericalToCartesian (const Type azimuthInRadians,
                                      const Type elevationInRadians,
                                      Type& x,
                                      Type& y,
                                      Type& z)
    {
        const Type cosElevation = cos (elevationInRadians);
        x = cosElevation * cos (azimuthInRadians);
        y = cosElevation * sin (azimuthInRadians);
        z = sin (elevationInRadians);
    }
};
