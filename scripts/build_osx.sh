#!/bin/bash
# Build ambix plugins for macOS
#
# Usage: ./build_osx.sh [--vst2] [--vst3] [--au] [--standalone] [--no-sign]
#        With no format flags, builds all formats.
#        --no-sign skips codesigning, notarization and stapling (for CI dry runs).

set -e

ROOT=$(cd "$(dirname "$0")/.."; pwd)
BUILD_DIR=$ROOT/build
VERSION=$(<"$ROOT/VERSION")

# =========================================================
# Parse arguments
# =========================================================

BUILD_VST2=false
BUILD_VST3=false
BUILD_AU=false
BUILD_STANDALONE=false
NO_SIGN=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --vst2)       BUILD_VST2=true ;;
        --vst3)       BUILD_VST3=true ;;
        --au)         BUILD_AU=true ;;
        --standalone) BUILD_STANDALONE=true ;;
        --no-sign)    NO_SIGN=true ;;
        *)            echo "Unknown option: $1"; echo "Usage: $0 [--vst2] [--vst3] [--au] [--standalone] [--no-sign]"; exit 1 ;;
    esac
    shift
done

# If no format flags given, build all
if ! $BUILD_VST2 && ! $BUILD_VST3 && ! $BUILD_AU && ! $BUILD_STANDALONE; then
    BUILD_VST2=true
    BUILD_VST3=true
    BUILD_AU=true
    BUILD_STANDALONE=true
fi

# Load codesigning credentials (only required when actually signing)
if ! $NO_SIGN; then
    CODESIGN_ENV="$ROOT/scripts/codesign.env"
    if [ ! -f "$CODESIGN_ENV" ]; then
        echo "Error: $CODESIGN_ENV not found. Create it with your codesigning credentials, or pass --no-sign."
        exit 1
    fi
    source "$CODESIGN_ENV"
fi

# =========================================================
# Helper functions
# =========================================================

codesign_bundles() {
    $NO_SIGN && return 0
    local dir="$1"
    local ext="$2"
    find "$dir" -type d -name "*.$ext" -print0 2>/dev/null | while IFS= read -r -d '' bundle; do
        codesign -s "$CODESIGN_APP" \
                 --deep --strict --force --verbose --timestamp --options=runtime \
                 "$bundle"
    done
}

# Build a simple installer from a single directory
build_installer() {
    local pkg_root="$1"
    local identifier="$2"
    local install_location="$3"
    local installer_name="$4"

    if $NO_SIGN; then
        pkgbuild --root "${pkg_root}" --identifier "${identifier}" --version ${VERSION} --install-location "${install_location}" "${installer_name}"
        return 0
    fi

    local unsigned="${BUILD_DIR}/$(basename "$installer_name" .pkg)_unsigned.pkg"

    pkgbuild --root "${pkg_root}" --identifier "${identifier}" --version ${VERSION} --install-location "${install_location}" "${unsigned}"

    productsign --sign "$CODESIGN_INSTALLER" "${unsigned}" "${installer_name}"
    rm -rf "${unsigned}"

    pkgutil --check-signature "${installer_name}"

    notarize_installer "${installer_name}"
}

# Build a combined installer from a plugins directory + the visualizer app
build_installer_with_visualizer() {
    local pkg_root="$1"
    local identifier="$2"
    local install_location="$3"
    local installer_name="$4"

    local plugins_pkg="${BUILD_DIR}/component_plugins.pkg"
    local visualizer_pkg="${BUILD_DIR}/component_visualizer.pkg"

    pkgbuild --root "${pkg_root}" --identifier "${identifier}" --version ${VERSION} --install-location "${install_location}" "${plugins_pkg}"
    # Generate component plist and disable relocation so the app always installs to /Applications
    local comp_plist="${BUILD_DIR}/visualizer_component.plist"
    pkgbuild --analyze --root "${VISUALIZER_DIR}" "${comp_plist}"
    /usr/libexec/PlistBuddy -c "Set :0:BundleIsRelocatable false" "${comp_plist}"
    pkgbuild --root "${VISUALIZER_DIR}" --identifier "com.kronlachner.ambix.visualizer" --version ${VERSION} --install-location "/Applications" --component-plist "${comp_plist}" "${visualizer_pkg}"
    rm -f "${comp_plist}"

    if $NO_SIGN; then
        productbuild --package "${plugins_pkg}" --package "${visualizer_pkg}" "${installer_name}"
        rm -f "${plugins_pkg}" "${visualizer_pkg}"
        return 0
    fi

    local combined_unsigned="${BUILD_DIR}/$(basename "$installer_name" .pkg)_unsigned.pkg"

    productbuild --package "${plugins_pkg}" --package "${visualizer_pkg}" "${combined_unsigned}"
    rm -f "${plugins_pkg}" "${visualizer_pkg}"

    productsign --sign "$CODESIGN_INSTALLER" "${combined_unsigned}" "${installer_name}"
    rm -f "${combined_unsigned}"

    pkgutil --check-signature "${installer_name}"

    notarize_installer "${installer_name}"
}

notarize_installer() {
    $NO_SIGN && return 0
    local installer_name="$1"

    echo ""; echo "notarizing $(basename "$installer_name")"
    xcrun notarytool submit "${installer_name}" --apple-id "$NOTARIZE_APPLE_ID" --password "$NOTARIZE_PASSWORD" --team-id "$NOTARIZE_TEAM_ID" --wait
    xcrun stapler staple "${installer_name}"

    echo ""; echo "verifying $(basename "$installer_name")"
    stapler validate "${installer_name}"
    spctl -a -vvv --assess --type install "${installer_name}"
}

