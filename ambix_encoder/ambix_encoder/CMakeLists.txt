SET ( WITH_EIGEN TRUE )
SET ( WITH_SphericalHarmonic TRUE )
SET ( WITH_MyMeterDsp TRUE )
SET ( WITH_LOCAL_NETWORK TRUE )  # enables NSLocalNetworkUsageDescription
SET ( WITH_DISCOVERY_HUB TRUE )   # process-wide NSD listener (multi-instance)

# MAX_INPUT_CHANNELS defaults to AMBI_CHANNELS (the SH coefficient count on
# the output bus) inside PluginProcessor.h — 36 for fixed 5th-order, 64 for
# the universal build. Override here only if you want to cap source count
# independently of the ambisonic order.
SET ( MAX_INPUT_CHANNELS_OVERRIDE "" CACHE STRING "Cap ambix_encoder max input channels (default = AMBI_CHANNELS)" )
IF (NOT MAX_INPUT_CHANNELS_OVERRIDE STREQUAL "")
    ADD_DEFINITIONS(-DMAX_INPUT_CHANNELS=${MAX_INPUT_CHANNELS_OVERRIDE})
ENDIF()

SET ( SPECIFIC_SOURE_DIR ${SRC_DIR}/ambix_encoder )

# VST2 fixed-order
IF(BUILD_VST)
	SET(UNIVERSAL_BUILD FALSE)
	SET( PLUGIN_CODE AE1${AMBI_ORDER})
	INCLUDE(${SRC_DIR}/Subproject.cmake)
ENDIF()

# VST3/AU universal
IF(BUILD_VST3 OR BUILD_AU)
	SET(UNIVERSAL_BUILD TRUE)
	SET( PLUGIN_CODE AE1U)
	INCLUDE(${SRC_DIR}/Subproject.cmake)
ENDIF()
