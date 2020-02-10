#include <application.h>

#include "qrcodegen.h"


// Defaults
// #define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)
// #define SERVICE_INTERVAL_INTERVAL (60 * 60 * 1000)

// LED instance
bc_led_t led;
bc_led_t led_lcd_red;
bc_led_t led_lcd_blue;
bc_led_t led_lcd_green;

// Button instance
bc_button_t button;

// GFX instance
bc_gfx_t *gfx;


void bc_change_qr_value(uint64_t *id, const char *topic, void *value, void *param);
void print_qr(const uint8_t qrcode[]);
void qrcode_project(char *text);


// QR code variables
char *orderIdUrl="http://blokko.blockchainadvies.nu/receive-order.html?order=1";


// subscribe table, format: topic, expect payload type, callback, user param
static const bc_radio_sub_t subs[] = {
    {"qr/-/chng/code", BC_RADIO_SUB_PT_STRING, bc_change_qr_value, NULL}
};


void bc_change_qr_value(uint64_t *id, const char *topic, void *value, void *param)
{
    bc_log_info("bc_change_qr_value triggered.");

    bc_led_pulse(&led_lcd_blue, 2000);

    // char *newUrl = *(char*)value; // compile warning "makes pointer from integer without a cast"
    // char newUrl = value;
    char *newUrl = (char*)value;

    bc_log_info("New URL set to %s.", newUrl);
    
    orderIdUrl = newUrl;
    qrcode_project(newUrl);

}


void print_qr(const uint8_t qrcode[])
{
    bc_log_info("print_qr started");

    bc_gfx_clear(gfx);

    bc_gfx_set_font(gfx, &bc_font_ubuntu_13);
    bc_gfx_draw_string(gfx, 2, 0, orderIdUrl, true);

    uint32_t offset_x = 15;
    uint32_t offset_y = 16;
    uint32_t box_size = 3;
	int size = qrcodegen_getSize(qrcode);
	int border = 2;
	for (int y = -border; y < size + border; y++) {
		for (int x = -border; x < size + border; x++) {
            uint32_t x1 = offset_x + x * box_size;
            uint32_t y1 = offset_y + y * box_size;
            uint32_t x2 = x1 + box_size;
            uint32_t y2 = y1 + box_size;

            bc_gfx_draw_fill_rectangle(gfx, x1, y1, x2, y2, qrcodegen_getModule(qrcode, x, y));
		}
	}
    bc_gfx_update(gfx);
}


// Make and print the QR Code symbol
void qrcode_project(char *text)
{
    bc_system_pll_enable();

	static uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	static uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, qrcodegen_Ecc_MEDIUM,	qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

	if (ok)
    {
		print_qr(qrcode);
    }

    bc_system_pll_disable();
}


void application_init(void)
{
    // Initialize logging
    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);
    
    // Create log
    bc_log_info("application_init started");

    // Initialize Radio
    bc_radio_init(BC_RADIO_MODE_NODE_LISTENING); 
    bc_radio_set_subs((bc_radio_sub_t *) subs, sizeof(subs)/sizeof(bc_radio_sub_t));
    // bc_radio_pairing_request(FIRMWARE, VERSION);

    // Initialize LCD
    bc_module_lcd_init();
    gfx = bc_module_lcd_get_gfx();

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);
    // bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    // bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    // bc_button_set_event_handler(&button, button_event_handler, NULL);
  
    // Initialize LED on LCD module
    bc_led_init_virtual(&led_lcd_red, BC_MODULE_LCD_LED_RED, bc_module_lcd_get_led_driver(), true);
    bc_led_init_virtual(&led_lcd_blue, BC_MODULE_LCD_LED_BLUE, bc_module_lcd_get_led_driver(), true);
    bc_led_init_virtual(&led_lcd_green, BC_MODULE_LCD_LED_GREEN, bc_module_lcd_get_led_driver(), true);

    // Initialize battery
/*  bc_module_battery_init();
    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);
*/

    // Initialize project
    qrcode_project(orderIdUrl);

    bc_led_pulse(&led_lcd_green, 2000);

}

