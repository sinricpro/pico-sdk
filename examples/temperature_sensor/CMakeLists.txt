# Temperature Sensor Example

add_executable(sinricpro_temperature_sensor_example
    main.c
)

target_link_libraries(sinricpro_temperature_sensor_example
    sinricpro
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    hardware_adc
)

pico_add_extra_outputs(sinricpro_temperature_sensor_example)

# Enable USB serial output
pico_enable_stdio_usb(sinricpro_temperature_sensor_example 1)
pico_enable_stdio_uart(sinricpro_temperature_sensor_example 0)
