#include "scpi_sequencer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "scpi/scpi.h"

#include "structs/pulse_scpi_config.h"
#include "sequencer/sequencer_common.h"
#include "serial/scpi_common.h"


static struct pulse_scpi_config sequencer_scpi_config[MAX_CLOCK_SYNCS + MAX_PULSE_CHANNELS];

// Return the scpi config with current channel configuration
struct pulse_scpi_config* sequencer_scpi_config_get()
{
    return sequencer_scpi_config;
}


void pulse_channels_clear()
{
    for (uint32_t i=0; i < MAX_CLOCK_SYNCS + MAX_PULSE_CHANNELS; i++)
    {
        sequencer_scpi_config[i] = (struct pulse_scpi_config) {0};
        sequencer_scpi_config[i].clock_divider = 1;
        sequencer_scpi_config[i].pcounter = 1;
    }
}


// Get pulse channel id
bool SCPI_get_channel_id(
    scpi_t* context,
    uint32_t* channel_id
) {
    // Allocate some variables
    int32_t numbers[1] = {0};
    int32_t choice = 0;

    // Get channel id, if specified
    SCPI_CommandNumbers(
        context,
        numbers, 
        1, // length (e.g., 1 element array)
        0 // 0 means to use T0/clock sequencer
    );

    // Check if user specified sequencer ID
    if (numbers[0] == 0)
    {
        *channel_id = 0u;
    }
    else
    {
        // Cast numbers to usable int type
        *channel_id = (uint32_t) numbers[0];
    }

    return false;
}


bool SCPI_is_clock_sequencer(
    uint32_t channel_id
) {
    return channel_id == 0;
}


bool SCPI_is_pulse_sequencer(
    uint32_t channel_id
) {
    return channel_id >= 1 && channel_id <= MAX_PULSE_CHANNELS;
}

bool SCPI_is_valid_channel_id(
    uint32_t channel_id
) {
    if (channel_id <= MAX_PULSE_CHANNELS)
        return true;

    return false;
}


// Set clock divider for all channels
scpi_result_t SCPI_ClockDivider(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    uint32_t clock_divider = 1;

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(
        context,
        &clock_divider,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    if (clock_divider == 0)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].clock_divider = clock_divider;

    return SCPI_RES_OK;
}


// Query clock divider at clock sequencer N
scpi_result_t SCPI_ClockDividerQ(
    scpi_t* context
) {
    // Allocate some variables
    uint32_t channel_id = 0;

    // Get channel id
    SCPI_get_channel_id(
        context,
        &channel_id
    );

    // If channel id is not clock sequencer, error
    if (!SCPI_is_valid_channel_id(channel_id))
    {
         SCPI_ErrorPush(
            context, 
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );
        
        return SCPI_RES_ERR;
    }

    // Return as uint32
    SCPI_ResultUInt32(
        context,
        sequencer_scpi_config[channel_id].clock_divider
    );
    
    return SCPI_RES_OK;
}

// Set the status of a clock sequencer at ID N
scpi_result_t SCPI_State(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    bool state = false;

    // If the system status is note (IDLE) or 5 (ABORTED), return an error
    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    // Get channel id
    SCPI_get_channel_id(
        context,
        &channel_id
    );

    // Check if channel id is valid
    if (!SCPI_is_valid_channel_id(channel_id))
    {
         SCPI_ErrorPush(
            context, 
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );
        
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamBool(context, &state, TRUE)) {
        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].state = state;

    return SCPI_RES_OK;
}


