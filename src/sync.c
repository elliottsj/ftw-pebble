#include <pebble.h>
#include "sync.h"
#include "data.h"

/* Message fields */
enum {
    // Message type
    MESSAGE_TYPE = 0,           // uint8_t
    // Sections
    SECTION_INDEX = 1,          // uint16_t
    SECTION_COUNT = 2,          // uint16_t
    SECTION_STOP_TAG = 3,       // char *
    SECTION_STOP_TITLE = 4,     // char *
    SECTION_STOP_COUNT = 5,     // uint16_t
    // Stops
    SECTION_STOP_INDEX = 6,     // uint16_t
    STOP_ROUTE_TAG = 7,         // char *
    STOP_ROUTE_TITLE = 8,       // char *
    STOP_DIRECTION_TAG = 9,     // char *
    STOP_DIRECTION_TITLE = 10,  // char *
    STOP_PREDICTION = 11,       // char *
    STOP_MINUTES_LABEL = 12     // char *
};

/* Possible message types */
enum {
    MESSAGE_REQUEST_SECTIONS_METADATA = 0,
    MESSAGE_REQUEST_SECTION_DATA = 1,
    MESSAGE_REQUEST_STOP_DATA = 2,
    MESSAGE_REQUEST_STOP_PREDICTION = 3,
    MESSAGE_SECTIONS_METADATA = 4,
    MESSAGE_SECTION_DATA = 5,
    MESSAGE_STOP_DATA = 6,
    MESSAGE_STOP_PREDICTION = 7
};

// A list of stops by section
static StopList *stop_list = NULL;

// Callback for when the stop list is fully loaded
static void (*stops_loaded_callback)(StopList *) = NULL;

// Callback for when a stop prediction has been loaded
static void (*stop_prediction_loaded_callback)(char *prediction, char *minutes_label) = NULL;

/**********************************************************
 ** UTILITIES
 **********************************************************/

/*
 * Return the name of the given message type
 */
static char *translate_message_type(unsigned char message_type) {
    switch (message_type) {
        case MESSAGE_REQUEST_SECTIONS_METADATA:
            return "MESSAGE_REQUEST_SECTIONS_METADATA";
        case MESSAGE_REQUEST_SECTION_DATA:
            return "MESSAGE_REQUEST_SECTION_DATA";
        case MESSAGE_REQUEST_STOP_DATA:
            return "MESSAGE_REQUEST_STOP_DATA";
        case MESSAGE_REQUEST_STOP_PREDICTION:
            return "MESSAGE_REQUEST_STOP_PREDICTION";
        case MESSAGE_SECTIONS_METADATA:
            return "MESSAGE_SECTIONS_METADATA";
        case MESSAGE_SECTION_DATA:
            return "MESSAGE_SECTION_DATA";
        case MESSAGE_STOP_DATA:
            return "MESSAGE_STOP_DATA";
        case MESSAGE_STOP_PREDICTION:
            return "MESSAGE_STOP_PREDICTION";
        default:
            return "MESSAGE_UNKNOWN";
    }
}

/*
 * Return the name of the given message field
 */
static char *translate_message_field(int message_field) {
    switch (message_field) {
        case MESSAGE_TYPE:
            return "MESSAGE_TYPE";
        case SECTION_INDEX:
            return "SECTION_INDEX";
        case SECTION_COUNT:
            return "SECTION_COUNT";
        case SECTION_STOP_TITLE:
            return "SECTION_STOP_TITLE";
        case SECTION_STOP_COUNT:
            return "SECTION_STOP_COUNT";
        case SECTION_STOP_INDEX:
            return "SECTION_STOP_INDEX";
        default:
            return "UNKNOWN_FIELD";
    }
}

/*
 * Return the name of the given AppMessageResult
 */
