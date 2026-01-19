#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "scpi/scpi.h"

#include "system/core_1.h"
#include "structs/clock_config.h"
#include "structs/pulse_config.h"
#include "sequencer/sequencer_common.h"
#include "scpi_common.h"

#define STATUS_ON TRUE
#define STATUS_OFF FALSE

const uint32_t OFFSET_TERM = 2; // offset from last valid instruction of buffer


// Enable a pulse sequencer at ID N
scpi_result_t SCPI_PulseEnable(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;

    // If the system status is note (IDLE) or 5 (ABORTED), return an error
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Now enable that pulse channel
    bool success = pulse_sequencer_state_set(
        pulse_id,
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


// Disable a pulse sequencer at ID N
scpi_result_t SCPI_PulseDisable(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Now enable that pulse channel
    bool success = pulse_sequencer_state_set(
        pulse_id,
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


// Query a pulse sequencer at ID N to see if it is active
scpi_result_t SCPI_PulseStatusQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Retrieve pulse sequencer container
    struct pulse_config* config_array = sequencer_pulse_config_get();

    // Get the state of the pulse sequencer at pulse_id
    bool pulse_id_state = config_array[pulse_id].active;

    // Return as bool
    SCPI_ResultBool(
        context,
        pulse_id_state
    );

    return SCPI_RES_OK;
}


// Set clock divider at pulse sequencer N
scpi_result_t SCPI_PulseClockDivider(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;
    uint32_t pulse_id = 0;
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

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
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

    bool success = pulse_divider_set(
        pulse_id,
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


// Query clock divider at pulse sequencer N
scpi_result_t SCPI_PulseClockDividerQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Retrieve pulse sequencer container
    struct pulse_config* config_array = sequencer_pulse_config_get();

    // Get the clock divider of the pulse sequencer at pulse_id
    uint32_t clock_divider = config_array[pulse_id].clock_divider;

    // Return as uint32
    SCPI_ResultUInt32(
        context,
        clock_divider
    );
    
    return SCPI_RES_OK;
}


// Set clock pin at pulse sequencer N
scpi_result_t SCPI_PulsePin(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;
    uint32_t param = 0;
    uint clock_pin_id = 0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Now get the clock divider if present
    if (!SCPI_ParamUInt32(context, &param, TRUE))
    {
        return SCPI_RES_ERR;
    }

    clock_pin_id = (uint) param;

    bool success = pulse_pin_clock_set(
        pulse_id,
        clock_pin_id
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


// Query clock pin at pulse sequencer N
scpi_result_t SCPI_PulsePinQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Retrieve pulse sequencer container
    struct pulse_config* config_array = sequencer_pulse_config_get();

    // Get the clock pin of the pulse sequencer at pulse_id
    // NOTE: Should we return clock pin ID instead?
    uint32_t clock_pin = config_array[pulse_id].clock_pin;

    // Return as uint32
    SCPI_ResultUInt32(
        context,
        clock_pin
    );
    
    return SCPI_RES_OK;
}


// Set instruction output state at pulse sequencer N via char buffer
scpi_result_t SCPI_PulseInstructions(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;
    uint32_t sequence_buffer[PULSE_INSTRUCTIONS_MAX] = {};
    uint32_t sequence_buffer_size = PULSE_INSTRUCTIONS_MAX;
    size_t buffer_instructions_read = 0;

    // !If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if(is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Configure default buffer pattern
    /*
    Important Note:
    The pulse output instruction are broken up into two parts: the output
    state and the delay duration to the next instruction. For instance,
    every even instruction starting from zero (e.g., 0, 2, 4, 6, 8...) sets
    the output state. Only the first 12 bits are used to set the output state
    while the rest are discarded. The delay instructions are at every odd
    instruction index (e.g., 1, 3, 5, 7, 9...) and are used in a delay loop.
    The final two instruction at the end of the instruction buffer are used
    to set the output state to idle and a terminating flag.

    // DO NOT FORGET TO SET THE TERMINATING FLAGS (0) AND MAKE SURE ALL OTHER INSTRUCTIONS ARE NON-ZERO!!!!
    */

    // Set all delay instructions to 1 (they can't be zero)
    for (int i = 1; i < PULSE_INSTRUCTIONS_MAX; i += 2)
    {
        sequence_buffer[i] = PULSE_INSTRUCTION_OFFSET + 1; // 6 cycle delay whhich maps to 1 cycle delay after offset
    }

    // Make sure last two elements are zero
    sequence_buffer[PULSE_INSTRUCTIONS_OUTPUT_TERM] = SEQUENCE_FLAG_END;
    sequence_buffer[PULSE_INSTRUCTIONS_DELAY_TERM] = SEQUENCE_FLAG_END;

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
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
    if (buffer_instructions_read > PULSE_INSTRUCTIONS_MAX - OFFSET_TERM)
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
    // lol, I noticed a coding format change on if statements, probaby should figure my shit out
    for (uint32_t i = 0; i < PULSE_INSTRUCTIONS_MAX - OFFSET_TERM; i+=2)
    {
        uint32_t output = sequence_buffer[i];
        uint32_t delay = sequence_buffer[i+1];

        // Validate output
        if (output & ~OUT_MASK)
        {
            SCPI_ErrorPush(
                context, 
                SCPI_ERROR_DATA_OUT_OF_RANGE
            );

            return SCPI_RES_ERR;
        }

        // Validate delay cycles
        if (delay < (PULSE_INSTRUCTION_OFFSET + 1))
        {
            SCPI_ErrorPush(
                context, 
                SCPI_ERROR_DATA_OUT_OF_RANGE
            );

            return SCPI_RES_ERR;
        }

        // If all is good, go ahead and offset the delay
        sequence_buffer[i+1] = delay - PULSE_INSTRUCTION_OFFSET;
    }

    bool success = pulse_instructions_load(
        pulse_id,
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


// Query instructions at pulse sequencer N
scpi_result_t SCPI_PulseInstructionsQ(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if (!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    // Retrieve pulse sequencer container
    struct pulse_config* config_array = sequencer_pulse_config_get();

    // Get the instructions of the pulse sequencer at pulse_id
    for (uint32_t i = 0; i < PULSE_INSTRUCTIONS_MAX; i++)
    {
        // Return as uint32
        SCPI_ResultUInt32(
            context,
            config_array[pulse_id].instructions[i]
        );
    }
    
    return SCPI_RES_OK;
}


// Reset pulse sequencer N
scpi_result_t SCPI_PulseReset(
    scpi_t* context
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    uint32_t pulse_id = 0;

    // If the system status is not 0 (IDLE) or 5 (ABORTED), return an error
    if (is_running())
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_PROGRAM_CURRENTLY_RUNNING
        );

        return SCPI_RES_ERR;
    }

    // Get pulse sequencer ID
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // default value, e.g., pulse id 0
    );

    // Cast numbers to usable int type
    pulse_id = (uint32_t) numbers[0];

    // Validate the ID
    // NOTE: We use clock id validate due to clock id and pulse id sharign same ids
    if(!clock_id_validate(pulse_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    bool success = pulse_sequencer_state_reset(
        pulse_id
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