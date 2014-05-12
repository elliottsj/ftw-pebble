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