static char *translate_error(AppMessageResult result) {
    switch (result) {
        case APP_MSG_OK:
            return "APP_MSG_OK";
        case APP_MSG_SEND_TIMEOUT:
            return "APP_MSG_SEND_TIMEOUT";
        case APP_MSG_SEND_REJECTED:
            return "APP_MSG_SEND_REJECTED";
        case APP_MSG_NOT_CONNECTED:
            return "APP_MSG_NOT_CONNECTED";
        case APP_MSG_APP_NOT_RUNNING:
            return "APP_MSG_APP_NOT_RUNNING";
        case APP_MSG_INVALID_ARGS:
            return "APP_MSG_INVALID_ARGS";
        case APP_MSG_BUSY:
            return "APP_MSG_BUSY";
        case APP_MSG_BUFFER_OVERFLOW:
            return "APP_MSG_BUFFER_OVERFLOW";
        case APP_MSG_ALREADY_RELEASED:
            return "APP_MSG_ALREADY_RELEASED";
        case APP_MSG_CALLBACK_ALREADY_REGISTERED:
            return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
        case APP_MSG_CALLBACK_NOT_REGISTERED:
            return "APP_MSG_CALLBACK_NOT_REGISTERED";
        case APP_MSG_OUT_OF_MEMORY:
            return "APP_MSG_OUT_OF_MEMORY";
        case APP_MSG_CLOSED:
            return "APP_MSG_CLOSED";
        case APP_MSG_INTERNAL_ERROR:
            return "APP_MSG_INTERNAL_ERROR";
        default:
            return "UNKNOWN ERROR";
    }
}

/*
 * Same as dict_find: Tries to find a Tuple with specified key in a dictionary.
 *
 * Logs a message via APP_LOG if the given key could not be found
 */
static Tuple *dict_find_log(DictionaryIterator *iter, uint32_t key) {
    Tuple *tuple = dict_find(iter, key);
    if (tuple == NULL)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "%s not found in message", translate_message_field(key));
    return tuple;
}

/**********************************************************
 ** OUTBOUND MESSAGING
 **********************************************************/

/*
 * Send a request to android for section metadata
 */
static void request_section_metadata(void) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet message_type_tuplet = TupletInteger(MESSAGE_TYPE, MESSAGE_REQUEST_SECTIONS_METADATA);
    dict_write_tuplet(iter, &message_type_tuplet);
    app_message_outbox_send();
}

/*
 * Send a request to android for the section with the given index
 */
static void request_section(int section_index) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting section with section_index == %d", section_index);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet message_type_tuplet = TupletInteger(MESSAGE_TYPE, MESSAGE_REQUEST_SECTION_DATA);
    dict_write_tuplet(iter, &message_type_tuplet);
    Tuplet section_index_tuplet = TupletInteger(SECTION_INDEX, section_index);
    dict_write_tuplet(iter, &section_index_tuplet);
    app_message_outbox_send();
}

/*
 * Send a request to android for the stop with the given section and stop index
 */
static void request_stop(int section_index, int stop_index) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting stop with section_index == %d, stop_index == %d", section_index, stop_index);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet message_type_tuplet = TupletInteger(MESSAGE_TYPE, MESSAGE_REQUEST_STOP_DATA);
    dict_write_tuplet(iter, &message_type_tuplet);
    Tuplet section_index_tuplet = TupletInteger(SECTION_INDEX, section_index);
    dict_write_tuplet(iter, &section_index_tuplet);
    Tuplet stop_index_tuplet = TupletInteger(SECTION_STOP_INDEX, stop_index);
    dict_write_tuplet(iter, &stop_index_tuplet);
    app_message_outbox_send();
}

/*
 * Send a request to android for prediction data for the given stop
 * (identified by section index and stop index within the section)
 */
static void request_prediction(char *route_tag, char *stop_tag) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting prediction with route_tag == %s, stop_tag == %s",
        route_tag, stop_tag);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    Tuplet message_type_tuplet = TupletInteger(MESSAGE_TYPE, MESSAGE_REQUEST_STOP_PREDICTION);
    dict_write_tuplet(iter, &message_type_tuplet);
    Tuplet route_tag_tuplet = TupletCString(STOP_ROUTE_TAG, route_tag);
    dict_write_tuplet(iter, &route_tag_tuplet);
    Tuplet stop_tag_tuplet = TupletCString(SECTION_STOP_TAG, stop_tag);
    dict_write_tuplet(iter, &stop_tag_tuplet);
    app_message_outbox_send();
}

