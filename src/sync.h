#pragma once

#include "data.h"

/*
 * Initialize event handlers and buffers for android sync.
 */
void init_sync();

/*
 * Sends a request to android for stop data.
 */
void sync_get_stops(void (*on_stops_loaded)(StopList *));

/*
 * Sends a request to android for prediction data for the given stop.
 */
void sync_get_prediction(char *route_tag, char *stop_tag, void (*on_prediction_loaded)(char *prediction, char *minutes_label));
