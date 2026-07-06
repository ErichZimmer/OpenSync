#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "scpi/scpi.h"

#include "system/core_1.h"
#include "structs/clock_config.h"
#include "sequencer/sequencer_common.h"
#include "scpi_common.h"


enum {
    MICROHERTZ = 0,
    MILLIHERTZ,
    HERTZ,
    KILOHERTZ,
    MEGAHERTZ
};

static double clock_sequence_buffer_freq[CLOCK_INSTRUCTIONS_MAX / 2] = {0};
static uint32_t clock_sequence_buffer_reps[CLOCK_INSTRUCTIONS_MAX / 2] = {0.0};
static size_t  clock_sequence_buffer_freqs_read = 0;
static size_t  clock_sequence_buffer_reps_read = 0;
const uint32_t clock_sequence_buffer_size = CLOCK_INSTRUCTIONS_MAX / 2;

// This is used in the INSTructions submodule for stateful operation.
static uint32_t clock_id_stateful = 0;


// Function to clear the static buffer cache for freq data
void clock_sequencer_cache_freq_clear()
{
    for (uint32_t i = 0; i < clock_sequence_buffer_size; i++)
    {
        clock_sequence_buffer_freq[i] = 0;
    }

    clock_sequence_buffer_freqs_read = 0;
}


// Function to clear the static buffer cache for reps data
void clock_sequencer_cache_reps_clear()
{
    for (uint32_t i = 0; i < clock_sequence_buffer_size; i++)
    {
        clock_sequence_buffer_reps[i] = 0.0;
    }

    clock_sequence_buffer_reps_read = 0;
}


// Clear all the clock instructions cache
void clock_sequencer_cache_clear()
{
    clock_sequencer_cache_freq_clear();
    clock_sequencer_cache_reps_clear();
}


// Get clock id from context and validate it. 
// If valid, change clock_id to that value
bool SCPI_check_clock_id_and_append_error(
    scpi_t* context,
    uint32_t* clock_id
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id_res = 0;

    // If the system status is note (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        STATEFUL // -1 means to use stateful sequencer ID
    );

    // Check if user specified sequencer ID
    if (numbers[0] == STATEFUL)
    {
        clock_id_res = clock_id_stateful;
    }
    else
    {
        // Cast numbers to usable int type
        clock_id_res = (uint32_t) numbers[0];
    }

    // Validate the ID
    if (!clock_id_validate(clock_id_res))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return true;
    }

    *clock_id = clock_id_res;

    return false;
}


// Set the stateful ID of a clock sequencer
scpi_result_t SCPI_ClockIndex(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;

    // If the system status is note (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }


    // Get clock sequencer ID 
    // We do not use SCPI_check_clock_id_and_append_error since it checks for statefullness
    if (!SCPI_ParamUInt32(context, &clock_id, TRUE))
    {
        return SCPI_RES_ERR;
    }

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    // Now set stateful clock index
    clock_id_stateful = clock_id;

    return SCPI_RES_OK;
}


// Query clock sequencer stateful index
scpi_result_t SCPI_ClockIndexQ(
    scpi_t* context
) {
    // Return as uint32
    SCPI_ResultUInt32(
        context,
        clock_id_stateful
    );
    
    return SCPI_RES_OK;
}


