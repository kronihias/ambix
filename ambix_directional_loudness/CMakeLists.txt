SET( WITH_EIGEN TRUE )
SET( WITH_SphericalHarmonic TRUE )
SET( WITH_T_DESIGN TRUE )
SET( WITH_SphFilter TRUE )

SET ( SPECIFIC_SOURE_DIR ${SRC_DIR}/ambix_directional_loudness )

SET( PLUGIN_CODE ADl${AMBI_ORDER})

INCLUDE(${SRC_DIR}/Subproject.cmake)