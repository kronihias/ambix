# Overlay triplet for ambix: same as vcpkg's bundled x64-windows-static-md
# (static libs, dynamic CRT — matches JUCE plug-ins' /MD default) but skips
# the Debug variant. We never link against a Debug fftw3f.lib, so building it
# would just waste ~half of the vcpkg fftw3 build time.

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE          dynamic)
set(VCPKG_LIBRARY_LINKAGE      static)
set(VCPKG_BUILD_TYPE           release)
