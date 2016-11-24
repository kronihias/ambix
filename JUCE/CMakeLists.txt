# this builds all juce modules except plugin_client wrapper

#IF ( APPLE )
#	ADD_DEFINITIONS(
#		#-std=c++11 -stdlib=libc++
#		-Wno-c++11-extensions
#	)
#ENDIF ( APPLE )

IF(UNIX AND NOT APPLE AND NOT ANDROID)
	set(CMAKE_CXX_FLAGS_DEBUG "-fPIC")
	set(CMAKE_CXX_FLAGS_RELEASE "-fPIC")
ENDIF(UNIX AND NOT APPLE AND NOT ANDROID)

INCLUDE_DIRECTORIES ( BEFORE build-juce )
# INCLUDE_DIRECTORIES ( BEFORE modules )

ADD_DEFINITIONS(-DJUCE_APP_CONFIG_HEADER=<AppConfig.h>)

ADD_LIBRARY ( JUCE_STATIC STATIC ${JUCE_SOURCE} )
TARGET_LINK_LIBRARIES ( JUCE_STATIC ${JUCE_LIBRARIES} )

GET_TARGET_PROPERTY(JUCE_STATIC_LIB JUCE_STATIC LOCATION)

set(JUCE_STATIC_LIB ${JUCE_STATIC_LIB} PARENT_SCOPE) # set global scope


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