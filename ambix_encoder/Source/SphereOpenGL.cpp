/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ==============================================================================
 */

#include "SphereOpenGL.h"
#include "../../common/JuceCompat.h"

using namespace ::juce::gl;

#define DEG2RAD 0.01745329252

double constrainAngle(double x){
    x = fmod(x + M_PI,2*M_PI);
    if (x < 0)
        x += 2*M_PI;
    return x - M_PI;
}

SphereOpenGL::SphereOpenGL() :
    sphere(0.9f, 12, 12),
    sphere_source(0.1f, 12, 12),
    sphere_source_small(0.05f, 12, 12)
{
    openGLContext.setRenderer (this);
    // Source-number labels are drawn directly in renderOpenGL using
    // pre-rendered textures, so JUCE doesn't need to composite anything
    // over the framebuffer.
    openGLContext.setComponentPaintingEnabled (false);
    openGLContext.setContinuousRepainting(true);
    openGLContext.attachTo (*this);

    setSize(240,240);
}

SphereOpenGL::~SphereOpenGL()
{
    openGLContext.detach();
}

void SphereOpenGL::renderOpenGL()
{
    OpenGLHelpers::clear (Colours::black.withAlpha (1.f));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float desktopScale = (float) openGLContext.getRenderingScale();
    glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));

    glLoadIdentity();
    glOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    GLfloat mat_specular[4]   = { 0.18f, 0.18f, 0.18f, 1.f };
    GLfloat mat_shininess[]   = { 40.f };
    GLfloat global_ambient[]  = { 0.2f, 0.2f, 0.2f, 1.f };
    GLfloat light0_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.f };
    GLfloat light0_diffuse[]  = { 0.9f, 0.9f, 0.9f, 1.f };
    GLfloat light0_specular[] = { 0.85f, 0.85f, 0.85f, 1.f };

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0);

    if (processor)
    {
        const int active = processor->getActiveSources();
        for (int i = 0; i < active; ++i)
        {
            const auto pos = processor->getSourceDisplayPos (i);
            const float az = pos.azDeg * (float) DEG2RAD;
            const float el = pos.elDeg * (float) DEG2RAD;
            const float x = 0.9f * sinf(az) * cosf(el);
            const float y = 0.9f * cosf(az) * cosf(el);
            const float z = 0.9f * sinf(el);

            // dot colour: yellow normally, orange when any finger is
            // currently dragging this source
            bool isDragging = false;
            for (const auto& kv : draggingByTouchIndex)
                if (kv.second == i) { isDragging = true; break; }
            if (isDragging)
                glColor4f (1.f, 0.6f, 0.f, 1.f);
            else
                glColor4f (1.f, 1.f, 0.f, 1.f);

            sphere_source.draw(x, y, z);
        }

        // In linked-mode multi-source case, draw a small marker at the centre
        // (azimuth_param/elevation_param) so the user can see the auto-spread
        // anchor.
        if (active > 1 && processor->isLinked())
        {
            const float azC = (processor->getParameter (Ambix_encoderAudioProcessor::AzimuthParam)   - 0.5f) * 360.f * (float) DEG2RAD;
            const float elC = (processor->getParameter (Ambix_encoderAudioProcessor::ElevationParam) - 0.5f) * 360.f * (float) DEG2RAD;
            const float x = 0.9f * sinf(azC) * cosf(elC);
            const float y = 0.9f * cosf(azC) * cosf(elC);
            const float z = 0.9f * sinf(elC);
            glColor4f(1.f, 0.f, 0.f, 0.5f);
            sphere_source_small.draw(x, y, z);
        }
    }

    // Draw the (translucent) sphere last so source dots stay visible at the
    // back hemisphere too.
    glColor4f(0.89f, 0.89f, 0.9f, 0.7f);
    sphere.draw(0, 0, 0);

    // Source-number labels: textured quads drawn at each source's projected
    // position. Done in GL (no JUCE paint cache) so they redraw with the
    // dots every frame, no timer needed.
    if (processor && ! labelTextures.isEmpty())
    {
        const int active = processor->getActiveSources();

        glDisable (GL_LIGHTING);
        glDisable (GL_DEPTH_TEST);
        glEnable (GL_TEXTURE_2D);
        glColor4f (1.f, 1.f, 1.f, 1.f); // texture's own ARGB drives colour

        // Quad half-size in clip space — small enough to read but not so
        // big it covers the dot. Tied to the sphere_source radius (0.1)
        // so the label scales with the visible source dots.
        constexpr float kLabelHalf = 0.085f;

        for (int i = 0; i < active && i < labelTextures.size(); ++i)
        {
            const auto pos = processor->getSourceDisplayPos (i);
            const float az = pos.azDeg * (float) DEG2RAD;
            const float el = pos.elDeg * (float) DEG2RAD;
            const float x = 0.9f * sinf (az) * cosf (el);
            const float y = 0.9f * cosf (az) * cosf (el);
            // Push slightly toward camera so the label sits on top of the
            // dot even with depth-test disabled (also fine if test re-
            // enabled later).
            const float z = 0.9f * sinf (el) + 0.05f;

            labelTextures.getUnchecked (i)->bind();
            // JUCE's OpenGLTexture::loadImage flips Y on upload (OpenGL's
            // standard texture origin is bottom-left while JUCE Image's is
            // top-left). Match by putting the image's top row at the top
            // of the quad — t increases upward in clip space too.
            glBegin (GL_QUADS);
                glTexCoord2f (0.f, 0.f); glVertex3f (x - kLabelHalf, y - kLabelHalf, z);
                glTexCoord2f (1.f, 0.f); glVertex3f (x + kLabelHalf, y - kLabelHalf, z);
                glTexCoord2f (1.f, 1.f); glVertex3f (x + kLabelHalf, y + kLabelHalf, z);
                glTexCoord2f (0.f, 1.f); glVertex3f (x - kLabelHalf, y + kLabelHalf, z);
            glEnd();
        }

        glBindTexture (GL_TEXTURE_2D, 0);
        glDisable (GL_TEXTURE_2D);
        glEnable (GL_DEPTH_TEST);
        glEnable (GL_LIGHTING);
    }
}

