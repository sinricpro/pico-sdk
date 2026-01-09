# Air Quality Sensor Example

add_executable(sinricpro_airqualitysensor_example
    main.c
)

target_link_libraries(sinricpro_airqualitysensor_example
    sinricpro
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    hardware_gpio
    hardware_i2c
)

pico_add_extra_outputs(sinricpro_airqualitysensor_example)

# Enable USB serial output
pico_enable_stdio_usb(sinricpro_airqualitysensor_example 1)
pico_enable_stdio_uart(sinricpro_airqualitysensor_example 0)
