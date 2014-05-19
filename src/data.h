#pragma once

#include <pebble.h>

typedef struct Stop {
    char *route_tag;
    char *route_title;
    char *direction_tag;
    char *direction_title;
    char *prediction;
    char *minutes_label;
} Stop;

typedef struct StopSection {
    // stop_tag and stop_title are equivalent for each stop in the section
    char *stop_tag;
    char *stop_title;
    uint16_t stop_count;
    Stop **stops;
} StopSection;

typedef struct StopList {
    uint16_t section_count;
    StopSection **sections;
} StopList;

void stop_list_destroy(StopList *stop_list);

StopList *stop_list_create(uint16_t section_count);

StopSection *stop_list_add_section(StopList *stop_list, uint16_t section_index, char *stop_tag, char *stop_title, uint16_t stop_count);

Stop *section_add_stop(StopSection *section, uint16_t stop_index, char *route_tag, char *route_title, char *direction_tag, char *direction_title);

Stop *stop_set_prediction(Stop *stop, char *prediction, char *minutes_label);

void dump_stop_list(StopList *stop_list);