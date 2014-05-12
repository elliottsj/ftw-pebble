#include <pebble.h>
#include "sync.h"
#include "data.h"

void (*stops_loaded_callback)(StopList *) = NULL;

void init_sync() {
    // Register AppMessage handlers + initialize
    // app_message_register_inbox_received(in_received_handler);
    // app_message_register_inbox_dropped(in_dropped_handler);
    // app_message_register_outbox_sent(out_sent_handler);
    // app_message_register_outbox_failed(out_failed_handler);
    // app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void sync_get_stops(void (*on_stops_loaded)(StopList *)) {
    // Send initial message to notify the app has started
    // DictionaryIterator *iter;
    // app_message_outbox_begin(&iter);
    // Tuplet value = TupletInteger(MESSAGE_TYPE, 1);
    // dict_write_tuplet(iter, &value);
    // app_message_outbox_send();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating sample data");

    // temporary sample data:
    StopList *stop_list = stop_list_create(2 /* 2 sections */);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating section 1");    
    StopSection *section1 = stop_list_add_section(stop_list, 1 /* 1 stop */, "College St @ Yonge St (College Station)");

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating section 1, stop 1");
    section_add_stop(section1,
                     "506",
                     "506 Carlton",
                     "506_1_506Sun",
                     "East » Main Street Station",
                     "4865",
                     "College St @ Yonge St (College Station)",
                     (int[]){ 3, 15 });

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating section 2");
    StopSection *section2 = stop_list_add_section(stop_list, 2 /* 2 stops */, "Eglinton Ave East At Redpath Ave");

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating section 2, stop 1");
    section_add_stop(section2,
                     "54",
                     "54 Lawrence East",
                     "54_0_54",
                     "East » Orton Park",
                     "6665",
                     "Eglinton Ave East @ Redpath Ave",
                     (int[]){ 2, 4 });

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating section 2, stop 2");
    section_add_stop(section2,
                     "103",
                     "103 Mt Pleasant North",
                     "103_1_103",
                     "North » Doncliffe",
                     "6665",
                     "Eglinton Ave East @ Redpath Ave",
                     (int[]){ 1, 5 });

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done creating sample data");

    on_stops_loaded(stop_list);
}
