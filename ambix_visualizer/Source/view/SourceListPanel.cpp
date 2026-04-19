#include "SourceListPanel.h"
#include "IconCatalogue.h"
#include "LevelMapping.h"

//==============================================================================
class SourceListPanel::ReplyPortCell : public juce::Component
{
public:
    explicit ReplyPortCell (SourceListPanel& owner) : panel (owner)
    {
        editor.setJustification (juce::Justification::centredLeft);
        editor.setInputRestrictions (6, "0123456789");
        editor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
        editor.setColour (juce::TextEditor::outlineColourId,   juce::Colour (0x00000000));
        editor.setColour (juce::TextEditor::focusedOutlineColourId,
                          juce::Colours::white.withAlpha (0.4f));
        editor.setColour (juce::TextEditor::textColourId, juce::Colours::white);
        editor.onReturnKey = [this]() { commit(); editor.giveAwayKeyboardFocus(); };
        editor.onFocusLost = [this]() { commit(); };
        addAndMakeVisible (editor);
    }

    void resized() override { editor.setBounds (getLocalBounds()); }

    void setSource (const EncoderSource& src)
    {
        key = src.key();
        origin = src.origin;

        const bool readOnly = (origin == EncoderOrigin::Ambix);
        editor.setReadOnly (readOnly);
        editor.setInterceptsMouseClicks (! readOnly, ! readOnly);
        editor.setColour (juce::TextEditor::textColourId,
                          readOnly ? juce::Colours::white.withAlpha (0.6f)
                                   : juce::Colours::white);

        if (! editor.hasKeyboardFocus (true))
        {
            const auto text = src.replyPort > 0 ? juce::String (src.replyPort)
                                                : juce::String();
            editor.setText (text, juce::dontSendNotification);
            if (origin == EncoderOrigin::MultiEncoder && src.replyPort <= 0)
                editor.setTextToShowWhenEmpty ("enter port",
                                               juce::Colours::white.withAlpha (0.35f));
            else
                editor.setTextToShowWhenEmpty ({}, juce::Colours::transparentBlack);
        }
    }

private:
    void commit()
    {
        if (editor.isReadOnly())
            return;
        const auto text = editor.getText().trim();
        const int p = text.isEmpty() ? 0 : text.getIntValue();
        panel.commitReplyPort (key, p);
    }

    SourceListPanel& panel;
    juce::TextEditor editor;
    EncoderSource::Key key;
    EncoderOrigin origin { EncoderOrigin::Ambix };
};

