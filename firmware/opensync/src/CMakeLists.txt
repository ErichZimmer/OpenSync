# Get relative paths to third party libraries
set(SCPI_LIB_DIR 
    ${PROJECT_SOURCE_DIR}/external/scpi-parser
)

set(PRAWN_DO_DIR
    ${PROJECT_SOURCE_DIR}/external/prawn_do
)

# Incl. necessary directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/usb_desc)

# scpi-parser does not have a cmake file, so add source files as needed
set(SCPI_SOURCE
    ${SCPI_LIB_DIR}/libscpi/src/parser.c
    ${SCPI_LIB_DIR}/libscpi/src/lexer.c
    ${SCPI_LIB_DIR}/libscpi/src/error.c
    ${SCPI_LIB_DIR}/libscpi/src/ieee488.c
    ${SCPI_LIB_DIR}/libscpi/src/minimal.c
    ${SCPI_LIB_DIR}/libscpi/src/utils.c
    ${SCPI_LIB_DIR}/libscpi/src/units.c
    ${SCPI_LIB_DIR}/libscpi/src/fifo.c
)

# OpenSync source files
set(OPENSYNC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/overclock/overclock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sequencer/sequencer_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sequencer/sequencer_clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sequencer/sequencer_output.c
    ${CMAKE_CURRENT_SOURCE_DIR}/status/sequencer_status.c
    ${CMAKE_CURRENT_SOURCE_DIR}/status/debug_status.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/serial_int_output.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/scpi-def.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/scpi_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/scpi_system.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/scpi_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/scpi_instrument.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/scpi_clock_sequencer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial/scpi_pulse_sequencer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/system/core_1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/system/core_2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/usb_desc/usb_descriptors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/version/opensync_version_info.c
)

# Add executable
add_executable(opensync 
    main.c
    ${OPENSYNC_SOURCE}
    ${PRAWN_DO_DIR}/fast_serial.c
    ${SCPI_SOURCE}
)

pico_set_program_name(opensync "OpenSync")
pico_set_program_version(opensync "0.1") # this number is irrespective of firmware!?

# Generate PIO headers
pico_generate_pio_header(opensync ${CMAKE_CURRENT_SOURCE_DIR}/pio_assembly/sequencer_pio_clock_freerun.pio)
pico_generate_pio_header(opensync ${CMAKE_CURRENT_SOURCE_DIR}/pio_assembly/sequencer_pio_clock_triggered.pio)
pico_generate_pio_header(opensync ${CMAKE_CURRENT_SOURCE_DIR}/pio_assembly/sequencer_pio_pulse_sequencer.pio)

# Enable native USB OTG
pico_enable_stdio_uart(opensync 0)
pico_enable_stdio_usb(opensync 1)

# Add the source files
target_include_directories(opensync PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PRAWN_DO_DIR}
    ${SCPI_LIB_DIR}/libscpi/inc
)

target_compile_definitions(opensync PUBLIC
    USE_FULL_ERROR_LIST
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