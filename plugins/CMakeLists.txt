# compile plugin dynamic libraries

include_directories(.. ../include ${GL_INC_DIR})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(
	-DGLSL_SRC_DIR=\"${GLSL_SHADER_DIR}\"
	-DFLEYE_PLUGIN_DIR=\"${PLUGINS_DIR}\"
	-DFLEYE_SCRIPT_DIR=\"${SCRIPTS_DIR}\"
	-DFLEYE_DATA_DIR=\"${DATA_DIR}\"
	)

file(GLOB PLUGIN_SOURCE_FILES "*.cc")
foreach(PLUGIN_SRC ${PLUGIN_SOURCE_FILES})
	get_filename_component(PLUGIN_SRC_NODIR ${PLUGIN_SRC} NAME)
	get_filename_component(PLUGIN_NAME ${PLUGIN_SRC_NODIR} NAME_WE)
	add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SRC})
	target_link_libraries(${PLUGIN_NAME} thirdparty)
	install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION bin)
endforeach(PLUGIN_SRC)

