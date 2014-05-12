#include <pebble.h>
#include "data.h"
#include "sync.h"

// Whether we are currently on the splash screen
static bool on_splash;
// Window for splash
static Window *splash_window;
// Bitmap for logo
static GBitmap *image;
// Layer for bitmap
static BitmapLayer *image_layer;

// The window which displays a list of stops
static Window *menu_window;
static MenuLayer *menu_layer;

// The window which displays information about a single stop
static Window *stop_window;
static TextLayer *route_title_layer;
static TextLayer *direction_title_layer;
static TextLayer *stop_title_layer;
static TextLayer *stop_prediction_layer;
static TextLayer *minutes_text_layer;
static char *prediction_text;

// A list of stops by section
static StopList *stop_list;

static Stop *current_stop;

/**********************************************************
 ** SPLASH
 **********************************************************/

/*
 * Called when the window is first pushed to the screen when it's not already loaded.
 * Do the layout of the window.
 */
static void splash_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(splash_window);
    GRect bounds = layer_get_frame(window_layer);

    // Set background to black
    window_set_background_color(splash_window, GColorBlack);

    // Create image layer
    image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);
    image_layer = bitmap_layer_create(bounds);
    bitmap_layer_set_bitmap(image_layer, image);
    bitmap_layer_set_alignment(image_layer, GAlignCenter);
    layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
}

/*
 * Called when the window resumes after already being loaded.
 */
static void splash_window_appear(Window *window) {

}

/*
 * Called when the window leaves the screen.
 */
static void splash_window_disappear(Window *window) {
    
}

/*
 * Called when the window is removed from the window stack.
 * Destroy any layers associated with the window.
 */
static void splash_window_unload(Window *window) {
    gbitmap_destroy(image);
    bitmap_layer_destroy(image_layer);
}

/*
 * Display a splash screen
 */
static void init_splash_window(void) {
    splash_window = window_create();
    window_set_window_handlers(splash_window, (WindowHandlers) {
        .load = splash_window_load,
        .appear = splash_window_appear,
        .disappear = splash_window_disappear,
        .unload = splash_window_unload,
    });
}

/**********************************************************
 ** MENU
 **********************************************************/

/*
 * Returns the section count of the menu
 */
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
    return stop_list->section_count;
}

/*
 * Returns the row count of the section at section_index
 */
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return stop_list->sections[section_index]->stop_count;
}

/*
 * Returns the height of the section header of the section at section_index
 */
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    // Twice the default height
    return 2 * MENU_CELL_BASIC_HEADER_HEIGHT;
}

/*
 * Draws the section header at section_index
 */
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    menu_cell_basic_header_draw(ctx, cell_layer, stop_list->sections[section_index]->section_title);
}

/*
 * Draws the section header at cell_index
 */
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    Stop *stop = stop_list->sections[cell_index->section]->stops[cell_index->row];
    char *title = stop->route_title;
    char *subtitle = stop->direction_title;
    menu_cell_basic_draw(ctx, cell_layer, title, subtitle, NULL);
}

/*
 * Called when the user selects a menu item.
 * Open the stop window with the selected stop information.
 */
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    current_stop = stop_list->sections[cell_index->section]->stops[cell_index->row];
    window_stack_push(stop_window, true /* animated */);
}

/**********************************************************
 ** MENU WINDOW
 **********************************************************/

/*
 * Called when the window is first pushed to the screen when it's not already loaded.
 * Do the layout of the window.
 */
static void menu_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_frame(window_layer);

    // Create the menu layer
    menu_layer = menu_layer_create(bounds);

    // Set all the callbacks for the menu layer
    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
    });

    // Bind the menu layer's click config provider to the window for interactivity
    menu_layer_set_click_config_onto_window(menu_layer, window);

    // Add it to the window for display
    layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

/*
 * Called when the window resumes after already being loaded.
 */
static void menu_window_appear(Window *window) {

}

/*
 * Called when the window leaves the screen.
 */
static void menu_window_disappear(Window *window) {

}

/*
 * Called when the window is removed from the window stack.
 * Destroy any layers associated with the window.
 */
static void menu_window_unload(Window *window) {

}

/*
 * Create the menu window and set handlers
 */
static void init_menu_window() {
    menu_window = window_create();
    window_set_window_handlers(menu_window, (WindowHandlers) {
        .load = menu_window_load,
        .appear = menu_window_appear,
        .disappear = menu_window_disappear,
        .unload = menu_window_unload,
    });
}

/**********************************************************
 ** STOP WINDOW
 **********************************************************/

/*
 * Returns a formatted string for displaying predictions on screen
 */
char *format_prediction(int prediction[2]) {
    if (prediction[0] < 0) return "N/A";

    char format[10];
    strcat(format, prediction[0] == 0 ? "Due" : "%d");
    if (prediction[1] >= 0) {
        strcat(format, " & ");
        strcat(format, prediction[1] == 0 ? "Due" : "%d");
    }

    char *formatted_prediction = malloc(10);
    sprintf(formatted_prediction, format, prediction[0], prediction[1]);
    return formatted_prediction;
}

