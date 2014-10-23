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

#ifndef __ambix_encoder__SphereOpenGL__
#define __ambix_encoder__SphereOpenGL__

#include <iostream>
#include "PluginProcessor.h"
#include "JuceHeader.h"

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef __APPLE__
  #include <OpenGL/gl.h> //OS x libs
  #include <OpenGL/glu.h>
#else
    #ifdef _WIN32
      #include <windows.h>
    #endif
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

class SolidSphere
{
protected:
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texcoords;
    std::vector<GLushort> indices;
    
public:
    SolidSphere(float radius, unsigned int rings, unsigned int sectors)
    {
        float const R = 1.f/(float)(rings-1);
        float const S = 1.f/(float)(sectors-1);
        int r, s;
        
        vertices.resize(rings * sectors * 3);
        normals.resize(rings * sectors * 3);
        texcoords.resize(rings * sectors * 2);
        std::vector<GLfloat>::iterator v = vertices.begin();
        std::vector<GLfloat>::iterator n = normals.begin();
        std::vector<GLfloat>::iterator t = texcoords.begin();
        for(r = 0; r < (int)rings; r++) for(s = 0; s < (int)sectors; s++) {
            float const y = sinf( -(float)M_PI_2 + (float)M_PI * r * R );
            float const x = cosf(2*(float)M_PI * s * S) * sinf( (float)M_PI * r * R );
            float const z = sinf(2*(float)M_PI * s * S) * sinf( (float)M_PI * r * R );
            
            *t++ = s*S;
            *t++ = r*R;
            
            *v++ = x * radius;
            *v++ = y * radius;
            *v++ = z * radius;
            
            *n++ = x;
            *n++ = y;
            *n++ = z;
        }
        
        indices.resize(rings * sectors * 4);
        std::vector<GLushort>::iterator i = indices.begin();
        for(r = 0; r < (int)rings-1; r++) for(s = 0; s < (int)sectors-1; s++) {
            *i++ = r * sectors + s;
            *i++ = r * sectors + (s+1);
            *i++ = (r+1) * sectors + (s+1);
            *i++ = (r+1) * sectors + s;
        }
    }
    
    void draw(GLfloat x, GLfloat y, GLfloat z)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(x,y,z);
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        
        glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
        glNormalPointer(GL_FLOAT, 0, &normals[0]);
        glTexCoordPointer(2, GL_FLOAT, 0, &texcoords[0]);
        glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_SHORT, &indices[0]);
        glPopMatrix();
    }
};


class SphereOpenGL : public Component,
                    public OpenGLRenderer
{
    
public:
    SphereOpenGL();
    ~SphereOpenGL();
    
    //void paint (Graphics& g);
    
    void renderOpenGL();
    void newOpenGLContextCreated();
    void openGLContextClosing();
    
    void mouseDown (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    
    void setSource(float azimuth, float elevation, float width); // pass in degree!
    
    Ambix_encoderAudioProcessor* processor;
    
private:
    
    OpenGLContext openGLContext;
    
    SolidSphere sphere;
    SolidSphere sphere_source;
    SolidSphere sphere_source_small;
    
    float mPhi;
    float mTheta;
    
    float _mTheta; // old
    float _mPhi;
    
    float mWidth;
    float _mWidth;
    
    bool _first_run;
};



#endif /* defined(__ambix_encoder__SphereOpenGL__) */