/**********************************************************
 ** INBOUND MESSAGE HANDLERS
 **********************************************************/

/*
 * Upon receiving sections metadata, create a new stop_list and begin sync by requesting the first section data.
 */
static void on_receive_section_metadata(DictionaryIterator *data) {
    // Receiving sections metadata; begin to sync a new stop list
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received a MESSAGE_SECTIONS_METADATA. Syncing...");

    // Delete existing data
    stop_list_destroy(stop_list);

    Tuple *tuple;

    // Get the number of sections we're receiving
    if ((tuple = dict_find_log(data, SECTION_COUNT)) == NULL) return;
    uint16_t section_count = tuple->value->uint16;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found section_count == %d", section_count);

    // Create a new stop_list
    stop_list = stop_list_create(section_count);

    // Request first section data
    if (section_count > 0)
        request_section(0);
}

/*
 * Upon receiving section data, update stop_list with the new data.
 */
static void on_receive_section_data(DictionaryIterator *data) {
    // Received section data
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received a MESSAGE_SECTION_DATA");

    Tuple *tuple;

    // Get the index of the section we're receiving
    if ((tuple = dict_find(data, SECTION_INDEX)) == NULL) return;
    uint16_t section_index = tuple->value->uint16;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found section_index == %d", section_index);

    // Get the stop tag of the section we're receiving
    if ((tuple = dict_find(data, SECTION_STOP_TAG)) == NULL) return;
    char *stop_tag = tuple->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found stop_tag == %s", stop_tag);

    // Get the stop title of the section we're receiving
    if ((tuple = dict_find(data, SECTION_STOP_TITLE)) == NULL) return;
    char *stop_title = tuple->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found stop_title == %s", stop_title);

    // Get the stop count of the section we're receiving
    if ((tuple = dict_find(data, SECTION_STOP_COUNT)) == NULL) return;
    uint16_t stop_count = tuple->value->uint16;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found stop_count == %d", stop_count);

    // Update stop_list with the new data
    stop_list_add_section(stop_list, section_index, stop_tag, stop_title, stop_count);

    // Request first stop data
    if (stop_count > 0)
        request_stop(section_index, 0);
}

/*
 * Upon receiving stop data, update stop_list with the new data.
 * If this is the last stop, call the stops_loaded_callback callback.
 */
static void on_receive_stop_data(DictionaryIterator *data) {
    // Received section data
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received a MESSAGE_STOP_DATA");

    Tuple *tuple;

    // Get the index of the section containing the stop we're receiving
    if ((tuple = dict_find(data, SECTION_INDEX)) == NULL) return;
    uint16_t section_index = tuple->value->uint16;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found section_index == %d", section_index);

    // Get the index of the stop we're receiving
    if ((tuple = dict_find(data, SECTION_STOP_INDEX)) == NULL) return;
    uint16_t stop_index = tuple->value->uint16;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found stop_index == %d", stop_index);

    // Get the route tag of the stop we're receiving
    if ((tuple = dict_find(data, STOP_ROUTE_TAG)) == NULL) return;
    char *route_tag = tuple->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found route_tag == %s", route_tag);

    // Get the route title of the stop we're receiving
    if ((tuple = dict_find(data, STOP_ROUTE_TITLE)) == NULL) return;
    char *route_title = tuple->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found route_title == %s", route_title);

    // Get the direction tag of the stop we're receiving
    if ((tuple = dict_find(data, STOP_DIRECTION_TAG)) == NULL) return;
    char *direction_tag = tuple->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found direction_tag == %s", direction_tag);    

    // Get the direction title of the stop we're receiving
    if ((tuple = dict_find(data, STOP_DIRECTION_TITLE)) == NULL) return;
    char *direction_title = tuple->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found direction_title == %s", direction_title);

    // Update stop_list with the new data
    section_add_stop(stop_list->sections[section_index], stop_index, route_tag, route_title, direction_tag, direction_title);

    // Request next data
    if (stop_index + 1 < stop_list->sections[section_index]->stop_count) {
        // More stops remain; request the next stop
        request_stop(section_index, stop_index + 1);
    } else if (section_index + 1 < stop_list->section_count) {
        // More sections remain; request the next section
        request_section(section_index + 1);
    } else {
        // No more stops remain; call the callback function
        stops_loaded_callback(stop_list);
    }
}

