// Standard includes
#include "pebble.h"



// App-specific data
Window *window; // All apps must have at least one window
TextLayer *time_layer; 
TextLayer *battery_layer;
TextLayer *connection_layer;
static GBitmap *image;
static BitmapLayer *image_layer;



static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}


// Called once per second
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.

  strftime(time_text, sizeof(time_text), "%T", tick_time);
  text_layer_set_text(time_layer, time_text);

  handle_battery(battery_state_service_peek());
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(connection_layer, connected ? "connected" : "disconnected");
}

// Handle the start-up of the app
static void do_init(void) {

  // Create our app's base window
  window = window_create();
  window_stack_push(window, true);
  window_set_background_color(window, GColorWhite);
	
	

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);
	
	//my logo	
	image_layer = bitmap_layer_create(GRect(0, 0, frame.size.w /* width */, 42/* height */));
	image = gbitmap_create_with_resource(RESOURCE_ID_LOGO_IMAGE);
    bitmap_layer_set_bitmap(image_layer, image);
	bitmap_layer_set_background_color(image_layer, GColorClear);
    bitmap_layer_set_compositing_mode(image_layer, GCompOpAssign);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);

  // Init the text layer used to show the time
  time_layer = text_layer_create(GRect(0, 60, frame.size.w /* width */, 34/* height */));
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_font(time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_30)));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	

  connection_layer = text_layer_create(GRect(72, 144, /* width */ frame.size.w, 34 /* height */));
  text_layer_set_text_color(connection_layer, GColorBlack);
  text_layer_set_background_color(connection_layer, GColorClear);
  text_layer_set_font(connection_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_7)));
  text_layer_set_text_alignment(connection_layer, GTextAlignmentLeft);
  text_layer_set_text(connection_layer, "connected");

  battery_layer = text_layer_create(GRect(0, 144, /* width */ frame.size.w, 34 /* height */));
  text_layer_set_text_color(battery_layer, GColorBlack);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_FONT_7)));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentLeft);
  text_layer_set_text(battery_layer, "100% charged");

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, &handle_minute_tick);
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  layer_add_child(root_layer, text_layer_get_layer(time_layer));
  layer_add_child(root_layer, text_layer_get_layer(connection_layer));
  layer_add_child(root_layer, text_layer_get_layer(battery_layer));
}

static void do_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(time_layer);
  text_layer_destroy(connection_layer);
  text_layer_destroy(battery_layer);
  window_destroy(window);
}

// The main event/run loop for our app
int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}
