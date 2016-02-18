# copy shader files
file(GLOB_RECURSE GLSL_SOURCE_FILES "*.glsl")
install(FILES ${GLSL_SOURCE_FILES} DESTINATION share/fleye/shaders)