//==============================================================================
// Glyph-style icon picker: the cell shows the current icon; clicking it pops
// up a menu whose items are the icon glyphs themselves.
class SourceListPanel::IconCell : public juce::Component,
                                  public juce::SettableTooltipClient
{
public:
    explicit IconCell (SourceListPanel& owner) : panel (owner)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setSource (const EncoderSource& src)
    {
        key = src.key();
        currentIcon = src.icon;
        setTooltip ("Icon: " + IconCatalogue::toName (currentIcon));
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (3.0f);
        g.setColour (juce::Colour (0xFF1F1F1F));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        // Glyph takes the left square, "▾" chevron on the right.
        auto glyphArea = bounds.withTrimmedRight (16.0f).reduced (4.0f);
        const float side = juce::jmin (glyphArea.getWidth(), glyphArea.getHeight());
        glyphArea = glyphArea.withSizeKeepingCentre (side, side);

        if (currentIcon == IconCatalogue::Icon::Default)
        {
            g.setColour (juce::Colours::white.withAlpha (0.55f));
            g.setFont (juce::FontOptions (12.0f));
            g.drawText (juce::String (juce::CharPointer_UTF8 ("\xe2\x80\x93")),
                        glyphArea, juce::Justification::centred, false);
        }
        else
        {
            IconCatalogue::drawGlyph (g, currentIcon, glyphArea,
                                      juce::Colours::white.withAlpha (0.9f));
        }

        // chevron
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText (juce::String (juce::CharPointer_UTF8 ("\xe2\x96\xbe")),
                    bounds.removeFromRight (14.0f),
                    juce::Justification::centred, false);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        juce::PopupMenu menu;
        const auto names = IconCatalogue::getNames();
        for (int i = 0; i < names.size(); ++i)
        {
            const auto icon = IconCatalogue::fromName (names[i]);
            // 4th arg is the accessibility title (screen-reader label).
            menu.addCustomItem (i + 1,
                                std::make_unique<GlyphMenuItem> (icon, names[i]),
                                nullptr,
                                names[i]);
        }
        menu.showMenuAsync (juce::PopupMenu::Options()
                                .withTargetComponent (this)
                                .withMinimumWidth (64),
            [this, names] (int result)
            {
                if (result >= 1 && result <= names.size())
                {
                    const auto icon = IconCatalogue::fromName (names[result - 1]);
                    panel.commitIcon (key, icon);
                }
            });
    }

private:
    class GlyphMenuItem : public juce::PopupMenu::CustomComponent
    {
    public:
        // `isTriggeredAutomatically = true` (default) so JUCE dismisses the
        // menu and delivers the item id back to the caller on click. Earlier
        // this was `false`, which made the items uninteractive.
        GlyphMenuItem (IconCatalogue::Icon i, const juce::String& n)
            : icon (i), name (n) {}
        void getIdealSize (int& w, int& h) override { w = 120; h = 36; }
        void paint (juce::Graphics& g) override
        {
            if (isItemHighlighted())
                g.fillAll (juce::Colour (0x44FFFFFF));

            auto r = getLocalBounds().toFloat();
            auto glyph = r.removeFromLeft (36).reduced (6.0f);
            if (icon == IconCatalogue::Icon::Default)
            {
                g.setColour (juce::Colours::white.withAlpha (0.5f));
                g.setFont (juce::FontOptions (13.0f));
                g.drawText (juce::String (juce::CharPointer_UTF8 ("\xe2\x80\x93")),
                            glyph, juce::Justification::centred, false);
            }
            else
            {
                IconCatalogue::drawGlyph (g, icon, glyph, juce::Colours::white);
            }
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.setFont (juce::FontOptions (12.0f));
            g.drawText (name, r.reduced (6.0f, 0.0f),
                        juce::Justification::centredLeft, false);
        }
    private:
        IconCatalogue::Icon icon;
        juce::String name;
    };

    SourceListPanel& panel;
    EncoderSource::Key key;
    IconCatalogue::Icon currentIcon { IconCatalogue::Icon::Default };
};

//==============================================================================
// Row of small radio-style circles: one per group plus "none". Click a circle
// to assign the source to that group. Fast to assign many sources.
class SourceListPanel::GroupCell : public juce::Component,
                                   public juce::SettableTooltipClient
{
public:
    explicit GroupCell (SourceListPanel& owner) : panel (owner)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setSource (const EncoderSource& src,
                    const std::vector<GroupRegistry::Group>& groups)
    {
        key = src.key();
        assignedGroupId = src.groupId;
        options.clear();
        options.push_back ({ 0, "(none)" });
        for (const auto& g : groups)
            options.push_back ({ g.id,
                                 g.name.isEmpty()
                                    ? juce::String ("Group ") + juce::String (g.id)
                                    : g.name });
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const float radius = 9.0f;
        const float spacing = 24.0f;
        float x = 6.0f + radius;
        const float y = getHeight() * 0.5f;

        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));

        for (const auto& opt : options)
        {
            const bool selected = (opt.first == assignedGroupId);
            const juce::Rectangle<float> circle (x - radius, y - radius, radius * 2, radius * 2);

            if (selected)
            {
                g.setColour (juce::Colour (0xFF3B8FE0));
                g.fillEllipse (circle);
                g.setColour (juce::Colours::white);
            }
            else
            {
                g.setColour (juce::Colours::white.withAlpha (0.1f));
                g.fillEllipse (circle);
                g.setColour (juce::Colours::white.withAlpha (0.5f));
                g.drawEllipse (circle, 1.2f);
                g.setColour (juce::Colours::white.withAlpha (0.6f));
            }

            const auto label = (opt.first == 0) ? juce::String (juce::CharPointer_UTF8 ("\xe2\x80\x93")) // en-dash
                                                : juce::String (opt.first);
            g.drawText (label, circle, juce::Justification::centred, false);

            x += spacing;
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const float radius = 9.0f;
        const float spacing = 24.0f;
        float x = 6.0f + radius;
        const float y = getHeight() * 0.5f;
        for (const auto& opt : options)
        {
            const float dx = e.position.x - x;
            const float dy = e.position.y - y;
            if (std::sqrt (dx * dx + dy * dy) <= radius + 3.0f)
            {
                panel.commitGroup (key, opt.first);
                return;
            }
            x += spacing;
        }
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        // Update tooltip based on which circle we're over.
        const float radius = 9.0f;
        const float spacing = 24.0f;
        float x = 6.0f + radius;
        const float y = getHeight() * 0.5f;
        for (const auto& opt : options)
        {
            const float dx = e.position.x - x;
            const float dy = e.position.y - y;
            if (std::sqrt (dx * dx + dy * dy) <= radius + 3.0f)
            {
                setTooltip (opt.second);
                return;
            }
            x += spacing;
        }
        setTooltip ({});
    }

