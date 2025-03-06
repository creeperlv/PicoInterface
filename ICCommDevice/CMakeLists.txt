
add_executable(ICCommDevice ../src/devices/picoSPIComm/SPIICComm.c  ICCommMasterDevice.c )

pico_set_program_name(ICCommDevice "ICCommDevice")
pico_set_program_version(ICCommDevice "0.1")


# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(ICCommDevice 1)
pico_enable_stdio_usb(ICCommDevice 0)

# Add the standard library to the build
target_link_libraries(ICCommDevice
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(ICCommDevice PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(ICCommDevice
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

pico_add_extra_outputs(ICCommDevice)
