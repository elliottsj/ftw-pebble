#include <pebble.h>
#include "tabs_menu.h"

static SimpleMenuLayer *simple_menu_layer;
static SimpleMenuItem items[2];
static SimpleMenuSection sections[1];
static GBitmap *nearby_icon;
static GBitmap *saved_icon;

static void (*nearby_selected_callback)(void);
static void (*saved_selected_callback)(void);

// /*
//  * Called when the user selects a menu item.
//  * Open the stop window with the selected stop information.
//  */
// static void menu_select_callback(MenuLayer *simple_menu_layer, MenuIndex *cell_index, void *data) {
//     // current_section = stop_list->sections[cell_index->section];
//     // current_stop = current_section->stops[cell_index->row];
//     // window_stack_push(stop_window, true /* animated */);
// }

static void call_nearby_selected_callback(int index, void *context) {
    nearby_selected_callback();
}

static void call_saved_selected_callback(int index, void *context) {
    saved_selected_callback();
}

/*
 * Called when the window is first pushed to the screen when it's not already loaded.
 * Do the layout of the window.
 */
static void tabs_menu_window_load(Window *window) {
    // Retrieve icon resources
    nearby_icon = gbitmap_create_with_resource(RESOURCE_ID_LOCATION_ICON);
    saved_icon = gbitmap_create_with_resource(RESOURCE_ID_STAR_ICON);
    
    // Create the menu items
    items[0] = (SimpleMenuItem) {
        .icon = nearby_icon,
        .title = "NEARBY",
        .callback = call_nearby_selected_callback
    };
    items[1] = (SimpleMenuItem) {
        .icon = saved_icon,
        .title = "SAVED",
        .callback = call_saved_selected_callback
    };

    // Create the menu section
    sections[0] = (SimpleMenuSection) {
        .items = items,
        .num_items = 2,
        .title = "Stops"
    };

    // Create the menu layer
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);
    simple_menu_layer = simple_menu_layer_create(bounds, window, sections, 1 /* num_sections */, NULL /* callback_context */);

    // Add it to the window for display
    layer_add_child(window_layer, simple_menu_layer_get_layer(simple_menu_layer));
}

/*
 * Called when the window resumes after already being loaded.
 */
static void tabs_menu_window_appear(Window *window) {

}

/*
 * Called when the window leaves the screen.
 */
static void tabs_menu_window_disappear(Window *window) {

}

/*
 * Called when the window is removed from the window stack.
 * Destroy any layers associated with the window.
 */
static void tabs_menu_window_unload(Window *window) {
    simple_menu_layer_destroy(simple_menu_layer);
}

/*
 * Create the menu window and set handlers.
 * Return the created window.
 */
Window *init_tabs_menu_window(void (*on_nearby_selected)(void), void (*on_saved_selected)(void)) {
    // Save callback functions
    nearby_selected_callback = on_nearby_selected;
    saved_selected_callback = on_saved_selected;

    Window *tabs_menu_window = window_create();
    window_set_window_handlers(tabs_menu_window, (WindowHandlers) {
        .load = tabs_menu_window_load,
        .appear = tabs_menu_window_appear,
        .disappear = tabs_menu_window_disappear,
        .unload = tabs_menu_window_unload,
    });
    return tabs_menu_window;
}
