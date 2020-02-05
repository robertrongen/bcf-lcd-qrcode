#include <application.h>

#include "qrcodegen.h"


// Defaults
// #define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// GFX instance
bc_gfx_t *gfx;

void qrcode_project(char *project_name);

// QR code variables
char *orderIdUrl="http://blokko.blockchainadvies.nu/receive-order.html?order=1";

// Listen to MQTT messages
// How long should radio listen for messages after boot or a button press
#define INITIAL_LISTEN_INTERVAL (1 * 60 * 1000)

bc_scheduler_task_id_t listening_stopped_task_id;

void listening_stopped_handler(void* param) 
{
    bc_log_info("Listening for usage configuration stopped.");

    bc_led_set_mode(&led, BC_LED_MODE_OFF);
}

void start_listening()
{
    // bc_led_set_mode(&led, BC_LED_MODE_ON);

    int listening_interval_seconds = INITIAL_LISTEN_INTERVAL/1000;

    bc_radio_listen(INITIAL_LISTEN_INTERVAL);
    bc_radio_pub_int("core/-/listening-timeout", &listening_interval_seconds);

    bc_log_info("Waiting for counter update for next %i seconds...", listening_interval_seconds);

    bc_scheduler_plan_from_now(listening_stopped_task_id, INITIAL_LISTEN_INTERVAL);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    if (event == BC_BUTTON_EVENT_PRESS)
    {
        start_listening();
    }
}


// Subscribe to MQTT message changes
static const bc_radio_sub_t subs[] = {
    {"qr/-/chng/code", BC_RADIO_SUB_PT_STRING, bc_change_qr_value, NULL}
};
// bc_radio_set_subs(subs, sizeof(subs)/sizeof(subs[0]));


void bc_change_qr_value(uint64_t *id, const char *topic, void *value, void *param)
{
    bc_log_info("bc_change_qr_value triggered.");

    // orderIdUrl = *(char*)value; // compile warning "makes pointer from integer without a cast"
    // orderIdUrl = (char*)value;
    orderIdUrl = value;

    bc_log_info("New URL set to %s.", orderIdUrl);

    qrcode_project(orderIdUrl);

    bc_scheduler_plan_now(500);

}




static void print_qr(const uint8_t qrcode[])
{
    // Create log
    // bc_log_info("Button event handler - event: %i", event);
    bc_log_info("print_qr started");

    bc_gfx_clear(gfx);

    bc_gfx_set_font(gfx, &bc_font_ubuntu_13);
    // bc_gfx_draw_string(gfx, 2, 0, (char*)str_urls[project_index], true);
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
    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);
    bc_radio_set_subs((bc_radio_sub_t *) subs, sizeof(subs)/sizeof(bc_radio_sub_t));
    // bc_radio_set_subs(subs, sizeof(subs)/sizeof(subs[0]));
    // bc_radio_set_rx_timeout_for_sleeping_node(250);

    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);
  
    // Initialize LCD
    bc_module_lcd_init();
    gfx = bc_module_lcd_get_gfx();

    // Initialize battery
/*  bc_module_battery_init();
    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);
*/
    // Initialize project
    qrcode_project(orderIdUrl);

    // Register task to notify end of listening
    listening_stopped_task_id = bc_scheduler_register(listening_stopped_handler, NULL, BC_TICK_INFINITY);

    // Start listening for MQTT updates
    start_listening();

}
