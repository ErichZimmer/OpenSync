# Get prawn_digital_output usb library
set(PRAWN_DO_DIR ${PROJECT_SOURCE_DIR}/external/prawn_do)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(OPENSYNC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/overclock/overclock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sequencer/sequencer_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sequencer/sequencer_clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sequencer/sequencer_output.c
    ${CMAKE_CURRENT_SOURCE_DIR}/status/sequencer_status.c
    ${CMAKE_CURRENT_SOURCE_DIR}/status/debug_status.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/serial_int_output.c
    ${CMAKE_CURRENT_SOURCE_DIR}/system/core_1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/system/core_2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/version/opensync_version.c
)

# Add executable
add_executable(opensync 
    main.c 
    ${OPENSYNC_SOURCE}
    ${PRAWN_DO_DIR}/fast_serial.c
) 

pico_set_program_name(opensync "OpenSync")
pico_set_program_version(opensync "0.1")

# Generate PIO header
 pico_generate_pio_header(opensync ${CMAKE_CURRENT_SOURCE_DIR}/pio_assembly/sequencer_pio_clock_freerun.pio)
 pico_generate_pio_header(opensync ${CMAKE_CURRENT_SOURCE_DIR}/pio_assembly/sequencer_pio_clock_triggered.pio)
 pico_generate_pio_header(opensync ${CMAKE_CURRENT_SOURCE_DIR}/pio_assembly/sequencer_pio_pulse_sequencer.pio)

# Enable USB COMS
pico_enable_stdio_uart(opensync 0)
pico_enable_stdio_usb(opensync 1)

# Add the source files
target_include_directories(opensync PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PRAWN_DO_DIR}
)

# Add any user requested libraries including the standard library
target_link_libraries(opensync
    pico_stdlib
    pico_multicore
    pico_unique_id
    hardware_dma
    hardware_pio
    tinyusb_device
    tinyusb_board
)

# Generate UF2 file for flashing
pico_add_extra_outputs(opensync)