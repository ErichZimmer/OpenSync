#include <stdint.h>

#include "scpi_common.h"


// The compiler is unhappy about these  definitions, so redifine it here.
// https://helpfiles.keysight.com/csg/e5063a/product_information/error_messages/error_messages.htm

const int16_t SCPI_ERROR_DATA_OUT_OF_RANGE = -222;
const int16_t SCPI_ERROR_TOO_MUCH_DATA = -223;
