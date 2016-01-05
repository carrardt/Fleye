
add_definitions("-std=c11")

file(GLOB FLEYEGPIO_SRC "gpioController.c")
add_library(fleyeGPIO ${FLEYEGPIO_SRC})

file(GLOB TESTGPIO_SRC "test_gpio.c")
add_executable(test_gpio ${TESTGPIO_SRC})
target_link_libraries(test_gpio fleyeGPIO thirdparty)
install(TARGETS test_gpio RUNTIME DESTINATION bin)

