SET ( WITH_EIGEN TRUE )
SET ( WITH_SphericalHarmonic TRUE )
SET ( WITH_MyMeterDsp TRUE )

# Maximum number of input source channels supported by the unified binary.
# Active count is selectable at runtime via NumActiveSourcesParam.
SET ( MAX_INPUT_CHANNELS 8 CACHE STRING "Max input channels for ambix_encoder" )
ADD_DEFINITIONS(-DMAX_INPUT_CHANNELS=${MAX_INPUT_CHANNELS})

# INPUT_CHANNELS retained for any legacy compile-time guards in shared headers
# but no longer drives the bus-layout — the encoder now allocates max sources
# at construction and the active count is a runtime parameter.
ADD_DEFINITIONS(-DINPUT_CHANNELS=${MAX_INPUT_CHANNELS})

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