private:
    SourceListPanel& panel;
    EncoderSource::Key key;
    int assignedGroupId { 0 };
    std::vector<std::pair<int, juce::String>> options;
};

//==============================================================================
// Clickable cell showing the encoder's advertised project name (from NSD).
// Tap/click opens a popup that offers to create a group named after the
// project (and optionally sweep all sources sharing this project into it),
// or — if a group with the same name already exists — add to that one.
class SourceListPanel::ProjectCell : public juce::Component,
                                     public juce::SettableTooltipClient
{
public:
    explicit ProjectCell (SourceListPanel& owner) : panel (owner) {}

    void setSource (const EncoderSource& src, const juce::String& project)
    {
        key = src.key();
        currentProject = project;
        if (project.isNotEmpty())
        {
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
            setTooltip ("Create / assign group \"" + project + "\"");
        }
        else
        {
            setMouseCursor (juce::MouseCursor::NormalCursor);
            setTooltip ({});
        }
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto area = getLocalBounds().reduced (4, 0);
        g.setFont (juce::FontOptions (13.0f));
        if (currentProject.isNotEmpty())
        {
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawText (currentProject, area, juce::Justification::centredLeft, true);
        }
        else
        {
            // Same faint em-dash placeholder used before — lets the row read
            // "no project advertised yet" at a glance.
            g.setColour (juce::Colours::white.withAlpha (0.35f));
            g.drawText (juce::String::charToString ((juce::juce_wchar) 0x2014),
                        area, juce::Justification::centredLeft, false);
        }
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        if (currentProject.isEmpty())
            return;
        panel.showProjectGroupMenu (key, currentProject, this);
    }

private:
    SourceListPanel& panel;
    EncoderSource::Key key;
    juce::String currentProject;
};

//==============================================================================
// Small "-" button cell at the end of each row. Drops the source from the
// registry. The source reappears on the next broadcast from its encoder —
// the button is effectively "remove this stale row from the list".
class SourceListPanel::RemoveCell : public juce::Component,
                                    public juce::SettableTooltipClient
{
public:
    explicit RemoveCell (SourceListPanel& owner) : panel (owner)
    {
        button.setButtonText (juce::String::charToString ((juce::juce_wchar) 0x2212)); // "−"
        button.setTooltip ("Remove this source from the list. It'll reappear on "
                           "the next broadcast from its encoder.");
        button.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
        button.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF3A3A3A));
        button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFF5A2525));
        button.onClick = [this]() { panel.removeSource (key); };
        addAndMakeVisible (button);
    }

    void resized() override
    {
        // Compact, centered button so tapping is still easy on iOS while not
        // dominating the row.
        auto b = getLocalBounds().reduced (8, 4);
        const int side = juce::jmin (b.getHeight(), 32);
        b = b.withSizeKeepingCentre (side + 6, side);
        button.setBounds (b);
    }

    void setSource (const EncoderSource& src) { key = src.key(); }