void SphereOpenGL::newOpenGLContextCreated()
{
    // Textures must be created with a live GL context — defer until here.
    buildLabelTextures();
}

void SphereOpenGL::openGLContextClosing()
{
    releaseLabelTextures();
}

void SphereOpenGL::buildLabelTextures()
{
    releaseLabelTextures();

    // One texture per possible source number. Pre-rendered with JUCE's
    // font into a small ARGB image, then uploaded via OpenGLTexture.
    constexpr int kTexSize = 64;
    const int maxLabels = Ambix_encoderAudioProcessor::kMaxSources;

    for (int n = 1; n <= maxLabels; ++n)
    {
        juce::Image img (juce::Image::ARGB, kTexSize, kTexSize, true);
        {
            juce::Graphics g (img);
            g.setColour (juce::Colours::black);
            g.setFont (juce::Font (juce::FontOptions { kTexSize * 0.55f, juce::Font::bold }));
            g.drawText (juce::String (n),
                        0, 0, kTexSize, kTexSize,
                        juce::Justification::centred);
        }
        auto* tex = new juce::OpenGLTexture();
        tex->loadImage (img);
        labelTextures.add (tex);
    }
}

void SphereOpenGL::releaseLabelTextures()
{
    labelTextures.clear();
}

namespace
{
    // The sphere is drawn at radius 0.9 in unit clip space with the viewport
    // matching the component size. The visible sphere radius in pixels is
    // therefore 0.9 × (smaller half-extent). Used by every screen↔angle
    // mapping below — replaces an old hardcoded `105.f` that only matched
    // the legacy 240×240 fixed editor.
    inline float sphereRadiusPx (const juce::Component& c) noexcept
    {
        return 0.9f * (float) juce::jmin (c.getWidth(), c.getHeight()) * 0.5f;
    }
}

void SphereOpenGL::mouseDown(const juce::MouseEvent &e)
{
    if (! processor) return;

    // Snapshot current global params for relative-mode drag fallback.
    _mPhi   = (processor->getParameter (Ambix_encoderAudioProcessor::AzimuthParam)   - 0.5f) * (float)(2.f * M_PI);
    _mTheta = (processor->getParameter (Ambix_encoderAudioProcessor::ElevationParam) - 0.5f) * (float)(2.f * M_PI);
    _mWidth = processor->getParameter (Ambix_encoderAudioProcessor::WidthParam)              * (float)(2.f * M_PI);

    const float sphereR = sphereRadiusPx (*this);

    if (! processor->isLinked())
    {
        // Pick the source closest to this touch / mouse position. Each
        // active touch source gets its own puck assignment so multiple
        // fingers can drag separate pucks at once.
        const int active = processor->getActiveSources();
        int best = 0;
        float bestD = 1e9f;
        for (int i = 0; i < active; ++i)
        {
            const auto pos = processor->getSourceDisplayPos (i);
            const float az = pos.azDeg * (float) DEG2RAD;
            const float el = pos.elDeg * (float) DEG2RAD;
            const float r_pixels = sphereR * cosf (el);
            const float sx = (float)getWidth() / 2.f + r_pixels * sinf (az);
            const float sy = (float)getHeight() / 2.f - r_pixels * cosf (az);
            const float dx = (float)e.getPosition().x - sx;
            const float dy = (float)e.getPosition().y - sy;
            const float d = dx*dx + dy*dy;
            if (d < bestD) { bestD = d; best = i; }
        }
        draggingByTouchIndex[e.source.getIndex()] = best;

        // Override the global-param snapshot with the picked source's
        // actual position so the absolute-mode hemisphere logic starts
        // coherent with where the puck is. (Note: this snapshot is
        // mouse-only — relative / Alt drags don't multi-touch.)
        const auto pos = processor->getSourceDisplayPos (best);
        _mPhi   = pos.azDeg * (float) DEG2RAD;
        _mTheta = pos.elDeg * (float) DEG2RAD;
    }

    if (e.mods.isAltDown() && processor->isLinked())
    {
        // Compute centre indicator screen position for width drag.
        const float r_pixels = sphereR * cosf (_mTheta);
        const float centerScreenX = (float)getWidth() / 2.f + r_pixels * sinf (_mPhi);
        const float centerScreenY = (float)getHeight() / 2.f - r_pixels * cosf (_mPhi);
        const float dx = (float)e.getPosition().x - centerScreenX;
        const float dy = (float)e.getPosition().y - centerScreenY;
        _dragStartDistToCenter = sqrtf (dx*dx + dy*dy);
        return;
    }
    mouseDrag(e);
}

