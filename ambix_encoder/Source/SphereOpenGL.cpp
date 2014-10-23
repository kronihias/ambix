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

#include "SphereOpenGL.h"

#define DEG2RAD 0.01745329252
#define _3PI_4 3*M_PI/4

double constrainAngle(double x){
    x = fmod(x + M_PI,2*M_PI);
    if (x < 0)
        x += 2*M_PI;
    return x - M_PI;
}

SphereOpenGL::SphereOpenGL() :
sphere(0.9f, 12, 12),
sphere_source(0.1f, 12, 12),
sphere_source_small(0.05f, 12, 12),
_first_run(true)
{
    
    openGLContext.setRenderer (this);
    openGLContext.setComponentPaintingEnabled (false);
    openGLContext.setContinuousRepainting(true); // manually repaint ?    
    openGLContext.attachTo (*this);
    
    setSize(240,240);
    
    // startTimer(30); // repaint with timer?
}

SphereOpenGL::~SphereOpenGL()
{
    openGLContext.detach();
}

void SphereOpenGL::renderOpenGL()
{
    OpenGLHelpers::clear (Colours::black.withAlpha (1.f));
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    // glOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
    
    const float desktopScale = (float) openGLContext.getRenderingScale();
    
    glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
    
	// glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
	// glMatrixMode(GL_MODELVIEW);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    
    // glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    
    // set light parameters
    GLfloat mat_specular[4] = { 0.18f, 0.18f, 0.18f, 1.f };
    GLfloat mat_shininess[] = { 40.f };
    GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 1.f };
    GLfloat light0_ambient[] = { 0.0f, 0.0f, 0.0f, 1.f };
    GLfloat light0_diffuse[] = { 0.9f, 0.9f, 0.9f, 1.f };
    GLfloat light0_specular[] = { 0.85f, 0.85f, 0.85f, 1.f };
    
    
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0);
    
    
    
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);// todo include this into spotlight node
    
    
    
    // draw source
    glColor4f(1,1,0.0,1.0);
    
#if INPUT_CHANNELS < 2
    float x = 0.9f * sinf(mPhi) * cosf(mTheta);
    float y = 0.9f * cosf(mPhi) * cosf(mTheta);
    float z = 0.9f * sinf(mTheta);
    
    sphere_source.draw(x, y, z);
#else
    for (int i=0; i < INPUT_CHANNELS; i++)
    {
        // Convert Spherical to Cartesian coordinates: mPhi measured from +y
        // and mTheta measured counterclockwise from -z.
        
        float mPhi_this = mPhi - mWidth * (0.5f - (float)i/(float)(INPUT_CHANNELS-1));
        
        float x = 0.9f * sinf(mPhi_this) * cosf(mTheta);
        float y = 0.9f * cosf(mPhi_this) * cosf(mTheta);
        float z = 0.9f * sinf(mTheta);
        
        sphere_source.draw(x, y, z);
    }

# if INPUT_CHANNELS%2 == 0
    // draw small sphere at origin if even number of input channels
    float x = 0.9f * sinf(mPhi) * cosf(mTheta);
    float y = 0.9f * cosf(mPhi) * cosf(mTheta);
    float z = 0.9f * sinf(mTheta);
    glColor4f(1,0,0,0.5);
    sphere_source_small.draw(x, y, z);
#endif
    
#endif
    // draw sphere
    glColor4f(0.89f,0.89f,0.9f,0.7f);

    //glutSolidSphere(0.8, 50, 50);
    sphere.draw(0, 0, 0);
    
    
}

void SphereOpenGL::newOpenGLContextCreated()
{
    
    
}

void SphereOpenGL::openGLContextClosing()
{
    
}

void SphereOpenGL::setSource(float azimuth, float elevation, float width)
{
    mPhi = azimuth * (float)DEG2RAD;
    mTheta = elevation * (float)DEG2RAD;
    
    mWidth = width*2*M_PI;
    if (_first_run)
    {
        _mPhi = mPhi;
        _mTheta = mTheta;
        _mWidth = mWidth;
        
        _first_run = false;
    }
}

void SphereOpenGL::mouseDown(const juce::MouseEvent &e)
{
    mouseDrag(e);
}

void SphereOpenGL::mouseDrag(const juce::MouseEvent &e)
{
    
    if (e.mods.isRightButtonDown())
    {
        // Relative Mouse Mode
        
        // Azimuth
        if (!e.mods.isCtrlDown()) {
            mPhi = constrainAngle(_mPhi + (float) e.getDistanceFromDragStartX()/180.f);
        }
        
        // Elevation
        if (!e.mods.isShiftDown()) {
            mTheta = (float)constrainAngle(_mTheta + (float) e.getDistanceFromDragStartY()/200.f);
        }
    
    } else {
        // Absolute Mouse Mode
        
        float x = (float)e.getPosition().x - (float)this->getWidth() / 2.f;
        float y = (float)e.getPosition().y - (float)this->getHeight() / 2.f;
        
        //std::cout << x << " " << y << std::endl;
        
        float r = sqrtf(x*x + y*y)/105.f;
        float phi = atan2f(y, x);
        
        // Azimuth
        if (!e.mods.isCtrlDown()) {
            mPhi = (float)constrainAngle(phi + M_PI_2);
        }
        
        // Elevation
        if (!e.mods.isShiftDown()) {
            r = jmin(2.f, r);
            
            float mult = 1.f; // move on upper hemisphere
            
            if (_mTheta < 0.f)
                mult = -1.f; // move on lower hemisphere
            
            if (r < 1.f)
                mTheta = mult*(float)constrainAngle(acosf(r));
            else
                mTheta = mult*(float)constrainAngle(-acosf(2.f-r));
        }
        
    }
    
    // notify host
    if (processor)
    {
        processor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::AzimuthParam, mPhi/(2*(float)M_PI) + 0.5f);
        processor->setParameterNotifyingHost(Ambix_encoderAudioProcessor::ElevationParam, mTheta/(2*(float)M_PI) + 0.5f);
    }
}

void SphereOpenGL::mouseUp(const juce::MouseEvent &e)
{
    _mTheta = mTheta;
    _mPhi = mPhi;
}
