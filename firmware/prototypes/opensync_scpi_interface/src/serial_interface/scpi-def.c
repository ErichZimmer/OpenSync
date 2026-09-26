
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/unique_id.h"
#include "scpi/scpi.h"

#include "scpi-def.h"


static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
scpi_t scpi_context;


/**
 * Reimplement IEEE488.2 *TST?
 *
 * Result should be 0 if everything is ok
 * Result should be 1 if something goes wrong
 *
 * Return SCPI_RES_OK
 */
static scpi_result_t My_CoreTstQ(scpi_t * context) {

    SCPI_ResultInt32(context, 0);

    return SCPI_RES_OK;
}


static scpi_result_t DMM_ConfigureVoltageDc(scpi_t * context) {
    double param1, param2;
    fprintf(stderr, "conf:volt:dc\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    /* read second paraeter if present */
    if (!SCPI_ParamDouble(context, &param2, FALSE)) {
        /* do something, if parameter not present */
    }

    fprintf(stderr, "\tP1=%lf\r\n", param1);
    fprintf(stderr, "\tP2=%lf\r\n", param2);

    return SCPI_RES_OK;
}


static scpi_result_t TEST_Bool(scpi_t * context) {
    scpi_bool_t param1;
    fprintf(stderr, "TEST:BOOL\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamBool(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    fprintf(stderr, "\tP1=%d\r\n", param1);

    return SCPI_RES_OK;
}


const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS", .callback = SCPI_CoreCls,},
    { .pattern = "*ESE", .callback = SCPI_CoreEse,},
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
    { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
    { .pattern = "*OPC", .callback = SCPI_CoreOpc,},
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
    { .pattern = "*RST", .callback = SCPI_CoreRst,},
    { .pattern = "*SRE", .callback = SCPI_CoreSre,},
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
    { .pattern = "*TST?", .callback = My_CoreTstQ,},
    { .pattern = "*WAI", .callback = SCPI_CoreWai,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */ 
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,}, 
    {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,}, 
    {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,}, 

/*  {.pattern = "STATus:OPERation?", .callback = scpi_stub_callback,}, */
    {.pattern = "STATus:OPERation:EVENt?", .callback = SCPI_StatusOperationEventQ,}, 
    {.pattern = "STATus:OPERation:CONDition?", .callback = SCPI_StatusOperationConditionQ,}, 
    {.pattern = "STATus:OPERation:ENABle", .callback = SCPI_StatusOperationEnable,}, 
    {.pattern = "STATus:OPERation:ENABle?", .callback = SCPI_StatusOperationEnableQ,}, 
 
    {.pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,}, 
    {.pattern = "STATus:QUEStionable:CONDition?", .callback = SCPI_StatusQuestionableConditionQ,}, 
    {.pattern = "STATus:QUEStionable:ENABle", .callback = SCPI_StatusQuestionableEnable,}, 
    {.pattern = "STATus:QUEStionable:ENABle?", .callback = SCPI_StatusQuestionableEnableQ,}, 
 
    {.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,}, 

    {.pattern = "CONFigure:VOLTage:DC", .callback = DMM_ConfigureVoltageDc,},
    {.pattern = "MEASure:CURRent:DC?", .callback = SCPI_StubQ,},

    {.pattern = "TEST:BOOL", .callback = TEST_Bool,},

    SCPI_CMD_LIST_END
};


size_t SCPI_Write(
    scpi_t * context, 
    const char * data, 
    size_t len
) {
    (void) context;

    return fwrite(data, 1, len, stdout);
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {
    (void) context;

    fprintf(stderr, "**ERROR: %d, \"%s\"\r\n", (int16_t) err, SCPI_ErrorTranslate(err));
    return 0;
}

scpi_result_t SCPI_Reset(
    scpi_t * context
) {
    (void) context;
//    initInstrument();
    return SCPI_RES_OK;   
}


scpi_interface_t scpi_interface = {
    .write = SCPI_Write,
    .error = SCPI_Error,
    .reset = SCPI_Reset,            
    .control = NULL,
    .flush = NULL,
};

void scpi_instrument_init() {
    // buffer to hold flash ID
    static char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    static bool unique_initialised = false;
    if (! unique_initialised) {
        pico_get_unique_board_id_string(serial, sizeof(serial));
        unique_initialised = true;
    }

   // initInstrument(); 

    
     SCPI_Init(
        &scpi_context,
        scpi_commands,
        &scpi_interface,
        scpi_units_def,
        SCPI_IDN1, SCPI_IDN2, serial, SCPI_IDN4,
        scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
        scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE
    );
}