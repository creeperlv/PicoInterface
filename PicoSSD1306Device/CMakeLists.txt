
add_executable(PicoSSD1306DisplayDevice ../src/devices/picoSPIComm/SPIICComm.c ../src/devices/ssd1306/ssd1306_driver.c DisplayDevice.c )

pico_set_program_name(PicoSSD1306DisplayDevice "PicoSSD1306DisplayDevice")
pico_set_program_version(PicoSSD1306DisplayDevice "0.1")


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(PicoSSD1306DisplayDevice 1)
pico_enable_stdio_usb(PicoSSD1306DisplayDevice 0)

# Add the standard library to the build
target_link_libraries(PicoSSD1306DisplayDevice
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(PicoSSD1306DisplayDevice PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(PicoSSD1306DisplayDevice
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_clocks
        hardware_adc
        pico_stdlib
        )

pico_add_extra_outputs(PicoSSD1306DisplayDevice)
