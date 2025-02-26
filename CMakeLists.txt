# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Umidificador_Inteligente C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Umidificador_Inteligente Umidificador_Inteligente.c inc/ssd1306.c)

pico_set_program_name(Umidificador_Inteligente "Umidificador_Inteligente")
pico_set_program_version(Umidificador_Inteligente "0.1")

# Generate PIO header
pico_generate_pio_header(Umidificador_Inteligente ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Umidificador_Inteligente 1)
pico_enable_stdio_usb(Umidificador_Inteligente 1)

# Add the standard library to the build
target_link_libraries(Umidificador_Inteligente
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Umidificador_Inteligente PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(Umidificador_Inteligente 
        hardware_i2c
        hardware_pio
        hardware_pwm
        hardware_adc
        )

pico_add_extra_outputs(Umidificador_Inteligente)