// Set the status of a clock sequencer at ID N
scpi_result_t SCPI_ClockStatus(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;

    // If the system status is note (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid clock state choices
    const scpi_choice_def_t options[] = {
        {"ON",  STATUS_ON},
        {"OFF", STATUS_OFF},
        SCPI_CHOICE_LIST_END
    };

    // Now get the clock state if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    // Now enable that clock channel
    bool success = clock_sequencer_state_set(
        clock_id,
        (bool) choice
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query a clock sequencer at ID N to see if it is active
scpi_result_t SCPI_ClockStatusQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the state of the clock sequencer at clock_id
    bool clock_id_state = config_array[clock_id].active;

    // Return as bool
    SCPI_ResultBool(
        context,
        clock_id_state
    );

    return SCPI_RES_OK;
}


// Set clock divider at clock sequencer N
scpi_result_t SCPI_ClockClockDivider(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint32_t clock_divider = 1;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid clock cycle resolution choices
    const scpi_choice_def_t options[] = {
        {"HIGH",                CLOCK_DIV_RES_HIGH},
        {"HIGH_RES",            CLOCK_DIV_RES_HIGH},
        {"MED",                 CLOCK_DIV_RES_MED},
        {"MED_RES",             CLOCK_DIV_RES_MED},
        {"LOW",                 CLOCK_DIV_RES_LOW},
        {"LOW_RES",             CLOCK_DIV_RES_LOW},
        {"VERY_LOW",            CLOCK_DIV_RES_VERY_LOW},
        {"VERY_LOW_RES",        CLOCK_DIV_RES_VERY_LOW},
        {"VERY_VERY_LOW",       CLOCK_DIV_RES_VERY_VERY_LOW},
        {"VERY_VERY_LOW_RES",   CLOCK_DIV_RES_VERY_VERY_LOW},
        SCPI_CHOICE_LIST_END
    };

    // Now get the clock divider if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    // Cast choice into usable clock divider type
    clock_divider = (uint32_t) choice;

    bool success = clock_divider_set(
        clock_id,
        clock_divider
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query clock divider at clock sequencer N
scpi_result_t SCPI_ClockClockDividerQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the clock divider of the clock sequencer at clock_id
    uint32_t clock_divider = config_array[clock_id].clock_divider;

    // Return as uint32
    SCPI_ResultUInt32(
        context,
        clock_divider
    );
    
    return SCPI_RES_OK;
}


// Set clock units at clock sequencer N
scpi_result_t SCPI_ClockeUnits(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint64_t unit_offset = 0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid frequency resolution choices
    const scpi_choice_def_t options[] = {
        {"HZ",  HERTZ},
        {"KHZ", KILOHERTZ},
        {"MHZ", MEGAHERTZ},
        SCPI_CHOICE_LIST_END
    };

    // Now get the clock divider if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    switch (choice)
    {
        case HERTZ:
            unit_offset = 1.0;
            break;
        case KILOHERTZ:
            unit_offset = 1e3;
            break;
        case MEGAHERTZ:
            unit_offset = 1e6;
            break;
        default: // Default to Hertz
            unit_offset = 1.0;
    }

    // Cast choice into usable clock divider type
    bool success = clock_unit_offset_set(
        clock_id,
        unit_offset
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query clock data units offset at clock sequencer N
scpi_result_t SCPI_ClockUnitsQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the unit offset of the clock sequencer at clock_id
    double unit_offset = config_array[clock_id].unit_offset;

    // Return as double
    SCPI_ResultDouble(
        context,
        unit_offset
    );
    
    return SCPI_RES_OK;
}


// Write clock reps instructions to cache
scpi_result_t SCPI_ClockDataReps(
    scpi_t* context
) {
    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID (not used)
    uint32_t clock_id = 0;
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Reset the buffer
    clock_sequencer_cache_reps_clear();

    // Now get the instruction buffer, if present
    if (!SCPI_ParamArrayUInt32(
        context,
        clock_sequence_buffer_reps,
        clock_sequence_buffer_size,
        &clock_sequence_buffer_reps_read,
        0, // scpi array format type??
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query clock reps instructions from cache
scpi_result_t SCPI_ClockDataRepsQ(
    scpi_t* context
) {
    // Get clock sequencer ID (not used)
    uint32_t clock_id = 0;
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultArrayUInt32(
        context,
        clock_sequence_buffer_reps,
        clock_sequence_buffer_size,
        0 // what is scpi array format??
    );

    return SCPI_RES_OK;
}


// Write frequency instructions to cache
scpi_result_t SCPI_ClockDataFreq(
    scpi_t* context
) {
    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID (not used)
    uint32_t clock_id = 0;
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Clear delay cache
    clock_sequencer_cache_freq_clear();

    // Now get the instruction buffer, if present
    if (!SCPI_ParamArrayDouble(
        context,
        clock_sequence_buffer_freq,
        clock_sequence_buffer_size,
        &clock_sequence_buffer_freqs_read,
        0, // scpi array format type??
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query clock freq instructions from cache
scpi_result_t SCPI_ClockDataFreqQ(
    scpi_t* context
) {
    // Get clock sequencer ID (not used)
    uint32_t clock_id = 0;
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultArrayDouble(
        context,
        clock_sequence_buffer_freq,
        clock_sequence_buffer_size,
        0 // what is scpi array format??
    );

    return SCPI_RES_OK;
}


// Clear the instructions in cache
scpi_result_t SCPI_ClockDataClear(
    scpi_t* context
) {
    // Allocate some variables
    size_t instructions_read=0;

    // Get clock sequencer ID (not used)
    uint32_t clock_id = 0;
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Reset the buffer
    clock_sequencer_cache_reps_clear();
    clock_sequencer_cache_freq_clear();

    return SCPI_RES_OK;
}


// Set the clock instructions at clock sequencer N
scpi_result_t SCPI_ClockDataApply(
    scpi_t* context
) {
    // Allocate some variables
    uint32_t clock_id = 0;
    uint32_t freq_cycles = 0;

    uint32_t local_buffer[CLOCK_INSTRUCTIONS_MAX] = {};
    const uint32_t local_buffer_size = CLOCK_INSTRUCTIONS_MAX;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Make sure both instruction buffers have read same number of elements
    if (clock_sequence_buffer_freqs_read != clock_sequence_buffer_reps_read)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_LISTS_NOT_SAME_LENGTH
        );

        return SCPI_RES_ERR;
    }

    // Now get unit conversion paramerters
    struct clock_config* config_array = sequencer_clock_config_get();

    const double unit_offset = config_array[clock_id].unit_offset;
    const uint clock_divider = config_array[clock_id].clock_divider;

    for (uint32_t i = 0, j = 0; i < clock_sequence_buffer_size; i++, j+=2)
    {
        uint32_t reps = clock_sequence_buffer_reps[i];
        double freq = clock_sequence_buffer_freq[i];

        if (freq > SEQUENCER_DOUBLE_EPS)
        {
            // Convert delay into nanoseconds
            freq = 1e9 / (freq * unit_offset);

            // Convert nanoseconds to cycles, if possible
            if (!convert_nanos_to_cycles(
                (uint64_t) freq, // This truncates decimals similar to floor operation (e.g, 120.5 ns --> 120 ns)
                clock_divider,
                &freq_cycles
            )) {
                SCPI_ErrorPush(
                    context, 
                    SCPI_ERROR_DATA_OUT_OF_RANGE
                );

                return SCPI_RES_ERR;
            } 

            // Check for min cycles due to clock sequencer operations
            if (freq_cycles < CLOCK_INSTRUCTION_MIN)
            {
                SCPI_ErrorPush(
                    context, 
                    SCPI_ERROR_DATA_OUT_OF_RANGE
                );

                return SCPI_RES_ERR;
            }
        }
        // If the frequency is close to zero, set clock cycles to 0
        else
        {
            freq_cycles = 0;
        }

        // Reduce repitions by one if greater than one iteration due to the way the PIO program works
        if (reps > 1)
        {
            reps -= CLOCK_INSTRUCTION_OFFSET;
        }

        local_buffer[j] = reps;
        local_buffer[j+1] = freq_cycles;
    }

    bool success = clock_instructions_load(
        clock_id,
        local_buffer
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query instructions at clock sequencer N
scpi_result_t SCPI_ClockDataQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    SCPI_ResultArrayUInt32(
        context,
        config_array[clock_id].instructions,
        CLOCK_INSTRUCTIONS_MAX,
        0 // what is scpi array format??
    );
    
    return SCPI_RES_OK;
}


// Set clock mode at clock sequencer N
scpi_result_t SCPI_ClockMode(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint32_t clock_mode = 1;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid clock mode choices
    const scpi_choice_def_t options[] = {
        {"INTernal",         CLOCK_SCPI_MODE_FREERUN},
        {"EXTernal",         CLOCK_SCPI_MODE_SEQUENCER},
        SCPI_CHOICE_LIST_END
    };

    // Now get the clock divider if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    // Cast to usable type
    clock_mode = (uint32_t) choice;

    bool success = sequencer_clock_mode_set(
        clock_id,
        clock_mode
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query trigger mode at clock sequencer N
scpi_result_t SCPI_ClockModeQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the mode of the clock sequencer at clock_id
    uint32_t mode = config_array[clock_id].clock_mode;

    SCPI_ResultUInt32(
        context,
        mode
    );
    
    return SCPI_RES_OK;
}


// Set trigger mode at clock sequencer N
scpi_result_t SCPI_TriggerMode(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint32_t trigger_mode = 1;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid clock mode choices
    const scpi_choice_def_t options[] = {
        {"IMMediate",   CLOCK_TRIG_SOURCE_IMMEDIATE},
        {"EDGE",        CLOCK_TRIG_SOURCE_EDGE},
        {"GATE",        CLOCK_TRIG_SOURCE_GATE},
        SCPI_CHOICE_LIST_END
    };

    // Now get the trigger mode if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    // Cast to usable type
    trigger_mode = (uint32_t) choice;

    bool success = sequencer_trigger_mode_set(
        clock_id,
        trigger_mode
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query trigger mode at clock sequencer N
scpi_result_t SCPI_TriggerModeQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the mode of the clock sequencer at clock_id
    uint32_t mode = config_array[clock_id].trigger_source;

    SCPI_ResultUInt32(
        context,
        mode
    );
    
    return SCPI_RES_OK;
}


// Set trigger edge at clock sequencer N
scpi_result_t SCPI_TriggerEdge(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint32_t trigger_edge = 1;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid clock mode choices
    const scpi_choice_def_t options[] = {
        {"POSitive",    CLOCK_TRIG_EDGE_POSITIVE},
        {"NEGative",    CLOCK_TRIG_EDGE_NEGATIVE},
        SCPI_CHOICE_LIST_END
    };

    // Now get the trigger mode if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    // Cast to usable type
    trigger_edge = (uint32_t) choice;

    bool success = sequencer_trigger_edge_set(
        clock_id,
        trigger_edge
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query trigger edge at clock sequencer N
scpi_result_t SCPI_TriggerEdgeQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the mode of the clock sequencer at clock_id
    uint32_t mode = config_array[clock_id].trigger_edge;

    SCPI_ResultUInt32(
        context,
        mode
    );
    
    return SCPI_RES_OK;
}


// Set trigger level at clock sequencer N
scpi_result_t SCPI_TriggerLevel(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint32_t trigger_level = 1;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid clock mode choices
    const scpi_choice_def_t options[] = {
        {"HIGH",    CLOCK_GATE_LEVEL_HIGH},
        {"LOW",     CLOCK_GATE_LEVEL_LOW},
        SCPI_CHOICE_LIST_END
    };

    // Now get the trigger mode if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    // Cast to usable type
    trigger_level = (uint32_t) choice;

    bool success = sequencer_trigger_level_set(
        clock_id,
        trigger_level
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query trigger level at clock sequencer N
scpi_result_t SCPI_TriggerLevelQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the mode of the clock sequencer at clock_id
    uint32_t mode = config_array[clock_id].trigger_level;

    SCPI_ResultUInt32(
        context,
        mode
    );
    
    return SCPI_RES_OK;
}


// Set trigger pin at clock sequencer N
scpi_result_t SCPI_TriggerPin(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t param = 0;
    uint trigger_pin_id = 0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Now get the trigger pin if present
    if (!SCPI_ParamUInt32(context, &param, TRUE))
    {
        return SCPI_RES_ERR;
    }

    trigger_pin_id = (uint) param;

    // Validate the ID
    if (!trigger_id_validate(trigger_pin_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    bool success = trigger_pin_set(
        clock_id,
        trigger_pin_id
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query clock pin at clock sequencer N
scpi_result_t SCPI_TriggerPinQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the trigger pin of the trigger sequencer at ckock_id
    uint32_t trigger_pin = config_array[clock_id].trigger_pin;

    // Return as uint32
    SCPI_ResultUInt32(
        context,
        trigger_pin
    );
    
    return SCPI_RES_OK;
}


// Set clock data at clock sequencer N
scpi_result_t SCPI_TriggerUnits(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint64_t unit_offset = 0;

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Valid clock cycle resolution choices
    const scpi_choice_def_t options[] = {
        {"NS",  NANOSECOND},
        {"US",  MICROSECOND},
        {"MS",  MILLISECOND},
        {"S",   SECOND},
        {"M",   MINUTE},
        {"H",   HOUR},
        SCPI_CHOICE_LIST_END
    };

    // Now get the clock divider if present
    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    switch (choice)
    {
        case NANOSECOND:
            unit_offset = OFFSET_NANOSECOND;
            break;
        case MICROSECOND:
            unit_offset = OFFSET_MICROSECOND;
            break;
        case MILLISECOND:
            unit_offset = OFFSET_MILLISECOND;
            break;
        case SECOND:
            unit_offset = OFFSET_SECOND;
            break;
        case MINUTE:
            unit_offset = OFFSET_MINUTE;
            break;
        case HOUR:
            unit_offset = OFFSET_HOUR;
            break;
        default:
            unit_offset = OFFSET_NANOSECOND;
    }

    // Cast choice into usable clock divider type
    bool success = clock_trigger_unit_offset_set(
        clock_id,
        unit_offset
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query clock data units offset at clock sequencer N
scpi_result_t SCPI_TriggerUnitsQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the clock divider of the clock sequencer at clock_id
    double unit_offset = config_array[clock_id].unit_offset_trigger;

    // Return as uint32
    SCPI_ResultDouble(
        context,
        unit_offset
    );
    
    return SCPI_RES_OK;
}


// Set clock trigger skips at clock sequencer N
scpi_result_t SCPI_TriggerSkip(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t param = 0;
    uint32_t trigger_skips = 0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Now get the trigger delay if present
    if(!SCPI_ParamUInt32(context, &trigger_skips, TRUE))
    {
        return SCPI_RES_ERR;
    } 

    // This is not ideal, but there should only ever be two external trigger instructions and never more
    const bool success = trigger_skip_set(
        clock_id,
        trigger_skips
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query instructions at clock sequencer N
scpi_result_t SCPI_TriggerSkipQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    const uint32_t skips = config_array[clock_id].trigger_config[0]; // skips is index 0

    SCPI_ResultUInt32(
        context,
        skips
    );
    
    return SCPI_RES_OK;
}


// Set clock trigger delay at clock sequencer N
scpi_result_t SCPI_TriggerDelay(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t param = 0;
    uint32_t trigger_delay_cycles = 0;
    double trigger_delay = 0;
    double delay = 0.0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Now get the trigger delay if present
    if(!SCPI_ParamDouble(context, &trigger_delay, TRUE))
    {
        return SCPI_RES_ERR;
    } 

    // Now get unit conversion paramerters
    struct clock_config* config_array = sequencer_clock_config_get();

    const double unit_offset = config_array[clock_id].unit_offset_trigger;
    const uint clock_divider = config_array[clock_id].clock_divider;

    // Convert delay into nanoseconds
    delay = trigger_delay * unit_offset;

    // Convert nanoseconds to cycles, if possible
    if (!convert_nanos_to_cycles(
        (uint64_t) delay, // This truncates decimals similar to floor operation (e.g, 120.5 ns --> 120 ns)
        clock_divider,
        &trigger_delay_cycles
    )) {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    // This is not ideal, but there should only ever be two external trigger instructions and never more
    const bool success = trigger_delay_set(
        clock_id,
        trigger_delay_cycles
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query instructions at clock sequencer N
scpi_result_t SCPI_TriggerDelayQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    const uint32_t delays = config_array[clock_id].trigger_config[1]; // delays is index 1

    SCPI_ResultUInt32(
        context,
        delays
    );
    
    return SCPI_RES_OK;
}


// Set clock trigger count at clock sequencer N
scpi_result_t SCPI_TriggerCount(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t param = 0;
    uint32_t trigger_count = 0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Now get the trigger delay if present
    if(!SCPI_ParamUInt32(context, &trigger_count, TRUE))
    {
        return SCPI_RES_ERR;
    } 

    // This is not ideal, but there should only ever be two external trigger instructions and never more
    const bool success = trigger_count_set(
        clock_id,
        trigger_count
    );

    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


// Query instructions at clock sequencer N
scpi_result_t SCPI_TriggerCountQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    const uint32_t counts = config_array[clock_id].trigger_reps;

    SCPI_ResultUInt32(
        context,
        counts
    );
    
    return SCPI_RES_OK;
}


// Reset clock sequencer N
scpi_result_t SCPI_ClockReset(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    if (SCPI_check_clock_id_and_append_error(
        context,
        &clock_id
    )) {
        return SCPI_RES_ERR;
    }

    bool success = clock_sequencer_state_reset(
        clock_id
    );
    
    // If for some wierd reason we failed, raise an error
    if (!success)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PARAMETER_ERROR
        );

        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}