GET_FILENAME_COMPONENT(SUBDIRNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

ADD_DEFINITIONS(-DAMBI_ORDER=${AMBI_ORDER})

IF( DEFINED SPECIFIC_PROJECTNAME )
	# this is for ambix_decoder (which shares the code of ambix_binaural)
	SET (SUBPROJECT_NAME ${SPECIFIC_PROJECTNAME}_o${AMBI_ORDER})
ELSE( DEFINED SPECIFIC_PROJECTNAME )
	# this is the normal way...
	SET (SUBPROJECT_NAME ${SUBDIRNAME}_o${AMBI_ORDER})
ENDIF(DEFINED SPECIFIC_PROJECTNAME )

# add the folder with Juce includes
INCLUDE_DIRECTORIES ( JuceLibraryCode )

ADD_DEFINITIONS(-DJUCE_APP_CONFIG_HEADER=<AppConfig.h>)

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

############################
# AUDIO UNIT SOURCE
IF (BUILD_AU)
	LIST ( APPEND SOURCE ${AU_PLUGIN_SOURCE})
ENDIF (BUILD_AU)

#SORT IT
LIST ( SORT SOURCE )

ADD_LIBRARY(${SUBPROJECT_NAME} MODULE ${JUCE_PLUGIN_CLIENT_SOURCE} ${SOURCE} ${HEADER})

ADD_DEPENDENCIES(${SUBPROJECT_NAME} JUCE_STATIC)

####################
# additional libs (FFTW3F, Eigen3 is header only)

IF(WITH_FFTW3)

	# MESSAGE( STATUS "LINKING FFTW3F: " ${FFTW3F_LIBRARY} )
	
	TARGET_LINK_LIBRARIES( ${SUBPROJECT_NAME}
	
		${FFTW3F_LIBRARY}
		${FFTW3F_THREADS_LIBRARY}
	)
ENDIF(WITH_FFTW3)


IF(WITH_LIBSOXR)
	TARGET_LINK_LIBRARIES( ${SUBPROJECT_NAME}
		${LIBSOXR_LIBRARIES}
	)
ENDIF(WITH_LIBSOXR)


IF(WITH_ZITA_CONVOLVER)
	TARGET_LINK_LIBRARIES( ${SUBPROJECT_NAME}
		${LIBZITACONVOLVER_LIBRARIES}
	)
ENDIF(WITH_ZITA_CONVOLVER)

####################
#plattform specifics

IF(NOT APPLE)

	#osx need special treating for creating the bundle...
	
	TARGET_LINK_LIBRARIES( ${SUBPROJECT_NAME}
		${JUCE_STATIC_LIB}
		${JUCE_LIBRARIES}
	)
	
	# get the target folder for later copying it
	GET_TARGET_PROPERTY(FILEPATH ${SUBPROJECT_NAME} LOCATION)
	
ENDIF(NOT APPLE)





IF(APPLE)

	#prototype Info.plist
	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${SRC_DIR}/osx_ressources/MacOSXBundleInfo.plist.in)

	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES BUNDLE TRUE)
	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES BUNDLE_EXTENSION vst)
	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})
	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING ${VERSION})
	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME ${SUBPROJECT_NAME})


	SET_TARGET_PROPERTIES(${SUBPROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR})

	# link libs
	TARGET_LINK_LIBRARIES( ${SUBPROJECT_NAME}
		${JUCE_STATIC_LIB}
		${JUCE_LIBRARIES}
	)
	
	# strip it
	ADD_CUSTOM_COMMAND(
		TARGET ${SUBPROJECT_NAME} POST_BUILD 
		COMMAND ${CMAKE_STRIP} -x -S ${BIN_DIR}/${SUBPROJECT_NAME}.vst/Contents/MacOS/${SUBPROJECT_NAME}
	)
	
	
	#copy whole .vst folder to _bin/vst
	ADD_CUSTOM_COMMAND(
		TARGET ${SUBPROJECT_NAME} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} 
		ARGS -E copy_directory ${BIN_DIR}/${SUBPROJECT_NAME}.vst ${BIN_DIR}/_bin/vst/${SUBPROJECT_NAME}.vst
	)

	#copy PkgInfo
	ADD_CUSTOM_COMMAND(
		TARGET ${SUBPROJECT_NAME} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} 
		ARGS -E copy ${SRC_DIR}/osx_ressources/PkgInfo ${BIN_DIR}/_bin/vst/${SUBPROJECT_NAME}.vst/Contents/PkgInfo
	)
	
	IF( DEFINED OSX_COPY_LIB )
		#copy additional files (eg. dynamic libraries)
		ADD_CUSTOM_COMMAND(
			TARGET ${SUBPROJECT_NAME} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} 
			ARGS -E copy ${SRC_DIR}/mac-libs/${OSX_COPY_LIB} ${BIN_DIR}/_bin/vst/${SUBPROJECT_NAME}.vst/Contents/Frameworks/${OSX_COPY_LIB}
			)
	ENDIF( DEFINED OSX_COPY_LIB )
	
	IF(BUILD_AU)
		#copy whole .vst folder to _bin/au
		ADD_CUSTOM_COMMAND(
			TARGET ${SUBPROJECT_NAME} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} 
			ARGS -E copy_directory ${BIN_DIR}/${SUBPROJECT_NAME}.vst ${BIN_DIR}/_bin/au/${SUBPROJECT_NAME}.component
		)
		#copy PkgInfo
		ADD_CUSTOM_COMMAND(
			TARGET ${SUBPROJECT_NAME} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} 
			ARGS -E copy ${SRC_DIR}/osx_ressources/PkgInfo ${BIN_DIR}/_bin/au/${SUBPROJECT_NAME}.component/Contents/PkgInfo
		)
	ENDIF(BUILD_AU)

	#remove build in home dir
	ADD_CUSTOM_COMMAND(
		TARGET ${SUBPROJECT_NAME} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} 
		ARGS -E remove_directory ${BIN_DIR}/${SUBPROJECT_NAME}.vst
	)

ENDIF(APPLE)


IF(UNIX AND NOT APPLE AND NOT ANDROID)
	IF(BUILD_LV2)
		ADD_CUSTOM_COMMAND(
			TARGET ${SUBPROJECT_NAME} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} 
			ARGS -E copy ${FILEPATH} ${BIN_DIR}/_bin/lv2/${SUBPROJECT_NAME}.lv2/${SUBPROJECT_NAME}.so
		)
	
	ENDIF(BUILD_LV2)
	
	IF(BUILD_VST)
		ADD_CUSTOM_COMMAND(
			TARGET ${SUBPROJECT_NAME} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} 
			ARGS -E copy ${FILEPATH} ${BIN_DIR}/_bin/vst/${SUBPROJECT_NAME}.so
		)
	ENDIF(BUILD_VST)
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)


IF(WIN32)

	ADD_CUSTOM_COMMAND(
		TARGET ${SUBPROJECT_NAME} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} 
		ARGS -E copy ${FILEPATH} ${BIN_DIR}/_bin/vst/${SUBPROJECT_NAME}.dll
	)
ENDIF(WIN32)


################################################
# change to static linking for ms visual c++
IF(WIN32)

set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

ENDIF(WIN32)