private:
    SourceListPanel& panel;
    juce::TextButton button;
    EncoderSource::Key key;
};

//==============================================================================
SourceListPanel::SourceListPanel (SourceRegistry& r, GroupRegistry& gr,
                                  NetworkAdvertiser& adv)
    : registry (r), groupRegistry (gr), advertiser (adv)
{
    auto& header = table.getHeader();
    header.addColumn ("ID",      ColId,      40,  30, 80);
    header.addColumn ("Track",   ColName,    160, 60, 400);
    // Combined "Az / El" column — one compact cell replacing two. Puck
    // position is already the star of the 2D/3D views, so this is just a
    // quick numeric reference rather than the primary readout.
    header.addColumn ("Az / El", ColAzEl,     92,  70, 160);
    // Level sits right next to Az/El so the source's position + loudness
    // read as a single cluster of live numeric info.
    // Horizontal RMS + peak-hold segmented meter, Max/MSP-style. Only
    // populated for ambix_encoder sources (MultiEncoder doesn't carry
    // levels); MultiEncoder rows leave the cell blank.
    header.addColumn ("Level",   ColLevel,   160, 80, 360);
    // Project + Group get generous defaults now that Az/El/RMS/Peak are
    // gone — the live puck values are already visible in the 2D/3D views,
    // so duplicating them in the table was just eating horizontal space.
    header.addColumn ("Project", ColProject, 200, 60, 400);
    // Icon column narrowed — the glyph + chevron only need room for the
    // square + a tiny chevron.
    header.addColumn ("Icon",    ColIcon,    72,  56, 120);
    header.addColumn ("Group",   ColGroup,   200, 70, 360);
    header.addColumn ("Sender",  ColSender,  140, 80, 260);
    header.addColumn ("Reply",   ColReply,    80, 50, 140);
    header.addColumn ("",        ColRemove,   44, 40,  60);

    table.setModel (this);
    table.setRowHeight (28);
    table.getHeader().setStretchToFitActive (true);
    addAndMakeVisible (table);

    registry.addChangeListener (this);
    groupRegistry.addChangeListener (this);
    advertiser.addChangeListener (this);
    sourceCache  = registry.snapshot();
    groupCache   = groupRegistry.snapshot();
    encoderCache = advertiser.getEncoders();

    // 20 Hz is enough to animate the peak-hold decay smoothly without burning
    // cycles on the message thread. Gated on `isShowing()` inside the
    // callback so hidden tabs don't repaint.
    startTimerHz (20);
}

SourceListPanel::~SourceListPanel()
{
    stopTimer();
    registry.removeChangeListener (this);
    groupRegistry.removeChangeListener (this);
    advertiser.removeChangeListener (this);
}

void SourceListPanel::timerCallback()
{
    if (! isShowing())
        return;

    // Refresh caches lazily if a change arrived while we were hidden (or
    // was coalesced since the last paint).
    if (cachesDirty)
    {
        sourceCache  = registry.snapshot();
        groupCache   = groupRegistry.snapshot();
        encoderCache = advertiser.getEncoders();
        cachesDirty = false;
        const int expected = static_cast<int> (sourceCache.size());
        if (expected != table.getNumRows())
            table.updateContent();
    }
    // Drive meter animation (peak-hold decay between OSC ticks).
    table.repaint();
}

