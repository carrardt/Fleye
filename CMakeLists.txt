cmake_minimum_required (VERSION 2.8)
project (Fleye)

# various usage samples
# 
# ./Fleye/fleye -v -ex fixedfps -res 1280x960 -geom 640x480+32+32 -script wincoord
# optimal resolution/performance
# ./Fleye/fleye -ex fixedfps -res 1280x960 -geom 1024x768 -v
# ./Fleye/fleye -ex fixedfps -res 1280x960 -geom 640x480 -script camera-rgba
# sudo ./Fleye/fleye -awb auto -res 512x512 -geom 512x512 -script panTiltCalibration
# sudo fleye -ex fixedfps -res 960x720 -geom 960x720 -script objectfollower

# predefine location of jsoncpp include and libs.
# needs to install libjsoncpp-dev package
set( JsonCpp_FOUND 1 )
set( JsonCpp_INCLUDE_DIRS "/usr/include/jsoncpp" )
set( JsonCpp_LIBRARIES "/usr/lib/libjsoncpp.so" )

# Raspberry Pi VideoCore HW Abstraction Layer
SET(VCROOT "/opt/vc")
SET(VCINC "${VCROOT}/include")
SET(VCLIB "${VCROOT}/lib")
SET(MMAL_LIBS mmal_core mmal_util mmal_vc_client)

# OpenGL ES headers from userland code
SET(GL_INC_DIR ${VCINC} ${VCINC}/interface/vcos/pthreads)
SET(GL_LIB_DIR "${VCLIB}")

# Resource directories
SET(GLSL_SHADER_DIR ${CMAKE_INSTALL_PREFIX}/share/fleye/shaders)
SET(SCRIPTS_DIR ${CMAKE_INSTALL_PREFIX}/share/fleye/scripts)
SET(PLUGINS_DIR ${CMAKE_INSTALL_PREFIX}/lib/fleye/plugins)
SET(DATA_DIR ${CMAKE_INSTALL_PREFIX}/share/fleye/data)

add_subdirectory(thirdparty)
add_subdirectory(plugins)
add_subdirectory(scripts)
add_subdirectory(shaders)
add_subdirectory(RaspiCam)
add_subdirectory(TestCamera)
add_subdirectory(gpio)
add_subdirectory(data)

add_subdirectory(Fleye)

