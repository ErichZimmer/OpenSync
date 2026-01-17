#include <stdbool.h>
#include <stdint.h>

#include "status/sequencer_status.h"
#include "scpi_common.h"


bool is_running()
{
    uint32_t status_copy = sequencer_status_get();

    return !((status_copy == IDLE) || (status_copy == ABORTED));
}