juce::String SourceListPanel::lookupProjectFor (const EncoderSource& s) const
{
    // Only ambix_encoder sources carry a project name via NSD; MultiEncoder
    // doesn't advertise at all.
    if (s.origin != EncoderOrigin::Ambix)
        return {};

    // Match on (senderIp, encoder's advertised listen port). The NSD record
    // carries the encoder's OSC listen port as `enc.port`, which matches the
    // visualizer's `replyPort` for the same plugin instance. Matching on
    // encoderId alone would fail whenever two DAWs each assign id=1 to their
    // first encoder on the same host — port is genuinely unique per instance.
    // Fall back to id-matching for rows with replyPort==0 (legacy 8-arg
    // /ambi_enc broadcasters that never advertise a port).
    for (const auto& enc : encoderCache)
    {
        if (enc.ip.toString() != s.senderIp)
            continue;
        if (s.replyPort > 0 ? (enc.port == s.replyPort)
                            : (enc.encoderId == s.id))
            return enc.project;
    }
    return {};
}

void SourceListPanel::resized() { table.setBounds (getLocalBounds()); }

int SourceListPanel::getNumRows() { return static_cast<int> (sourceCache.size()); }

void SourceListPanel::paintRowBackground (juce::Graphics& g, int rowNumber,
                                          int, int, bool rowIsSelected)
{
    const auto base = (rowNumber % 2 == 0) ? juce::Colour (0xFF262626)
                                           : juce::Colour (0xFF202020);
    g.fillAll (rowIsSelected ? base.brighter (0.15f) : base);
}

void SourceListPanel::paintCell (juce::Graphics& g, int rowNumber, int columnId,
                                 int width, int height, bool)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int> (sourceCache.size()))
        return;

    const auto& s = sourceCache[static_cast<size_t> (rowNumber)];

    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.setFont (juce::FontOptions (13.0f));

    auto area = juce::Rectangle<int> (4, 0, width - 8, height);

    switch (columnId)
    {
        case ColId:
            g.drawText (juce::String (s.getDisplayId()),
                        area, juce::Justification::centredLeft, false);
            break;
        case ColName:
            g.drawText (s.getDisplayLabel(), area, juce::Justification::centredLeft, true);
            break;
        case ColAzEl:
        {
            // "+123° / -45°" — azimuth can go to ±180, elevation only ±90
            // so 2 digits are enough there. Monospace for easy scanning;
            // degree sign via UTF-8 (U+00B0).
            auto fmt = [] (float deg, int digits)
            {
                const int rounded = juce::roundToInt (deg);
                // %+0Nd produces sign + zero-padded width-N-including-sign.
                const auto pattern = juce::String ("%+0")
                                   + juce::String (digits + 1) + "d";
                return juce::String::formatted (pattern.toRawUTF8(), rounded)
                     + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0"));
            };
            const auto txt = fmt (s.azimuthDeg,   3)
                           + juce::String (" / ")
                           + fmt (s.elevationDeg, 2);
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(),
                                          12.0f, juce::Font::plain));
            g.drawText (txt, area, juce::Justification::centredLeft, false);
            break;
        }
        case ColProject:
            break; // rendered by ProjectCell component
        case ColSender:
            g.drawText (s.senderIp, area, juce::Justification::centredLeft, true);
            break;
        case ColLevel:
            if (s.origin == EncoderOrigin::Ambix)
                drawLevelMeter (g, s, area);
            break;
        case ColIcon:
        case ColGroup:
        case ColReply:
        case ColRemove:
            break; // rendered by component cells
        default: break;
    }
}

