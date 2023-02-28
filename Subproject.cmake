GET_FILENAME_COMPONENT(SUBDIRNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

IF( DEFINED SPECIFIC_PROJECTNAME )
	# this is for ambix_decoder (which shares the code of ambix_binaural)
	SET (SUBPROJECT_NAME ${SPECIFIC_PROJECTNAME}_o${AMBI_ORDER})
ELSE( DEFINED SPECIFIC_PROJECTNAME )
	# this is the normal way...
	SET (SUBPROJECT_NAME ${SUBDIRNAME}_o${AMBI_ORDER})
ENDIF(DEFINED SPECIFIC_PROJECTNAME )

# add the folder with Juce includes
INCLUDE_DIRECTORIES ( JuceLibraryCode )

IF(DEFINED SPECIFIC_SOURE_DIR)
	INCLUDE_DIRECTORIES ( ${SPECIFIC_SOURE_DIR}/JuceLibraryCode )
ENDIF(DEFINED SPECIFIC_SOURE_DIR)

#############################
# add all c, cpp, cc files from the Source directory
FILE ( GLOB_RECURSE SOURCE Source/*.c* )
FILE ( GLOB_RECURSE HEADER Source/*.h* )

IF(DEFINED SPECIFIC_SOURE_DIR)
	FILE ( GLOB_RECURSE SOURCE ${SPECIFIC_SOURE_DIR}/Source/*.c* )
	FILE ( GLOB_RECURSE HEADER ${SPECIFIC_SOURE_DIR}/Source/*.h* )
ENDIF(DEFINED SPECIFIC_SOURE_DIR)

############################
# ignore some source files (specially made for ambix_decoder)
# little bit strange construction but it works...

IF(DEFINED IGNORE_SRC_FILES)

	FOREACH ( CUR_SRC ${SOURCE} )

		GET_FILENAME_COMPONENT(CUR_SRC_NAME ${CUR_SRC} NAME)

		LIST ( FIND IGNORE_SRC_FILES ${CUR_SRC_NAME} FOUND )
		IF(NOT ${FOUND} EQUAL -1)
			LIST ( FIND SOURCE ${CUR_SRC} REMOVE_ID )
			LIST ( REMOVE_AT SOURCE ${REMOVE_ID} )
			# MESSAGE ( STATUS "REMOVED SRC FILE: " ${CUR_SRC_NAME})
		ENDIF(NOT ${FOUND} EQUAL -1)

	ENDFOREACH ( CUR_SRC ${SOURCE} )

ENDIF(DEFINED IGNORE_SRC_FILES)


############################
# add from common directory
IF(WITH_SphericalHarmonic)
	LIST ( APPEND SOURCE ${SRC_DIR}/common/SphericalHarmonic/ShChebyshev.cpp
		${SRC_DIR}/common/SphericalHarmonic/ShLegendre.cpp
		${SRC_DIR}/common/SphericalHarmonic/ShNorm.cpp
		${SRC_DIR}/common/SphericalHarmonic/SphericalHarmonic.cpp)

	LIST ( APPEND HEADER ${SRC_DIR}/common/SphericalHarmonic/ShChebyshev.h
		${SRC_DIR}/common/SphericalHarmonic/ShLegendre.h
		${SRC_DIR}/common/SphericalHarmonic/ShNorm.h
		${SRC_DIR}/common/SphericalHarmonic/SphericalHarmonic.h
		${SRC_DIR}/common/SphericalHarmonic/ch_sequence.h
		${SRC_DIR}/common/SphericalHarmonic/ch_cs.h
		${SRC_DIR}/common/SphericalHarmonic/normalization.h
		${SRC_DIR}/common/SphericalHarmonic/tools.h)
ENDIF(WITH_SphericalHarmonic)

IF(WITH_SphFilter)
	LIST ( APPEND SOURCE ${SRC_DIR}/common/SphFilter/SphFilter.cpp)
	LIST ( APPEND HEADER ${SRC_DIR}/common/SphFilter/SphFilter.h)
ENDIF(WITH_SphFilter)

IF(WITH_MyMeter)
	LIST ( APPEND SOURCE ${SRC_DIR}/common/MyMeter/MyMeter.cpp)
	LIST ( APPEND HEADER ${SRC_DIR}/common/MyMeter/MyMeter.h)
ENDIF(WITH_MyMeter)

IF(WITH_MyMeterDsp)
	LIST ( APPEND SOURCE ${SRC_DIR}/common/MyMeterDsp/MyMeterDsp.cpp)
	LIST ( APPEND HEADER ${SRC_DIR}/common/MyMeterDsp/MyMeterDsp.h)
ENDIF(WITH_MyMeterDsp)

IF(WITH_T_DESIGN)
	LIST ( APPEND SOURCE ${SRC_DIR}/common/Ressources/t_design.cpp)
	LIST ( APPEND HEADER ${SRC_DIR}/common/Ressources/t_design.h)
ENDIF(WITH_T_DESIGN)

IF(WITH_LegendreU)
	LIST ( APPEND SOURCE ${SRC_DIR}/common/LegendreU/LegendreU.cpp)
	LIST ( APPEND HEADER ${SRC_DIR}/common/LegendreU/LegendreU.h)
ENDIF(WITH_LegendreU)


#SORT IT
LIST ( SORT SOURCE )

IF(NOT APPLE)
	SET ( WITH_FFTW3 TRUE )
ENDIF(NOT APPLE)


juce_add_plugin (${SUBPROJECT_NAME}
    PLUGIN_MANUFACTURER_CODE Kron
    PLUGIN_CODE ABi5
    COMPANY_NAME "kronlachner"
    PRODUCT_NAME ${SUBPROJECT_NAME}
    FORMATS ${AMBIX_FORMATS}
    VERSION ${VERSION}
    LV2URI http://www.matthiaskronlachner.com/${SUBPROJECT_NAME})

juce_generate_juce_header(${SUBPROJECT_NAME})

target_sources (${SUBPROJECT_NAME} PRIVATE
    ${SOURCE}
    ${HEADER}
    )

target_compile_definitions (${SUBPROJECT_NAME} PRIVATE
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=0
    JUCE_USE_MP3AUDIOFORMAT=0
    JUCE_USE_LAME_AUDIO_FORMAT=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries (${SUBPROJECT_NAME} PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_osc
    juce::juce_dsp
    juce::juce_opengl
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    #juce::juce_recommended_warning_flags
	)