# =========================================================
# Clean and prepare
# =========================================================

rm -rf "$BUILD_DIR" 2> /dev/null || sudo rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
mkdir -p "$ROOT/_OSX_RELEASE"

# =========================================================
# Build ambix_visualizer (included in VST2 and VST3 installers)
# =========================================================

VISUALIZER_DIR=$BUILD_DIR/standalone

if $BUILD_VST2 || $BUILD_VST3; then
    echo ""; echo "=== Building ambix_visualizer v$VERSION ==="

    pushd "$BUILD_DIR"
    cmake .. -G Ninja \
        -DNUM_OUTPUTS_DECODER=64 \
        -DBUILD_VST=FALSE \
        -DBUILD_VST3=FALSE \
        -DBUILD_AU=FALSE \
        -DBUILD_STANDALONE=FALSE \
        -DBUILD_VISUALIZER=TRUE
    ninja ambix_visualizer
    popd

    echo ""; echo "codesigning ambix_visualizer"
    codesign_bundles "$VISUALIZER_DIR" "app"
fi

# =========================================================
# VST2 — one build per ambisonic order (+ visualizer)
# =========================================================

if $BUILD_VST2; then
    for order in 1 3 5 7; do
        VST_DIR=$BUILD_DIR/vst_o${order}

        echo ""; echo "=== Building VST2 order $order v$VERSION ==="

        pushd "$BUILD_DIR"
        cmake .. -G Ninja \
            -DAMBI_ORDER:STRING=${order} \
            -DNUM_OUTPUTS_DECODER=64 \
            -DBUILD_VST=TRUE \
            -DBUILD_VST3=FALSE \
            -DBUILD_AU=FALSE \
            -DBUILD_STANDALONE=FALSE \
            -DBUILD_VISUALIZER=FALSE
        ninja
        popd

        echo ""; echo "codesigning VST2 order $order"
        codesign_bundles "$VST_DIR" "vst"

        INSTALLER=${ROOT}/_OSX_RELEASE/ambix_v${VERSION}_macos_vst2_o${order}.pkg
        build_installer_with_visualizer "$VST_DIR" "com.kronlachner.ambix.vst2.o${order}" "/Library/Audio/Plug-Ins/VST/ambix" "$INSTALLER"
    done
fi

# =========================================================
# VST3 — single universal build (orders 1-7) (+ visualizer)
# =========================================================

if $BUILD_VST3; then
    VST3_DIR=$BUILD_DIR/vst3

    echo ""; echo "=== Building VST3 universal v$VERSION ==="

    pushd "$BUILD_DIR"
    cmake .. -G Ninja \
        -DNUM_OUTPUTS_DECODER=64 \
        -DBUILD_VST=FALSE \
        -DBUILD_VST3=TRUE \
        -DBUILD_AU=FALSE \
        -DBUILD_STANDALONE=FALSE \
        -DBUILD_VISUALIZER=FALSE
    ninja
    popd

    echo ""; echo "codesigning VST3 plugins"
    codesign_bundles "$VST3_DIR" "vst3"

    INSTALLER=${ROOT}/_OSX_RELEASE/ambix_v${VERSION}_macos_vst3.pkg
    build_installer_with_visualizer "$VST3_DIR" "com.kronlachner.ambix.vst3" "/Library/Audio/Plug-Ins/VST3/ambix" "$INSTALLER"
fi

# =========================================================
# AudioUnit — single universal build (orders 1-7)
# =========================================================

if $BUILD_AU; then
    AU_DIR=$BUILD_DIR/au

    echo ""; echo "=== Building AudioUnit universal v$VERSION ==="

    pushd "$BUILD_DIR"
    cmake .. -G Ninja \
        -DNUM_OUTPUTS_DECODER=64 \
        -DBUILD_VST=FALSE \
        -DBUILD_VST3=FALSE \
        -DBUILD_AU=TRUE \
        -DBUILD_STANDALONE=FALSE \
        -DBUILD_VISUALIZER=FALSE
    ninja
    popd

    echo ""; echo "codesigning AudioUnit plugins"
    codesign_bundles "$AU_DIR" "component"

    INSTALLER=${ROOT}/_OSX_RELEASE/ambix_v${VERSION}_macos_au.pkg
    build_installer "$AU_DIR" "com.kronlachner.ambix.au" "/Library/Audio/Plug-Ins/Components/ambix" "$INSTALLER"
fi

# =========================================================
# Standalone — single universal build (orders 1-7)
# =========================================================

if $BUILD_STANDALONE; then
    STANDALONE_DIR=$BUILD_DIR/standalone

    echo ""; echo "=== Building Standalone universal v$VERSION ==="

    pushd "$BUILD_DIR"
    cmake .. -G Ninja \
        -DNUM_OUTPUTS_DECODER=64 \
        -DBUILD_VST=FALSE \
        -DBUILD_VST3=FALSE \
        -DBUILD_AU=FALSE \
        -DBUILD_STANDALONE=TRUE \
        -DBUILD_VISUALIZER=TRUE
    ninja
    popd

    echo ""; echo "codesigning Standalone apps"
    codesign_bundles "$STANDALONE_DIR" "app"

    INSTALLER=${ROOT}/_OSX_RELEASE/ambix_v${VERSION}_macos_standalone.pkg
    build_installer "$STANDALONE_DIR" "com.kronlachner.ambix.standalone" "/Applications/ambix" "$INSTALLER"
fi

# =========================================================
echo ""
echo "Done! Installers:"
ls -la "${ROOT}/_OSX_RELEASE/"*.pkg 2>/dev/null || echo "(none)"
