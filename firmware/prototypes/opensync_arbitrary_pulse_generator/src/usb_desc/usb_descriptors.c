// USB descriptor is based off of TinyUSB examples and Pier's YouTube video

#include "tusb.h"
#include "pico/unique_id.h"


// Apparently, this is the standard and is also used in the PrawnBlaster
#define MAX_ENDPOINT0_SIZE  64
#define ENDPOINT_BULK_SIZE  64


#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT 0x02
#define EPNUM_CDC_IN 0x82

enum{
	ITF_NUM_CDC = 0,
	ITF_NUM_CDC_DATA,
	ITF_NUM_TOTAL
};

// USB descriptor indexes
enum {
    LANGID = 0,
    MANUFACTURER,
    PRODUCT,
    SERIAL,
    INTERFACE,
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

//*********************//
//* DEVICE DESCRIPTOR *//
//*********************//

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0110, // USB 1.1

    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0x2E8A,
    .idProduct          = 0x000A,
    .bcdDevice          = 0x0100,

    .iManufacturer      = MANUFACTURER,
    .iProduct           = PRODUCT,
    .iSerialNumber      = SERIAL,

    .bNumConfigurations = 0x01
};


// Callback to get device descritor
uint8_t const* tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}


//****************************//
//* CONFIGURATION DESCRIPTOR *//
//****************************//

// full speed configuration
uint8_t const desc_configuration[] = {
	// Config number, interface count, string index, total length, attribute, power in mA
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64)
};

// Callback fuction to retriece config
uint8_t const* tud_descriptor_configuration_cb(
    uint8_t index
) {
	return desc_configuration;
}


//*********************//
//* STRING DESCRIPTOR *//
//*********************//

// RP2350 serial number array
static char usb_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

// String descriptors
char const* string_desc_arr[] = {
    [LANGID]        = (const char[]) { 0x09, 0x04 },  // Supported language ID - English (US)
    [MANUFACTURER]  = "Raspberry Pi",
    [PRODUCT]       = "RP2350B", 
    [SERIAL]        = usb_serial_str,
    [INTERFACE]     = "Board CDC",
};


// Callback to retrieve USB descriptor
uint16_t const* tud_descriptor_string_cb(
    uint8_t index,
     uint16_t langid
) {
    static uint16_t _desc_str[32 + 1];

    (void) langid;
    size_t chr_count;
    const char *str;

	if(!usb_serial_str[0])
    {
		pico_get_unique_board_id_string(
            usb_serial_str, 
            sizeof(usb_serial_str)
        );
	}

    switch (index)
    {
        case LANGID:
            memcpy(
                &_desc_str[1], 
                string_desc_arr[0], 
                2
            );
            chr_count = 1;

            break;

        case SERIAL:
        case MANUFACTURER:
        case PRODUCT:
        case INTERFACE:
            str = string_desc_arr[index];
            chr_count = strlen(str);

            // Ensure we don't overwrite the buffer
            size_t const max_count = (sizeof(_desc_str) / sizeof(_desc_str[0])) - 1;

            if (chr_count > max_count) 
            {
                chr_count = max_count;
            }

            // Convert to UTF-16
            for (size_t ii = 0; ii < chr_count; ii++)
            {
                _desc_str[1 + ii] = str[ii];
            }

            break;
            
        default:
            return NULL;
    }
}