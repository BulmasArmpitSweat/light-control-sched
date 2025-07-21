# Generated Cmake Pico project file
# Edited by yours truly

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
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

set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS" CACHE STRING "FreeRTOS Kernel Path")
set(FREERTOS_RP2040_PORT_PATH "${PROJECT_SOURCE_DIR}/FreeRTOS/portable/ThirdParty/GCC/RP2040" CACHE STRING "FreeRTOS RP2040 Port Path")

set(FREERTOS_KERNEL_HEAP "4" CACHE STRING "Free RTOS heap implementation")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Pull in FreeRTOS Kernel shit
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(light_control_sched C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(light_control_sched light_control_sched.cpp )

pico_set_program_name(light_control_sched "light_control_sched")
pico_set_program_version(light_control_sched "0.1")

# Generate PIO header
pico_generate_pio_header(light_control_sched ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(light_control_sched 0)
pico_enable_stdio_usb(light_control_sched 1)

# Add the standard library to the build
target_link_libraries(light_control_sched
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(light_control_sched PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${FREERTOS_KERNEL_PATH}/include
        ${FREERTOS_RP2040_PORT_PATH}
        ${FREERTOS_RP2040_PORT_PATH}/include
        ${USERHOME}/.pico-sdk/sdk/2.1.1
)

# Add any user requested libraries
target_link_libraries(light_control_sched
        hardware_timer
        hardware_watchdog
        hardware_clocks
        hardware_rtc
        pico_multicore
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_freertos
        pico_lwip_sntp
        pico_pwm
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        )

pico_add_extra_outputs(light_control_sched)