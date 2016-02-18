# copy scripts files
file(GLOB_RECURSE SCRIPTS_FILES "*.json")
install(FILES ${SCRIPTS_FILES} DESTINATION share/fleye/scripts)
