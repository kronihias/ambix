#include "PresetManager.h"

PresetManager::PresetManager (const juce::String& pluginFolderName)
    : folderName_ (pluginFolderName)
{
}

juce::File PresetManager::getPresetDir() const
{
    // Matches the existing mcfx_graph convention (PluginListManager already
    // puts its plugin-list cache at userApplicationDataDirectory/mcfx_graph/).
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile (folderName_)
               .getChildFile ("presets");
}

bool PresetManager::ensurePresetDirExists() const
{
    auto dir = getPresetDir();
    if (dir.isDirectory()) return true;
    auto res = dir.createDirectory();
    return res.wasOk();
}

std::vector<PresetManager::Entry> PresetManager::listPresets() const
{
    std::vector<Entry> out;

    auto dir = getPresetDir();
    if (! dir.isDirectory()) return out;

    // Non-recursive — presets live flat in this folder by design. If a user
    // drops subfolders in there we ignore them rather than guessing intent.
    juce::Array<juce::File> files;
    dir.findChildFiles (files, juce::File::findFiles, /*recursive=*/false, "*.json");

    out.reserve ((size_t) files.size());
    for (const auto& f : files)
        out.push_back ({ f.getFileNameWithoutExtension(), f });

    std::sort (out.begin(), out.end(),
               [] (const Entry& a, const Entry& b)
               { return a.name.compareIgnoreCase (b.name) < 0; });

    return out;
}

juce::File PresetManager::fileForName (const juce::String& name) const
{
    return getPresetDir().getChildFile (sanitise (name) + ".json");
}

bool PresetManager::exists (const juce::String& name) const
{
    return fileForName (name).existsAsFile();
}

juce::String PresetManager::sanitise (const juce::String& rawName)
{
    auto trimmed = rawName.trim();
    if (trimmed.isEmpty()) return {};

    // Replace anything that's risky on any of the three supported OSes
    // (Windows is the strictest — disallows / \ : * ? " < > | plus controls).
    juce::String out;
    out.preallocateBytes ((size_t) trimmed.getNumBytesAsUTF8());

    for (auto c : trimmed)
    {
        const auto ch = (juce::juce_wchar) c;
        const bool unsafe =
               ch == '/' || ch == '\\' || ch == ':'
            || ch == '*' || ch == '?'  || ch == '"'
            || ch == '<' || ch == '>'  || ch == '|'
            || ch < 0x20;
        out += unsafe ? juce::juce_wchar ('_') : ch;
    }

    // After substitution the only thing left could still be all underscores —
    // that's fine, the user explicitly asked for it. Truly-empty is filtered
    // by the trim() check above.
    return out;
}
