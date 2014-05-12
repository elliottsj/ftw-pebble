#pragma once

#include <pebble.h>

typedef struct Stop {
    char *route_tag;
    char *route_title;
    char *direction_tag;
    char *direction_title;
    char *stop_tag;
    char *stop_title;
    int prediction[2];
} Stop;

typedef struct StopSection {
    uint16_t stop_count;
    // section_title == stop_title for each stop since all stops in a section have the same title
    char *section_title;
    // the index after the last stop in the section
    uint16_t next_index;
    Stop **stops;
} StopSection;

typedef struct StopList {
    uint16_t section_count;
    // the index after the last section in the list
    uint16_t next_index;
    StopSection **sections;
} StopList;

StopList *stop_list_create(uint16_t section_count);

void stop_list_destroy(StopList *stop_list);

StopSection *stop_list_add_section(StopList *stop_list, uint16_t stop_count, char *section_title);

Stop *section_add_stop(StopSection *section,
                       char *route_tag,
                       char *route_title,
                       char *direction_tag,
                       char *direction_title,
                       char *stop_tag,
                       char *stop_title,
                       int prediction[2]);