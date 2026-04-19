#include "Settings.h"

SettingsStore::SettingsStore()
{
    juce::PropertiesFile::Options opts;
    opts.applicationName     = "ambix_visualizer";
    opts.filenameSuffix      = "settings";
    opts.folderName          = "ambix";
    opts.osxLibrarySubFolder = "Application Support";
    opts.commonToAllUsers    = false;
    appProps.setStorageParameters (opts);
}

AppSettings SettingsStore::load()
{
    AppSettings s;
    auto* p = appProps.getUserSettings();
    if (p != nullptr)
    {
        s.listenPort              = p->getIntValue    ("listenPort",           s.listenPort);
        s.autoClearEnabled        = p->getBoolValue   ("autoClearEnabled",     s.autoClearEnabled);
        s.autoClearTimeoutSec     = p->getIntValue    ("autoClearTimeoutSec",  s.autoClearTimeoutSec);
        s.minDb                   = static_cast<float>(p->getDoubleValue ("minDb",               static_cast<double>(s.minDb)));
        s.colorMap                = LevelMapping::colorMapFromName (
                                        p->getValue   ("colorMap",
                                                       LevelMapping::colorMapToName (s.colorMap)));
        s.upperHemisphereOnly     = p->getBoolValue   ("upperHemisphereOnly",  s.upperHemisphereOnly);
        s.upperHemisphereFloorDeg = static_cast<float>(p->getDoubleValue (
                                         "upperHemisphereFloorDeg",
                                         static_cast<double>(s.upperHemisphereFloorDeg)));
        s.flipLeftRight           = p->getBoolValue   ("flipLeftRight",        s.flipLeftRight);
        s.currentTabIndex         = p->getIntValue    ("currentTabIndex",      s.currentTabIndex);
        s.sphereLocked            = p->getBoolValue   ("sphereLocked",         s.sphereLocked);
        s.sphereYawDeg            = static_cast<float>(p->getDoubleValue ("sphereYawDeg",   static_cast<double>(s.sphereYawDeg)));
        s.spherePitchDeg          = static_cast<float>(p->getDoubleValue ("spherePitchDeg", static_cast<double>(s.spherePitchDeg)));

        s.visualizerUuid          = p->getValue     ("visualizerUuid",       {});
        s.subscribedEncoderIds    = p->getValue     ("subscribedEncoderIds", {});

        const int puckSz = p->getIntValue ("puckSize", static_cast<int> (s.puckSize));
        s.puckSize = static_cast<AppSettings::PuckSize> (juce::jlimit (0, 2, puckSz));
    }

    // First-run bootstrap: assign a stable UUID.
    if (s.visualizerUuid.isEmpty())
        s.visualizerUuid = juce::Uuid().toString();

    return s;
}

juce::File SettingsStore::getFile() const
{
    // Mirror Options resolution without writing anything.
    juce::PropertiesFile::Options opts;
    opts.applicationName     = "ambix_visualizer";
    opts.filenameSuffix      = "settings";
    opts.folderName          = "ambix";
    opts.osxLibrarySubFolder = "Application Support";
    opts.commonToAllUsers    = false;
    return opts.getDefaultFile();
}

bool SettingsStore::fileExists() const
{
    return getFile().existsAsFile();
}

void SettingsStore::save (const AppSettings& s)
{
    auto* p = appProps.getUserSettings();
    if (p == nullptr) return;
    p->setValue ("listenPort",             s.listenPort);
    p->setValue ("autoClearEnabled",       s.autoClearEnabled);
    p->setValue ("autoClearTimeoutSec",    s.autoClearTimeoutSec);
    p->setValue ("minDb",                  static_cast<double>(s.minDb));
    p->setValue ("colorMap",               LevelMapping::colorMapToName (s.colorMap));
    p->setValue ("upperHemisphereOnly",    s.upperHemisphereOnly);
    p->setValue ("upperHemisphereFloorDeg",
                 static_cast<double> (s.upperHemisphereFloorDeg));
    p->setValue ("flipLeftRight",          s.flipLeftRight);
    p->setValue ("currentTabIndex",        s.currentTabIndex);
    p->setValue ("sphereLocked",           s.sphereLocked);
    p->setValue ("sphereYawDeg",           static_cast<double>(s.sphereYawDeg));
    p->setValue ("spherePitchDeg",         static_cast<double>(s.spherePitchDeg));
    p->setValue ("visualizerUuid",         s.visualizerUuid);
    p->setValue ("subscribedEncoderIds",   s.subscribedEncoderIds);
    p->setValue ("puckSize",               static_cast<int> (s.puckSize));
    p->saveIfNeeded();
}
