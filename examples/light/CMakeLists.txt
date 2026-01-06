# Light Example

add_executable(sinricpro_light_example
    main.c
)

target_link_libraries(sinricpro_light_example
    sinricpro
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    hardware_gpio
    hardware_pwm
)

pico_add_extra_outputs(sinricpro_light_example)

# Enable USB serial output
pico_enable_stdio_usb(sinricpro_light_example 1)
pico_enable_stdio_uart(sinricpro_light_example 0)
