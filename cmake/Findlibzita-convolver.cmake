# - Try to find libzita-convolver
# Once done this will define
#
# LIBZITACONVOLVER_FOUND - system has LIBZITACONVOLVER
# LIBZITACONVOLVER_INCLUDE_DIRS - the LIBZITACONVOLVER include directory
# LIBZITACONVOLVER_LIBRARIES - Link these to use LIBZITACONVOLVER
# LIBZITACONVOLVER_DEFINITIONS - Compiler switches required for using LIBZITACONVOLVER
#
# Adapted from cmake-modules Google Code project
#
# Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# (Changes for LIBZITACONVOLVER) Copyright (c) 2014 Matthias Kronlachner <m.kronlachner@gmail.com>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (LIBZITACONVOLVER_LIBRARIES AND LIBZITACONVOLVER_INCLUDE_DIRS)
  # in cache already
  set(LIBZITACONVOLVER_FOUND TRUE)
else (LIBZITACONVOLVER_LIBRARIES AND LIBZITACONVOLVER_INCLUDE_DIRS)
  find_path(LIBZITACONVOLVER_INCLUDE_DIR
    NAMES
      zita-convolver.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(LIBZITACONVOLVER_LIBRARY
    NAMES
      zita-convolver
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(LIBZITACONVOLVER_INCLUDE_DIRS
    ${LIBZITACONVOLVER_INCLUDE_DIR}
  )
  set(LIBZITACONVOLVER_LIBRARIES
    ${LIBZITACONVOLVER_LIBRARY}
)

  if (LIBZITACONVOLVER_INCLUDE_DIRS AND LIBZITACONVOLVER_LIBRARIES)
     set(LIBZITACONVOLVER_FOUND TRUE)
  endif (LIBZITACONVOLVER_INCLUDE_DIRS AND LIBZITACONVOLVER_LIBRARIES)

  if (LIBZITACONVOLVER_FOUND)
    if (NOT LIBZITACONVOLVER_FIND_QUIETLY)
      message(STATUS "Found LIBZITACONVOLVER: ${LIBZITACONVOLVER_LIBRARIES}")
    endif (NOT LIBZITACONVOLVER_FIND_QUIETLY)
  else (LIBZITACONVOLVER_FOUND)
    if (LIBZITACONVOLVER_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find LIBZITACONVOLVER")
    endif (LIBZITACONVOLVER_FIND_REQUIRED)
  endif (LIBZITACONVOLVER_FOUND)

  # show the LIBZITACONVOLVER_INCLUDE_DIRS and LIBZITACONVOLVER_LIBRARIES variables only in the advanced view
  mark_as_advanced(LIBZITACONVOLVER_INCLUDE_DIRS LIBZITACONVOLVER_LIBRARIES)

endif (LIBZITACONVOLVER_LIBRARIES AND LIBZITACONVOLVER_INCLUDE_DIRS)
