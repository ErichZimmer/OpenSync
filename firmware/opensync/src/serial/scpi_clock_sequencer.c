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
const uint32_t clock_sequence_buffer_size = CLOCK_INSTRUCTIONS_MAX / 2;


// Function to clear the static buffer cache for freq data
void clock_sequencer_cache_freq_clear()
{
    for (uint32_t i = 0; i < clock_sequence_buffer_size; i++)
    {
        clock_sequence_buffer_freq[i] = 0;
    }
}


// Function to clear the static buffer cache for reps data
void clock_sequencer_cache_reps_clear()
{
    for (uint32_t i = 0; i < clock_sequence_buffer_size; i++)
    {
        clock_sequence_buffer_reps[i] = 0.0;
    }
}


// Clear all the cache
void clock_sequencer_cache_clear()
{
    clock_sequencer_cache_freq_clear();
    clock_sequencer_cache_reps_clear();
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
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

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
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clocks id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

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
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

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
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

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
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Valid frequency resolution choices
    const scpi_choice_def_t options[] = {
        {"MICROhertz",  MICROHERTZ},
        {"MILLIhertz",  MILLIHERTZ},
        {"HERTz",       HERTZ},
        {"KILOhertz",   KILOHERTZ},
        {"MEGAhertz",   MEGAHERTZ},
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
        case MICROHERTZ:
            unit_offset = 1e-6;
            break;
        case MILLIHERTZ:
            unit_offset = 1e-3;
            break;
        case HERTZ:
            unit_offset = 1.0;
            break;
        case KILOHERTZ:
            unit_offset = 1e3;
            break;
        case MEGAHERTZ:
            unit_offset = 1e6;
            break;
        default:
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
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

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
    // Allocate some variables
    size_t instructions_read=0;

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if(is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Reset the buffer
    clock_sequencer_cache_reps_clear();

    // Now get the instruction buffer, if present
    if (!SCPI_ParamArrayUInt32(
        context,
        clock_sequence_buffer_reps,
        clock_sequence_buffer_size,
        &instructions_read,
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
    // Allocate some variables
    size_t instructions_read=0;

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if(is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Clear delay cache
    clock_sequencer_cache_freq_clear();

    // Now get the instruction buffer, if present
    if (!SCPI_ParamArrayDouble(
        context,
        clock_sequence_buffer_freq,
        clock_sequence_buffer_size,
        &instructions_read,
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

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if(is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

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
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t freq_cycles = 0;

    uint32_t local_buffer[CLOCK_INSTRUCTIONS_MAX] = {};
    const uint32_t local_buffer_size = CLOCK_INSTRUCTIONS_MAX;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
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
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

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


// Set clock trigger mode at clock sequencer N
scpi_result_t SCPI_ClockTriggerMode(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t clock_id = 0;
    uint32_t clock_type = 1;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Valid clock cycle resolution choices
    const scpi_choice_def_t options[] = {
        {"INTernal",        INTERNAL},
        {"RISING",          RISING_EDGE},
        {"RISING_EDGE",     RISING_EDGE},
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
    clock_type = (uint32_t) choice;

    bool success = sequencer_clock_type_set(
        clock_id,
        clock_type
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
scpi_result_t SCPI_ClockTriggerModeQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    // Get the trigger mode of the clock sequencer at clock_id
    uint32_t clock_type = config_array[clock_id].clock_type;

    SCPI_ResultUInt32(
        context,
        clock_type
    );
    
    return SCPI_RES_OK;
}


// Set clock trigger instructions at clock sequencer N
scpi_result_t SCPI_ClockTriggerData(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t trigger_skips = 0;
    uint32_t trigger_delay = 0;
    uint32_t trigger_reps = 0;
    uint32_t param = 0;
    bool success = 0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if(is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if(!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Get the trigger skip number if present
    if(!SCPI_ParamUInt32(context, &trigger_skips, TRUE))
    {
        return SCPI_RES_ERR;
    }

    // Now get the trigger delay if present
    if(!SCPI_ParamUInt32(context, &trigger_delay, TRUE))
    {
        return SCPI_RES_ERR;
    }

    // Finally get the trigger reps if present
    if(!SCPI_ParamUInt32(context, &trigger_reps, TRUE))
    {
        return SCPI_RES_ERR;
    }

    // This is not ideal, but there should only ever be two external trigger instructions and never more
    success = clock_trigger_instructions_load(
        clock_id,
        trigger_skips,
        trigger_delay
    );

    // If loading trigger isntructions is successful, then continue to loading reps
    if (success)
    {
        success = clock_reps_trigger_set(
            clock_id,
            trigger_reps
        );
    }

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
scpi_result_t SCPI_ClockTriggerDataQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t instruction_buffer[CLOCK_TRIGGERS_MAX + 1] = {0};

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Retrieve clock sequencer container
    struct clock_config* config_array = sequencer_clock_config_get();

    instruction_buffer[0] = config_array[clock_id].trigger_config[0]; // skips
    instruction_buffer[1] = config_array[clock_id].trigger_config[1]; // delay
    instruction_buffer[2] = config_array[clock_id].trigger_reps; // reps

    SCPI_ResultArrayUInt32(
        context,
        instruction_buffer,
        CLOCK_TRIGGERS_MAX + 1,
        0 // what is scpi array format??
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
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get clock sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., clock id 0
    );

    // Cast numbers to usable int type
    clock_id = (uint32_t) numbers[0];

    // Validate the ID
    if (!clock_id_validate(clock_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

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