void SourceListPanel::drawLevelMeter (juce::Graphics& g,
                                      const EncoderSource& s,
                                      juce::Rectangle<int> areaInt) const
{
    // Max/MSP-style segmented horizontal meter. Segments light up with the
    // RMS level; the peak-hold position is rendered as a single brighter
    // highlighted segment that decays via `peakHoldEffective` so transients
    // stay visible for ~150 ms hold + ~600 ms release.
    constexpr int   kNumSegments = 14;
    constexpr float kFloorDb     = -48.0f;
    constexpr float kCeilingDb   =  +3.0f;

    const auto bounds = areaInt.toFloat().reduced (2.0f, 6.0f);
    if (bounds.getWidth() < 20.0f || bounds.getHeight() < 4.0f)
        return;

    // Per-segment dB threshold: segment i lights once level >= thresh[i].
    auto thresholdDb = [&] (int seg) noexcept
    {
        const float t = static_cast<float> (seg + 1) / static_cast<float> (kNumSegments);
        return juce::jmap (t, kFloorDb, kCeilingDb);
    };

    // Classic green/yellow/orange/red ramp keyed on the threshold itself so
    // the colour of a segment doesn't change between rows — only whether
    // it's lit.
    auto segmentColour = [] (float thrDb) noexcept
    {
        if (thrDb >=  -1.0f) return juce::Colour (0xFFE04030); // red
        if (thrDb >=  -6.0f) return juce::Colour (0xFFE08A30); // orange
        if (thrDb >= -18.0f) return juce::Colour (0xFFE0C94B); // yellow
        return juce::Colour (0xFF52B03B);                      // green
    };

    const float rmsDb  = LevelMapping::linearToDb (s.rmsLinear);
    const float peakL  = LevelMapping::peakHoldEffective (s.peakHoldLinear,
                                                          s.peakHoldTimeMs,
                                                          juce::Time::getMillisecondCounter());
    const float peakDb = LevelMapping::linearToDb (peakL);

    const float gap    = 2.0f;
    const float segW   = (bounds.getWidth() - gap * (kNumSegments - 1)) / kNumSegments;
    const float segH   = bounds.getHeight();

    // Dim backdrop (unlit segment) and current colour (lit) are both taken
    // from the palette — unlit segments darken to ~18 % for a subtle
    // background cue without drawing attention away from the live level.
    for (int i = 0; i < kNumSegments; ++i)
    {
        const float thr = thresholdDb (i);
        const auto  col = segmentColour (thr);
        const juce::Rectangle<float> r (bounds.getX() + i * (segW + gap),
                                        bounds.getY(), segW, segH);

        const bool lit      = rmsDb  >= thr - 0.001f;
        const bool peakHere = i == juce::jlimit (0, kNumSegments - 1,
                                   static_cast<int> (std::floor (
                                       (peakDb - kFloorDb)
                                       / ((kCeilingDb - kFloorDb) / kNumSegments))));

        if (peakHere && peakL > 0.0005f)
        {
            // Peak-hold highlight: slightly brightened segment colour so it
            // pops out whether that segment would otherwise be lit or dark.
            g.setColour (col.brighter (0.6f));
            g.fillRect (r);
        }
        else if (lit)
        {
            g.setColour (col);
            g.fillRect (r);
        }
        else
        {
            g.setColour (col.withAlpha (0.18f));
            g.fillRect (r);
        }
    }
}

juce::Component* SourceListPanel::refreshComponentForCell (int rowNumber, int columnId,
                                                           bool, juce::Component* existing)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int> (sourceCache.size()))
    {
        delete existing;
        return nullptr;
    }
    const auto& src = sourceCache[static_cast<size_t> (rowNumber)];

    if (columnId == ColReply)
    {
        auto* cell = dynamic_cast<ReplyPortCell*> (existing);
        if (cell == nullptr) { delete existing; cell = new ReplyPortCell (*this); }
        cell->setSource (src);
        return cell;
    }
    if (columnId == ColIcon)
    {
        auto* cell = dynamic_cast<IconCell*> (existing);
        if (cell == nullptr) { delete existing; cell = new IconCell (*this); }
        cell->setSource (src);
        return cell;
    }
    if (columnId == ColGroup)
    {
        auto* cell = dynamic_cast<GroupCell*> (existing);
        if (cell == nullptr) { delete existing; cell = new GroupCell (*this); }
        cell->setSource (src, groupCache);
        return cell;
    }
    if (columnId == ColRemove)
    {
        auto* cell = dynamic_cast<RemoveCell*> (existing);
        if (cell == nullptr) { delete existing; cell = new RemoveCell (*this); }
        cell->setSource (src);
        return cell;
    }
    if (columnId == ColProject)
    {
        auto* cell = dynamic_cast<ProjectCell*> (existing);
        if (cell == nullptr) { delete existing; cell = new ProjectCell (*this); }
        cell->setSource (src, lookupProjectFor (src));
        return cell;
    }

    delete existing;
    return nullptr;
}

