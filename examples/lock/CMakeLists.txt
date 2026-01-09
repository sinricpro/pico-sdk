# SinricPro Lock Example for Raspberry Pi Pico W

add_executable(sinricpro_lock_example
    main.c
)

target_link_libraries(sinricpro_lock_example
    sinricpro
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    hardware_gpio
)

# Enable USB output, disable UART
pico_enable_stdio_usb(sinricpro_lock_example 1)
pico_enable_stdio_uart(sinricpro_lock_example 0)

# Create UF2 file for drag-and-drop programming
pico_add_extra_outputs(sinricpro_lock_example)

# Set hostname for WiFi
target_compile_definitions(sinricpro_lock_example PRIVATE
    CYW43_HOST_NAME="SinricProLock"
)
