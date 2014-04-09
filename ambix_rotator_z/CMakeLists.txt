IF(WITH_OSC)
	SET( WITH_LIBLO TRUE )
ENDIF(WITH_OSC)

SET(OSX_COPY_LIB liblo.7.dylib)

INCLUDE(${SRC_DIR}/CMakeLists_subprojects.txt.inc)