void SourceListPanel::commitReplyPort (const EncoderSource::Key& key, int port)
{
    registry.setReplyPort (key, port);
}

void SourceListPanel::commitIcon (const EncoderSource::Key& key, IconCatalogue::Icon icon)
{
    registry.setIcon (key, icon);
}

void SourceListPanel::commitGroup (const EncoderSource::Key& key, int groupId)
{
    registry.setGroup (key, groupId);
}

void SourceListPanel::removeSource (const EncoderSource::Key& key)
{
    registry.remove (key);
}

void SourceListPanel::showProjectGroupMenu (const EncoderSource::Key& clickedKey,
                                            const juce::String& project,
                                            juce::Component* target)
{
    if (project.isEmpty())
        return;

    // Does a group with this name already exist? Match is case-insensitive
    // on trim — DAW project names can have trailing whitespace after file
    // save/load round-trips and we don't want that to mint duplicates.
    int existingGroupId = 0;
    {
        const auto trimmed = project.trim();
        for (const auto& g : groupCache)
        {
            if (g.name.trim().equalsIgnoreCase (trimmed))
            {
                existingGroupId = g.id;
                break;
            }
        }
    }

    // Find every source currently advertising the same project, so the
    // "add all" options know what they're operating on.
    std::vector<EncoderSource::Key> matchingKeys;
    for (const auto& s : sourceCache)
    {
        if (lookupProjectFor (s) == project)
            matchingKeys.push_back (s.key());
    }
    const int matchCount = static_cast<int> (matchingKeys.size());

    juce::PopupMenu menu;
    menu.addSectionHeader ("Project: " + project);

    enum MenuId
    {
        IdCreateThis = 1,
        IdCreateAll,
        IdAddExistingThis,
        IdAddExistingAll
    };

    if (existingGroupId == 0)
    {
        menu.addItem (IdCreateThis,
                      "Create group \"" + project + "\" and add this source");
        menu.addItem (IdCreateAll,
                      "Create group \"" + project + "\" and add all "
                        + juce::String (matchCount) + " matching source"
                        + (matchCount == 1 ? "" : "s"),
                      matchCount > 0);
    }
    else
    {
        menu.addItem (IdAddExistingThis,
                      "Add this source to existing group \"" + project + "\"");
        menu.addItem (IdAddExistingAll,
                      "Add all " + juce::String (matchCount)
                        + " matching source" + (matchCount == 1 ? "" : "s")
                        + " to existing group \"" + project + "\"",
                      matchCount > 0);
    }

    juce::WeakReference<juce::Component> targetRef (target);
    // Capture by value — the menu outlives this call. `matchingKeys` and the
    // rest are cheap to copy.
    menu.showMenuAsync (juce::PopupMenu::Options()
                            .withTargetComponent (target),
        [this, clickedKey, project, existingGroupId, matchingKeys]
        (int result)
        {
            if (result == 0)
                return;

            const bool createNew = (existingGroupId == 0);
            const int groupId = createNew ? groupRegistry.add (project)
                                          : existingGroupId;

            const bool applyAll =
                (result == IdCreateAll || result == IdAddExistingAll);

            if (applyAll)
            {
                for (const auto& k : matchingKeys)
                    registry.setGroup (k, groupId);
            }
            else
            {
                registry.setGroup (clickedKey, groupId);
            }
        });
    juce::ignoreUnused (targetRef);
}

void SourceListPanel::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // While the Source List tab isn't visible, skip the snapshot + table
    // refresh. The 20 Hz timer will rebuild caches the moment we come back
    // into view. This cuts a large chunk of message-thread churn that was
    // contributing to the sporadic 2D/3D-view stutter.
    if (! isShowing())
    {
        cachesDirty = true;
        return;
    }

    sourceCache  = registry.snapshot();
    groupCache   = groupRegistry.snapshot();
    encoderCache = advertiser.getEncoders();
    cachesDirty = false;
    table.updateContent();
    table.repaint();
}
