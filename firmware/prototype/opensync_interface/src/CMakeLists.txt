set(SCPI_LIB_DIR 
    ${PROJECT_SOURCE_DIR}/external/scpi-parser
)

set(PRAWN_DO_DIR
    ${PROJECT_SOURCE_DIR}/external/prawn_do
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/usb_desc)

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

set(OPENSYNC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/usb_desc/usb_descriptors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/version/opensync_version.c
    ${CMAKE_CURRENT_SOURCE_DIR}/serial_interface/scpi-def.c
)

# Add executable
add_executable(opensync_interface 
    main.c
    ${OPENSYNC_SOURCE}
    ${PRAWN_DO_DIR}/fast_serial.c
    ${SCPI_SOURCE}
)

pico_set_program_name(opensync_interface "opensync_interface")
pico_set_program_version(opensync_interface "0.1")

# Enable USB COMS
pico_enable_stdio_uart(opensync_interface 0)
pico_enable_stdio_usb(opensync_interface 1)

# Add the source files
target_include_directories(opensync_interface PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PRAWN_DO_DIR}
    ${SCPI_LIB_DIR}/libscpi/inc
)

# Add any user requested libraries including the standard library
target_link_libraries(opensync_interface
    pico_stdlib
    pico_multicore
    pico_unique_id
    tinyusb_device
    tinyusb_board
)

# Generate UF2 file for flashing
pico_add_extra_outputs(opensync_interface)