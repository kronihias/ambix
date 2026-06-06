SET( WITH_EIGEN TRUE )
SET( WITH_SphericalHarmonic TRUE )
SET( WITH_T_DESIGN TRUE )
SET( WITH_SphFilter TRUE )

SET ( SPECIFIC_SOURE_DIR ${SRC_DIR}/ambix_vmic )

# Standalone: vmic output is a variable number of virtual mics (up to
# NUM_FILTERS_VMIC), independent of the ambisonic input order.
SET (AMBIX_STANDALONE_EXTRA_DEFS JSA_STANDALONE_MAX_OUTPUT_CHANNELS=${NUM_FILTERS_VMIC})

# VST2 + LV2 fixed-order (both share the _oX naming)
IF(BUILD_VST OR BUILD_LV2)
	SET(UNIVERSAL_BUILD FALSE)
	SET( PLUGIN_CODE AVm${AMBI_ORDER})
	INCLUDE(${SRC_DIR}/Subproject.cmake)
ENDIF()

# VST3/AU universal
IF(BUILD_VST3 OR BUILD_AU)
	SET(UNIVERSAL_BUILD TRUE)
	SET( PLUGIN_CODE AVmU)
	INCLUDE(${SRC_DIR}/Subproject.cmake)
ENDIF()