void SphereOpenGL::mouseDrag(const juce::MouseEvent &e)
{
    if (! processor) return;

    const float sphereR = sphereRadiusPx (*this);

    if (e.mods.isAltDown() && processor->isLinked())
    {
        // Width drag
        const float r_pixels = sphereR * cosf (_mTheta);
        const float centerScreenX = (float)getWidth() / 2.f + r_pixels * sinf (_mPhi);
        const float centerScreenY = (float)getHeight() / 2.f - r_pixels * cosf (_mPhi);
        const float dx = (float)e.getPosition().x - centerScreenX;
        const float dy = (float)e.getPosition().y - centerScreenY;
        const float currentDist = sqrtf (dx*dx + dy*dy);
        const float deltaDist = currentDist - _dragStartDistToCenter;
        const float newWidth = jlimit (0.f, (float)(2 * M_PI),
                                        _mWidth + deltaDist / sphereR * (float)M_PI);
        setParameterNotifyingHost (processor, Ambix_encoderAudioProcessor::WidthParam,
                                   newWidth / (2.f * (float)M_PI));
        return;
    }

    float mPhi = _mPhi;
    float mTheta = _mTheta;

    if (e.mods.isRightButtonDown())
    {
        if (!e.mods.isCtrlDown())
            mPhi   = (float) constrainAngle (_mPhi   + (float) e.getDistanceFromDragStartX() / 180.f);
        if (!e.mods.isShiftDown())
            mTheta = (float) constrainAngle (_mTheta + (float) e.getDistanceFromDragStartY() / 200.f);
    }
    else
    {
        const float x = (float)e.getPosition().x - (float)this->getWidth() / 2.f;
        const float y = (float)e.getPosition().y - (float)this->getHeight() / 2.f;

        float r = sqrtf (x*x + y*y) / sphereR;
        const float phi = atan2f (y, x);

        if (!e.mods.isCtrlDown())
            mPhi = (float) constrainAngle (phi + M_PI_2);

        if (!e.mods.isShiftDown())
        {
            r = jmin (2.f, r);
            float mult = (_mTheta < 0.f) ? -1.f : 1.f;
            if (r < 1.f)
                mTheta = mult * (float) constrainAngle (acosf (r));
            else
                mTheta = mult * (float) constrainAngle (-acosf (2.f - r));
        }
    }

    if (processor->isLinked())
    {
        setParameterNotifyingHost (processor, Ambix_encoderAudioProcessor::AzimuthParam,
                                   mPhi   / (2.f * (float)M_PI) + 0.5f);
        setParameterNotifyingHost (processor, Ambix_encoderAudioProcessor::ElevationParam,
                                   mTheta / (2.f * (float)M_PI) + 0.5f);
    }
    else
    {
        // Route the drag to whichever puck THIS touch source picked up.
        auto it = draggingByTouchIndex.find (e.source.getIndex());
        if (it == draggingByTouchIndex.end()) return;
        const int draggingSource = it->second;
        setParameterNotifyingHost (processor,
                                   Ambix_encoderAudioProcessor::sourceParamIndex (draggingSource, Ambix_encoderAudioProcessor::SrcAz),
                                   jlimit (0.f, 1.f, mPhi   / (2.f * (float)M_PI) + 0.5f));
        setParameterNotifyingHost (processor,
                                   Ambix_encoderAudioProcessor::sourceParamIndex (draggingSource, Ambix_encoderAudioProcessor::SrcEl),
                                   jlimit (0.f, 1.f, mTheta / (2.f * (float)M_PI) + 0.5f));
    }
}

void SphereOpenGL::mouseUp(const juce::MouseEvent& e)
{
    draggingByTouchIndex.erase (e.source.getIndex());
}