/*
 * Pluralizes "minutes" if necessary, or omits it if vehicles are due
 */
char *format_minutes(int prediction[2]) {
    return "minutes";
}

/*
 * Set the text in the stop window
 */
void stop_window_set_text(char *route_title, char *direction_title, char *stop_title, int prediction[2]) {
    text_layer_set_text(route_title_layer, route_title);
    text_layer_set_text(direction_title_layer, direction_title);
    text_layer_set_text(stop_title_layer, stop_title);

    prediction_text = format_prediction(prediction);
    text_layer_set_text(stop_prediction_layer, prediction_text);
    text_layer_set_text(minutes_text_layer, "minutes");
}

/*
 * Called when the window is first pushed to the screen when it's not already loaded.
 * Do the layout of the window.
 */
static void stop_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Set background color to black
    window_set_background_color(stop_window, GColorBlack);

    // Create route_title
    route_title_layer = text_layer_create((GRect) {
        .origin = { 0, 0 },
        .size = { bounds.size.w, 28 }
    });
    text_layer_set_font(route_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(route_title_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(route_title_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_color(route_title_layer, GColorWhite);
    text_layer_set_background_color(route_title_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(route_title_layer));

    // Create direction_title
    direction_title_layer = text_layer_create((GRect) {
        .origin = { 0, 28 },
        .size = { bounds.size.w, 40 }
    });
    text_layer_set_font(direction_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(direction_title_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(direction_title_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_color(direction_title_layer, GColorWhite);
    text_layer_set_background_color(direction_title_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(direction_title_layer));

    // Create stop_title
    stop_title_layer = text_layer_create((GRect) {
        .origin = { 0, 60 },
        .size = { bounds.size.w, 40 }
    });
    text_layer_set_font(stop_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(stop_title_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(stop_title_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_color(stop_title_layer, GColorWhite);
    text_layer_set_background_color(stop_title_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(stop_title_layer));

    // Create stop_eta
    stop_prediction_layer = text_layer_create((GRect) {
        .origin = { 0, 100 },
        .size = { bounds.size.w, bounds.size.h }
    });
    text_layer_set_font(stop_prediction_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(stop_prediction_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(stop_prediction_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_color(stop_prediction_layer, GColorBlack);
    text_layer_set_background_color(stop_prediction_layer, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(stop_prediction_layer));

    // Create "minutes" text with small font
    minutes_text_layer = text_layer_create((GRect) {
        .origin = { 0, 130 },
        .size = { bounds.size.w, bounds.size.h }
    });
    text_layer_set_font(minutes_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(minutes_text_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(minutes_text_layer, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_color(minutes_text_layer, GColorBlack);
    text_layer_set_background_color(minutes_text_layer, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(minutes_text_layer));
}

/*
 * Called when the window resumes after already being loaded.
 */
static void stop_window_appear(Window *window) {
    stop_window_set_text("test", "test", "test", (int[2]){ 1, 2 });
}

/*
 * Called when the window leaves the screen.
 */
static void stop_window_disappear(Window *window) {

}

/*
 * Called when the window is removed from the window stack.
 * Destroy any layers associated with the window.
 */
static void stop_window_unload(Window *window) {
    text_layer_destroy(route_title_layer);
    text_layer_destroy(direction_title_layer);
    text_layer_destroy(stop_title_layer);
    text_layer_destroy(stop_prediction_layer);
    text_layer_destroy(minutes_text_layer);
    free(prediction_text);
}

/*
 * Create the stop window and set handlers
 */
static void init_stop_window() {
    stop_window = window_create();
    // window_set_click_config_provider(stop_window, click_config_provider);
    window_set_window_handlers(stop_window, (WindowHandlers) {
        .load = stop_window_load,
        .appear = stop_window_appear,
        .disappear = stop_window_disappear,
        .unload = stop_window_unload,
    });
}

/**********************************************************
 ** MAIN
 **********************************************************/

/*
 * Called when stops have been loaded from android
 */
static void on_stops_loaded(StopList *loaded_stop_list) {
    stop_list = loaded_stop_list;

    // Done loading; push menu window with loaded stops
    on_splash = false;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Pushing menu to screen");
    window_stack_push(menu_window, true /* animated */);
    window_stack_remove(splash_window, false /* animated */);
}

/*
 * Show splash with logo and load a menu of stops from android
 */
static void init(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Initializing FTW");

    // Starting on splash screen
    on_splash = true;

    // Show the splash screen
    init_splash_window();
    window_stack_push(splash_window, true /* animated */);

    // Create menu window and set window handlers
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating menu");
    init_menu_window();

    // Create stop window and set window handlers
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating stop window");
    init_stop_window();

    // Initialize sync and request stop data from android
    init_sync();
    sync_get_stops(on_stops_loaded);
}

static void deinit(void) {
    window_destroy(splash_window);
    window_destroy(menu_window);
    window_destroy(stop_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
