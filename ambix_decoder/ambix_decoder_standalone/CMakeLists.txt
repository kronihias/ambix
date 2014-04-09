SET ( WITH_MyMeter TRUE )
SET ( WITH_MyMeterDsp TRUE )
SET ( WITH_FFTW3 FALSE )

# binaural
ADD_DEFINITIONS (-DBINAURAL_DECODER=0)
SET ( SPECIFIC_PROJECTNAME ambix_decoder_standalone )

SET ( SPECIFIC_SOURE_DIR ${SRC_DIR}/ambix_binaural )

SET ( IGNORE_SRC_FILES SpkConv.cpp 
	vresampler.cc 
	resampler.cc 
	resampler-table.cc 
	zita-convolver.cc 
	AudioFFT.cpp 
	Convolver.cpp 
	FFTConvolver.cpp 
	TwoStageFFTConvolver.cpp 
	Utilities.cpp )
	
INCLUDE(${SRC_DIR}/CMakeLists_subprojects_standalone.txt.inc)
