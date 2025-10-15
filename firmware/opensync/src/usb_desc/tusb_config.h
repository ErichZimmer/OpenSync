#pragma once


//***********************//
//* BOARD CONFIGURATION *//
//***********************//

// RHPort number (Root Hub Port); 0 = single USB port (USB0); I think
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT        0
#endif

#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)

#define BOARD_DEVICE_RHPORT_SPEED   OPT_MODE_FULL_SPEED


//************************//
//* COMMON CONFIGURATION *//
//************************//

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN      __attribute__ ((aligned(4)))
#endif


//************************//
//* DEVICE CONFIGURATION *//
//************************//

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE  64
#endif


//***********************//
//* CLASS CONFIGURATION *//
//***********************//

#define CFG_TUD_CDC               1
#define CFG_TUD_HID               0
#define CFG_TUD_MSC               0
#define CFG_TUD_MIDI              0
#define CFG_TUD_VENDOR            0

#define CFG_TUD_CDC_RX_BUFSIZE   64
#define CFG_TUD_CDC_TX_BUFSIZE   64