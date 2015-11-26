ADD_SUBDIRECTORY ( ambix_directional_loudness )

IF (BUILD_STANDALONE)
	ADD_SUBDIRECTORY ( ambix_directional_loudness_standalone )
ENDIF (BUILD_STANDALONE)