# SinricPro SDK for Raspberry Pi Pico W
# Requires pico-sdk and PICO_SDK_PATH environment variable

cmake_minimum_required(VERSION 3.13)

# Set Pico board to Pico W (required for WiFi support)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Import Pico SDK
include(pico_sdk_import.cmake)

project(sinricpro_sdk C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize Pico SDK
pico_sdk_init()

# =============================================================================
# cJSON Library (submodule)
# =============================================================================
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON/CMakeLists.txt)
    set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(lib/cJSON)
else()
    message(STATUS "cJSON not found. Run: git submodule add https://github.com/DaveGamble/cJSON.git lib/cJSON")
endif()

# =============================================================================
# SinricPro Library
# =============================================================================
add_library(sinricpro STATIC
    # Core
    src/core/sinricpro.c
    src/core/sinricpro_debug.c
    src/core/signature.c
    src/core/message_queue.c
    src/core/event_limiter.c
    src/core/websocket_client.c
    src/core/json_helpers.c

    # Capabilities
    src/capabilities/power_state.c
    src/capabilities/brightness.c
    src/capabilities/power_level.c
    src/capabilities/color.c
    src/capabilities/color_temperature.c
    src/capabilities/motion_sensor.c
    src/capabilities/temperature_sensor.c
    src/capabilities/contact_sensor.c

    # Devices
    src/devices/sinricpro_switch.c
    src/devices/sinricpro_dimswitch.c
    src/devices/sinricpro_light.c
    src/devices/sinricpro_motion_sensor.c
    src/devices/sinricpro_temperature_sensor.c
    src/devices/sinricpro_contact_sensor.c
)

target_include_directories(sinricpro PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON
)

target_include_directories(sinricpro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sinricpro PUBLIC
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    pico_lwip_mbedtls
    pico_mbedtls
    pico_rand
    hardware_adc
)

if(TARGET cjson)
    target_link_libraries(sinricpro PUBLIC cjson)
endif()

# =============================================================================
# Examples
# =============================================================================
option(SINRICPRO_BUILD_EXAMPLES "Build SinricPro examples" ON)

if(SINRICPRO_BUILD_EXAMPLES)
    add_subdirectory(examples/switch)
    add_subdirectory(examples/dimswitch)
    add_subdirectory(examples/light)
    add_subdirectory(examples/motion_sensor)
    add_subdirectory(examples/temperature_sensor)
    add_subdirectory(examples/contact_sensor)
endif()

# =============================================================================
# Compiler Warnings
# =============================================================================
target_compile_options(sinricpro PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)