/*
 * Upon receiving stop prediction data, update stop_list with the new data and call
 */
static void on_receive_stop_prediction(DictionaryIterator *data) {
    // Received section data
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received a MESSAGE_STOP_PREDICTION");

    Tuple *tuple;

    // Get the prediction string of the stop we're receiving
    if ((tuple = dict_find(data, STOP_PREDICTION)) == NULL) return;
    char *prediction = malloc(tuple->length);
    strcpy(prediction, tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found prediction == %s", prediction);

    // Get the minutes label the stop we're receiving
    if ((tuple = dict_find(data, STOP_MINUTES_LABEL)) == NULL) return;
    char *minutes_label = malloc(tuple->length);
    strcpy(minutes_label, tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Found minutes_label == %s", minutes_label);

    // Call the callback function
    stop_prediction_loaded_callback(prediction, minutes_label);
}

/**********************************************************
 ** APP MESSAGE HANDLERS
 **********************************************************/

static void on_in_message_received(DictionaryIterator *received, void *context) {
    // First, find the message type
    Tuple *message_type_tuple = dict_find(received, MESSAGE_TYPE);
    if (message_type_tuple == NULL) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Received a message without message type!");
        return;
    }
    unsigned char message_type = message_type_tuple->value->uint8;
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Received message with type %s", translate_message_type(message_type));

    // Determine what to do with the message
    switch(message_type) {
        case MESSAGE_SECTIONS_METADATA:
            on_receive_section_metadata(received);
            break;
        case MESSAGE_SECTION_DATA:
            on_receive_section_data(received);
            break;
        case MESSAGE_STOP_DATA:
            on_receive_stop_data(received);
            break;
        case MESSAGE_STOP_PREDICTION:
            on_receive_stop_prediction(received);
            break;
        default:
            APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown message type %d", message_type);
    }
}

static void on_in_message_dropped(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Incoming message was dropped: %s", translate_error(reason));
}

static void on_out_message_delivered(DictionaryIterator *sent, void *context) {
    // unsigned char message_type = dict_find(sent, MESSAGE_TYPE)->value->uint8;
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "Sent message with type %s", translate_message_type(message_type));
}

static void on_out_message_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    unsigned char message_type = dict_find(failed, MESSAGE_TYPE)->value->uint8;
    APP_LOG(APP_LOG_LEVEL_WARNING, "Failed to send message with type %s: %s", translate_message_type(message_type), translate_error(reason));
}

/**********************************************************
 ** PUBLIC FUNCTIONS
 **********************************************************/

/*
 * Initialize sync by registering AppMessage handlers
 */
void init_sync() {
    // Register AppMessage handlers + initialize
    app_message_register_inbox_received(on_in_message_received);
    app_message_register_inbox_dropped(on_in_message_dropped);
    app_message_register_outbox_sent(on_out_message_delivered);
    app_message_register_outbox_failed(on_out_message_failed);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void sync_get_stops(void (*on_stops_loaded)(StopList *)) {
    // Send initial message to notify the app has started and request stop data
    request_section_metadata();

    // Save callback function
    stops_loaded_callback = on_stops_loaded;
}

void sync_get_prediction(char *route_tag, char *stop_tag, void (*on_prediction_loaded)(char *prediction, char *minutes_label)) {
    // Request prediction data from android
    request_prediction(route_tag, stop_tag);

    // Save callback function
    stop_prediction_loaded_callback = on_prediction_loaded;
}