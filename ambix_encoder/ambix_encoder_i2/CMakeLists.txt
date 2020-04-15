SET( WITH_EIGEN TRUE )
SET( WITH_SphericalHarmonic TRUE )
SET ( WITH_MyMeterDsp TRUE )

ADD_DEFINITIONS(-DINPUT_CHANNELS=2)
ADD_DEFINITIONS(-DJucePlugin_LV2URI="https://github.com/kronihias/ambix/ambix_encoder_i2")

# SET ( SPECIFIC_PROJECTNAME ambix_encoder )
SET ( SPECIFIC_SOURE_DIR ${SRC_DIR}/ambix_encoder )

INCLUDE(${SRC_DIR}/CMakeLists_subprojects.txt.inc)
