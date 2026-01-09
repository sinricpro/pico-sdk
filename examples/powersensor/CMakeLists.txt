# Power Sensor Example

add_executable(sinricpro_powersensor_example
    main.c
)

target_link_libraries(sinricpro_powersensor_example
    sinricpro
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    hardware_gpio
    hardware_adc
)

pico_add_extra_outputs(sinricpro_powersensor_example)

# Enable USB serial output
pico_enable_stdio_usb(sinricpro_powersensor_example 1)
pico_enable_stdio_uart(sinricpro_powersensor_example 0)
