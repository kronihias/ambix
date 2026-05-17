/*
  ==============================================================================

   PresetManager — shared helper used by mcfx_graph and mcfx_mimoeq for managing
   a per-user "named presets" folder.

   The folder layout is:

       <userApplicationDataDirectory>/<pluginFolderName>/presets/<name>.json

   On macOS this resolves to ~/Library/Application Support/<pluginFolderName>/presets.
   The directory is created lazily — listPresets() never creates it, and the
   helper has no opinion on what JSON the host plugin puts in each file.

   The class is intentionally I/O-only: no JUCE Components, no UI. The host
   editor builds its own juce::PopupMenu from listPresets() and pipes the
   resulting juce::File into whatever loader/saver the plugin already has.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

class PresetManager
{
public:
    // pluginFolderName: subfolder name under userApplicationDataDirectory.
    // For example "mcfx_graph" or "mcfx_mimoeq". Stored verbatim — no extra
    // sanitisation, since this is hard-coded by each plugin's editor.
    explicit PresetManager (const juce::String& pluginFolderName);

    // Returns <userApplicationDataDirectory>/<pluginFolderName>/presets.
    // Does NOT create the directory — call ensurePresetDirExists() before
    // writing a file there, or rely on juce::File::create() at write time.
    juce::File getPresetDir() const;

    // Creates the preset directory if it doesn't exist. Returns true on
    // success (including the "already exists" case). Safe to call any time.
    bool ensurePresetDirExists() const;

    struct Entry
    {
        juce::String name;   // filename without .json extension
        juce::File   file;   // absolute path
    };

    // Enumerates *.json files in the preset directory and returns them sorted
    // alphabetically (case-insensitive) by display name. Returns an empty
    // vector if the directory does not yet exist.
    std::vector<Entry> listPresets() const;

    // Returns the path a preset named `name` *would* live at (after sanitise()).
    // Does not check whether the file exists.
    juce::File fileForName (const juce::String& name) const;

    // True if fileForName(name) exists on disk.
    bool exists (const juce::String& name) const;

    // Strip whitespace, reject empty results, replace path-unsafe characters
    // (/ \ : * ? " < > | and control chars) with '_'. Returns an empty string
    // if there's nothing usable left after stripping — callers should treat
    // that as an invalid name and re-prompt the user.
    static juce::String sanitise (const juce::String& rawName);

private:
    juce::String folderName_;
};
