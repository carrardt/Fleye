include_directories(.. ../include ../RaspiCam ${GL_INC_DIR} ${JsonCpp_INCLUDE_DIRS} ${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
link_directories(${VCLIB})

file(GLOB FLEYE_SRC "*.cc")

add_compile_options(-std=c++11 -pthread) # in cmake 2.8.12 and later
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(
	-DGLSL_SRC_DIR=\"${GLSL_SHADER_DIR}\"
	-DFLEYE_PLUGIN_DIR=\"${PLUGINS_DIR}\"
	-DFLEYE_SCRIPT_DIR=\"${SCRIPTS_DIR}\"
	-DFLEYE_DATA_DIR=\"${DATA_DIR}\"
	)

add_executable(fleye ${FLEYE_SRC})
target_link_libraries(fleye fleyeGPIO RaspiCam jsoncpp thirdparty ${MMAL_LIBS} vcos bcm_host GLESv2 EGL pthread m dl)
install(TARGETS fleye RUNTIME DESTINATION bin)