// Query a clock sequencer at ID N to see if it is active
scpi_result_t SCPI_StateQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    // Get channel id
    SCPI_get_channel_id(
        context,
        &channel_id
    );

    // Check if channel id is valid
    if (!SCPI_is_valid_channel_id(channel_id))
    {
         SCPI_ErrorPush(
            context, 
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );
        
        return SCPI_RES_ERR;
    }

    bool channel_state = sequencer_scpi_config[channel_id].state;

    // Return as bool
    SCPI_ResultBool(
        context,
        channel_state
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_ChannelOutputMode(
    scpi_t* context
) {
    int32_t choice = 0;
    uint32_t channel_id = 0;

    const scpi_choice_def_t options[] = {
        {"LVTtl", 0},
        {"TTL",   1},
        SCPI_CHOICE_LIST_END
    };

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].level =
        (uint) choice;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_ChannelOutputModeQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    const char* result = NULL;

    const scpi_choice_def_t options[] = {
        {"LVTtl", 0},
        {"TTL",   1},
        SCPI_CHOICE_LIST_END
    };

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(
        options,
        (int32_t) sequencer_scpi_config[channel_id].level,
        &result
    )) {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_CORRUPT
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(
        context,
        result
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_Period(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    uint32_t clock_divider = 1;
    scpi_number_t period = {0};

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    // Get clock divider since we use it for determining min/max delays
    clock_divider = sequencer_scpi_config[channel_id].clock_divider;

    if (!SCPI_ParamNumber(
        context,
        scpi_special_numbers_def,
        &period,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    if (period.special)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_ILLEGAL_PARAMETER_VALUE
        );

        return SCPI_RES_ERR;
    }

    if (period.unit != SCPI_UNIT_NONE &&
        period.unit != SCPI_UNIT_SECOND)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_INVALID_SUFFIX
        );

        return SCPI_RES_ERR;
    }

    if (period.content.value < (DELAY_CLOCK_MIN * clock_divider) ||
        period.content.value > (DELAY_CLOCK_MAX * clock_divider))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].period = (double) period.content.value;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_PeriodQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(
        context,
        sequencer_scpi_config[channel_id].period
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_BurstCounter(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    uint32_t counter = 0;

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(
        context,
        &counter,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    if (counter > BCOUNTER_MAX)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].bcounter = (uint) counter;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_BurstCounterQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32(
        context,
        sequencer_scpi_config[channel_id].bcounter
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_PulseCounter(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    uint32_t counter = 0;

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(
        context,
        &counter,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    if (counter > COUNTERS_MAX)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].pcounter = (uint) counter;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_PulseCounterQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32(
        context,
        sequencer_scpi_config[channel_id].pcounter
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_OffCounter(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    uint32_t counter = 0;

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(
        context,
        &counter,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    if (counter > COUNTERS_MAX)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].ocounter = (uint) counter;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_OffCounterQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32(
        context,
        sequencer_scpi_config[channel_id].ocounter
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_Sync(
    scpi_t* context
) {
    int32_t choice = 0;
    uint32_t channel_id = 0;

    const scpi_choice_def_t options[] = {
        {"T0",  0},
        {"CHA", 1},
        {"CHB", 2},
        {"CHC", 3},
        {"CHD", 4},
        {"CHE", 5},
        {"CHF", 6},
        {"CHG", 7},
        {"CHH", 8},
        SCPI_CHOICE_LIST_END
    };

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    if ((uint32_t) choice == channel_id)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_ILLEGAL_PARAMETER_VALUE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].sync = (uint) choice;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_SyncQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    const char* result = NULL;

    const scpi_choice_def_t options[] = {
        {"T0",  0},
        {"CHA", 1},
        {"CHB", 2},
        {"CHC", 3},
        {"CHD", 4},
        {"CHE", 5},
        {"CHF", 6},
        {"CHG", 7},
        {"CHH", 8},
        SCPI_CHOICE_LIST_END
    };

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(
        options,
        (int32_t) sequencer_scpi_config[channel_id].sync,
        &result
    )) {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_CORRUPT
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(
        context,
        result
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_Buffer(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    uint32_t instructions_read = 0;
    uint32_t clock_divider = 1;

    bool state_buffer[6] = {false};
    double delay_buffer[6] = {0.0f};

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    // Get clock divider since we use it for determining min/max delays
    clock_divider = sequencer_scpi_config[channel_id].clock_divider;

    while (instructions_read < MAX_PULSE_PAIR)
    {
        scpi_bool_t state = FALSE;
        scpi_number_t delay = {0};

        scpi_bool_t state_mandatory = instructions_read == 0 ? TRUE : FALSE;

        // Buffer structure is read in a slightly unconventional way.
        // Values are inputted in state,delay repeating pairs.
        // Ex: ON,15US,OFF,1MS

        // Get boolean state
        if (!SCPI_ParamBool(
            context,
            &state,
            state_mandatory
        )) {
            if (state_mandatory ||
                SCPI_ParamErrorOccurred(context))
            {
                return SCPI_RES_ERR;
            }

            break;
        }

        // Get double delay
        if (!SCPI_ParamNumber(
            context,
            scpi_special_numbers_def,
            &delay,
            TRUE
        )) {
            return SCPI_RES_ERR;
        }

        if (delay.special)
        {
            SCPI_ErrorPush(
                context,
                SCPI_ERROR_ILLEGAL_PARAMETER_VALUE
            );

            return SCPI_RES_ERR;
        }

        if (delay.unit != SCPI_UNIT_NONE &&
            delay.unit != SCPI_UNIT_SECOND)
        {
            SCPI_ErrorPush(
                context,
                SCPI_ERROR_INVALID_SUFFIX
            );

            return SCPI_RES_ERR;
        }

        if (delay.content.value != 0.0 && (
            delay.content.value < (DELAY_PULSE_MIN * clock_divider) ||
            delay.content.value > (DELAY_PULSE_MAX * clock_divider) ))
        {
            SCPI_ErrorPush(
                context,
                SCPI_ERROR_DATA_OUT_OF_RANGE
            );

            return SCPI_RES_ERR;
        }

        state_buffer[instructions_read] = (bool) state;
        delay_buffer[instructions_read] = (double) delay.content.value;

        instructions_read++;
    }

    // If there are more than the supported amount of instructions given, notify user
    if (instructions_read == MAX_PULSE_PAIR)
    {
        scpi_parameter_t extra_parameter;

        if (SCPI_Parameter(
            context,
            &extra_parameter,
            FALSE
        )) {
            SCPI_ErrorPush(
                context,
                SCPI_ERROR_TOO_MUCH_DATA
            );

            return SCPI_RES_ERR;
        }

        if (SCPI_ParamErrorOccurred(context))
        {
            return SCPI_RES_ERR;
        }
    }

    // Load parsed instructions into scpi config buffer
    for (size_t i = 0; i < MAX_PULSE_PAIR; i++)
    {
        sequencer_scpi_config[channel_id].state_buffer[i] =
            state_buffer[i];

        sequencer_scpi_config[channel_id].delay_buffer[i] =
            delay_buffer[i];
    }

    return SCPI_RES_OK;
}


scpi_result_t SCPI_BufferQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    for (uint32_t i = 0; i < MAX_PULSE_PAIR; i++)
    {
        SCPI_ResultMnemonic(
            context,
            sequencer_scpi_config[channel_id].state_buffer[i]
                ? "ON"
                : "OFF"
        );

        SCPI_ResultDouble(
            context,
            sequencer_scpi_config[channel_id].delay_buffer[i]
        );
    }

    return SCPI_RES_OK;
}


scpi_result_t SCPI_WaitCounter(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    uint32_t counter = 0;

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(
        context,
        &counter,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    if (counter > COUNTERS_MAX)
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_OUT_OF_RANGE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].wcounter = (uint) counter;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_WaitCounterQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt32(
        context,
        sequencer_scpi_config[channel_id].wcounter
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_GateMode(
    scpi_t* context
) {
    int32_t choice = 0;
    uint32_t channel_id = 0;

    const scpi_choice_def_t options[] = {
        {"DISabled", 0},
        {"PULSe",    1},
        {"OUTPut",   2},
        {"CHANnel",  3},
        SCPI_CHOICE_LIST_END
    };

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].gate_mode =
        (uint) choice;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_GateModeQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    const char* result = NULL;

    const scpi_choice_def_t options[] = {
        {"DISabled", 0},
        {"PULSe",    1},
        {"OUTPut",   2},
        {"CHANnel",  3},
        SCPI_CHOICE_LIST_END
    };

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(
        options,
        (int32_t) sequencer_scpi_config[channel_id].gate_mode,
        &result
    )) {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_CORRUPT
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(
        context,
        result
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_GateLogicAll(
    scpi_t* context,
    bool is_pulse_channel
) {
    int32_t choice = 0;
    uint32_t channel_id = 0;

    const scpi_choice_def_t options[] = {
        {"LOW",  0},
        {"HIGH", 1},
        SCPI_CHOICE_LIST_END
    };

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    // Check if channel is is valid depending on if ths is clock or pulse channel
    if (is_pulse_channel && !SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );
        
        return SCPI_RES_ERR;
    }
    else if (!is_pulse_channel && !SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );
        
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].gate_level =
        (uint) choice;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_GateLogic(
    scpi_t* context
) {
    scpi_result_t flag = SCPI_GateLogicAll(
        context,
        false
    );

    if (flag != SCPI_RES_OK)
        return SCPI_RES_ERR;
    
    return SCPI_RES_OK;
}

// Channel gate logic is tied to T0 logic
scpi_result_t SCPI_ChannelGateLogic(
    scpi_t* context
) {
    scpi_result_t flag = SCPI_GateLogicAll(
        context,
        true
    );

    if (flag != SCPI_RES_OK)
        return SCPI_RES_ERR;
    
    return SCPI_RES_OK;
}


scpi_result_t SCPI_GateLogicAllQ(
    scpi_t* context,
    bool is_pulse_channel
) {
    uint32_t channel_id = 0;
    const char* result = NULL;

    const scpi_choice_def_t options[] = {
        {"LOW",  0},
        {"HIGH", 1},
        SCPI_CHOICE_LIST_END
    };

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    // Check if channel is is valid depending on if ths is clock or pulse channel
    if (is_pulse_channel && !SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );
        
        return SCPI_RES_ERR;
    }
    else if (!is_pulse_channel && !SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context, 
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );
        
        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(
        options,
        (int32_t) sequencer_scpi_config[channel_id].gate_level,
        &result
    )) {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_CORRUPT
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(
        context,
        result
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_GateLogicQ(
    scpi_t* context
) {
    scpi_result_t flag = SCPI_GateLogicAllQ(
        context,
        false
    );

    if (flag != SCPI_RES_OK)
        return SCPI_RES_ERR;
    
    return SCPI_RES_OK;
}


scpi_result_t SCPI_ChannelGateLogicQ(
    scpi_t* context
) {
    scpi_result_t flag = SCPI_GateLogicAllQ(
        context,
        true
    );

    if (flag != SCPI_RES_OK)
        return SCPI_RES_ERR;
    
    return SCPI_RES_OK;
}


scpi_result_t SCPI_ChannelGateMode(
    scpi_t* context
) {
    int32_t choice = 0;
    uint32_t channel_id = 0;

    const scpi_choice_def_t options[] = {
        {"DISabled", 0},
        {"PULSe",    1},
        {"OUTPut",    2},
        SCPI_CHOICE_LIST_END
    };

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].gate_mode =
        (uint) choice;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_ChannelGateModeQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    const char* result = NULL;

    const scpi_choice_def_t options[] = {
        {"DISabled", 0},
        {"PULSe",    1},
        {"OUTPut",   2},
        SCPI_CHOICE_LIST_END
    };

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_pulse_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(
        options,
        (int32_t) sequencer_scpi_config[channel_id].gate_mode,
        &result
    )) {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_CORRUPT
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(
        context,
        result
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_TriggerMode(
    scpi_t* context
) {
    int32_t choice = 0;
    uint32_t channel_id = 0;

    const scpi_choice_def_t options[] = {
        {"DISabled",  0},
        {"TRIGgered", 1},
        SCPI_CHOICE_LIST_END
    };

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].trig_mode =
        (uint) choice;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_TriggerModeQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    const char* result = NULL;

    const scpi_choice_def_t options[] = {
        {"DISabled",  0},
        {"TRIGgered", 1},
        SCPI_CHOICE_LIST_END
    };

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(
        options,
        (int32_t) sequencer_scpi_config[channel_id].trig_mode,
        &result
    )) {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_CORRUPT
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(
        context,
        result
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_TriggerEdge(
    scpi_t* context
) {
    int32_t choice = 0;
    uint32_t channel_id = 0;

    const scpi_choice_def_t options[] = {
        {"RISing",  0},
        {"FALLing", 1},
        SCPI_CHOICE_LIST_END
    };

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamChoice(
        context,
        options,
        &choice,
        TRUE
    )) {
        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id].trig_edge =
        (uint) choice;

    return SCPI_RES_OK;
}


scpi_result_t SCPI_TriggerEdgeQ(
    scpi_t* context
) {
    uint32_t channel_id = 0;
    const char* result = NULL;

    const scpi_choice_def_t options[] = {
        {"RISing",  0},
        {"FALLing", 1},
        SCPI_CHOICE_LIST_END
    };

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_clock_sequencer(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    if (!SCPI_ChoiceToName(
        options,
        (int32_t) sequencer_scpi_config[channel_id].trig_edge,
        &result
    )) {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_DATA_CORRUPT
        );

        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(
        context,
        result
    );

    return SCPI_RES_OK;
}


scpi_result_t SCPI_Reset(
    scpi_t* context
) {
    uint32_t channel_id = 0;

    if (SCPI_check_running_and_append_error(context))
    {
        return SCPI_RES_ERR;
    }

    SCPI_get_channel_id(
        context,
        &channel_id
    );

    if (!SCPI_is_valid_channel_id(channel_id))
    {
        SCPI_ErrorPush(
            context,
            SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE
        );

        return SCPI_RES_ERR;
    }

    sequencer_scpi_config[channel_id] = (struct pulse_scpi_config) {0};
    sequencer_scpi_config[channel_id].clock_divider = 1;
    sequencer_scpi_config[channel_id].pcounter = 1;

    return SCPI_RES_OK;
}