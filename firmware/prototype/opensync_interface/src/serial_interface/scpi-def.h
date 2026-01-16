#pragma once

#include "scpi/scpi.h"

#include "version/opensync_version.h"

#define SCPI_INPUT_BUFFER_LENGTH 256
#define SCPI_ERROR_QUEUE_SIZE 17

#define SCPI_IDN1 "OpenPIV"
#define SCPI_IDN2 "OpenSync"
#define SCPI_IDN3 NULL
#define SCPI_IDN4 OPENSYNC_VERSION

extern scpi_interface_t scpi_interface;
// extern char scpi_input_buffer[];
extern scpi_error_t scpi_error_queue_data[];
extern scpi_t scpi_context;

void scpi_instrument_init();