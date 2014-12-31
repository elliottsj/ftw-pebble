#include <pebble.h>
#include "data.h"
#include "sync.h"

#include "windows/tabs_menu.h"
// #include "windows/stops_menu.h"
// #include "windows/stop_window.h"

// The menu listing "NEARBY" and "SAVED"
static Window *tab_menu_window;

// // The window which displays a list of stops
// static Window *stops_menu_window;

// // The window which displays information about a single stop
// static Window *stop_window;

// A list of stops by section
static StopList *stop_list;

static StopSection *current_section;
static Stop *current_stop;

/*
 * Called when the user selects the "NEARBY" menu item.
 * Wait for bluetooth if necessary, then open the stop list window, requesting nearby stops.
 */
static void on_nearby_selected(void) {

}

/*
 * Called when the user selects the "SAVED" menu item.
 * Wait for bluetooth if necessary, then open the stop list window, requesting saved stops.
 */
static void on_saved_selected(void) {

}

/*
 * When bluetooth connection is available, initialize data sync
 */
static void on_bluetooth_connection(bool connected) {
    // Initialize sync and request stop data from phone
    // init_sync();
    // sync_get_stops(on_stops_loaded);
}

/*
 * Show splash with logo and load a menu of stops from phone
 */
static void init(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Initializing Faster Than Walking");

    // Create and push the tab menu window
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating tab menu");
    tab_menu_window = init_tabs_menu_window(NULL, NULL);
    window_stack_push(tab_menu_window, true /* animated */);

    // // Create stop menu window and set window handlers
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating stop menu");
    // stop_menu_window = init_stop_menu_window();

    // // Create stop window and set window handlers
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating stop window");
    // stop_window = init_stop_window();

    // Require bluetooth connection
    if (bluetooth_connection_service_peek()) {
        // Bluetooth is connected now
        on_bluetooth_connection(true);
    } else {
        // Wait for connection
        bluetooth_connection_service_subscribe(on_bluetooth_connection);
    }
}

static void deinit(void) {
    window_destroy(tab_menu_window);
    // window_destroy(stops_menu_window);
    // window_destroy(stop_window);
    // stop_list_destroy(stop_list);
}

/*
 * Entry point
 */
int main(void) {
    // Initialize windows and register event handlers
    init();

    // Enter the main event loop. This will block until the app is ready to exit.
    app_event_loop();

    // De-allocate resources before exiting
    deinit();
}
