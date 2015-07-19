SET ( WITH_MyMeter TRUE )
SET ( WITH_MyMeterDsp TRUE )
SET ( WITH_FFTW3 FALSE )

# binaural
ADD_DEFINITIONS (-DBINAURAL_DECODER=0)
SET ( SPECIFIC_PROJECTNAME ambix_decoder )

SET ( SPECIFIC_SOURE_DIR ${SRC_DIR}/ambix_binaural )

SET ( IGNORE_SRC_FILES MtxConv.cpp
  ConvolverData.cpp)

INCLUDE(${SRC_DIR}/CMakeLists_subprojects.txt.inc)
