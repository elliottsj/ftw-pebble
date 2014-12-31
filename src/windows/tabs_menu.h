#pragma once

/*
 * Create the menu window and set handlers.
 * Return the created window.
 */
Window *init_tabs_menu_window(void (*on_nearby_selected)(void), void (*on_saved_selected)(void));
