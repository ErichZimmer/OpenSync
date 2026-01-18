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


#define STATUS_ON TRUE
#define STATUS_OFF FALSE

const uint32_t INTERNAL = 0;
const uint32_t RISING = 1;


// Enable a clock sequencer at ID N
scpi_result_t SCPI_ClockEnable(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
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

    // Now enable that clock channel
    bool success = clock_sequencer_state_set(
        clock_id,
        STATUS_ON
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


// Disable a clock sequencer at ID N
scpi_result_t SCPI_ClockDisable(
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

    // Now disable that clock channel
    bool success = clock_sequencer_state_set(
        clock_id,
        STATUS_OFF
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
        {"HIGH_res",            CLOCK_DIV_RES_HIGH},
        {"MED_res",             CLOCK_DIV_RES_MED},
        {"LOW_res",             CLOCK_DIV_RES_LOW},
        {"VERY_LOW_res",        CLOCK_DIV_RES_VERY_LOW},
        {"VERY_VERY_LOW_res",  CLOCK_DIV_RES_VERY_VERY_LOW},
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


// Set the instructiond at clock sequencer N
scpi_result_t SCPI_ClockInstructions(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t clock_id = 0;
    uint32_t sequence_buffer[CLOCK_INSTRUCTIONS_MAX] = {};
    uint32_t sequence_buffer_size = CLOCK_INSTRUCTIONS_MAX;
    size_t buffer_instructions_read = 0;

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

    // Now get the instruction buffer, if present
    if (!SCPI_ParamArrayUInt32(
        context,
        sequence_buffer,
        sequence_buffer_size,
        &buffer_instructions_read,
        0, // scpi array format type??
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    // Check the length of this instruction buffer
    if (buffer_instructions_read > CLOCK_INSTRUCTIONS_MAX)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_TOO_MUCH_DATA
        );

        return SCPI_RES_ERR;
    }

    // Make sure instructions are sent in pairs of two
    if (buffer_instructions_read % 2)
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_TOO_MUCH_DATA
        );

        return SCPI_RES_ERR;
    }

    // Now, check each value to make sure it is sane
    for (uint32_t i = 0; i < CLOCK_INSTRUCTIONS_MAX; i+=2)
    {
        uint32_t reps = sequence_buffer[i];
        uint32_t delay = sequence_buffer[i+1];

        // Make sure that the clock cycles are not too small
//        if (delay < ????????)
//        {
//            SCPI_ErrorPush(
//                context, 
//                SCPI_ERROR_DATA_OUT_OF_RANGE
//            );
//
//            return SCPI_RES_ERR;
//        }

        // Reduce repitions by one if greater than one iteration due to the way the PIO program works
        if (reps > 1)
        {
            reps -= CLOCK_INSTRUCTION_OFFSET;
        }

        sequence_buffer[i] = reps;
        sequence_buffer[i+1] = delay;
    }

    bool success = clock_instructions_load(
        clock_id,
        sequence_buffer
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
scpi_result_t SCPI_ClockInstructionsQ(
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


// Set clock trigger mode to internal at clock sequencer N
scpi_result_t SCPI_ClockTriggerInternal(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
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

    bool success = sequencer_clock_type_set(
        clock_id,
        INTERNAL
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


// Set clock trigger mode to rising at clock sequencer N
scpi_result_t SCPI_ClockTriggerRisingEdge(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
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

    bool success = sequencer_clock_type_set(
        clock_id,
        RISING
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


// Set clock trigger instructions at clock sequencer N
scpi_result_t SCPI_ClockTriggerInstructions(
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
scpi_result_t SCPI_ClockTriggerInstructionsQ(
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