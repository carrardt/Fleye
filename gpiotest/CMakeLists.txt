file(GLOB TESTGPIO_SRC "*.c")

add_definitions("-std=c11")
add_executable(test_gpio ${TESTGPIO_SRC})

target_link_libraries(test_gpio thirdparty)

install(TARGETS test_gpio RUNTIME DESTINATION bin)

