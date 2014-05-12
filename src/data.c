#include "data.h"

StopList *stop_list_create(uint16_t section_count) {
    StopList *stop_list = malloc(sizeof(StopList));
    stop_list->section_count = section_count;
    stop_list->next_index = 0;
    stop_list->sections = malloc(section_count * sizeof(StopSection *));
    return stop_list;
}

void stop_list_destroy(StopList *stop_list) {
    for (int i = 0; i < stop_list->section_count; i++) {
        StopSection *section = stop_list->sections[i];
        for (int j = 0; j < section->stop_count; j++) {
            Stop *stop = section->stops[j];
            free(stop->route_tag);
            free(stop->route_title);
            free(stop->direction_tag);
            free(stop->direction_title);
            free(stop->stop_tag);
            free(stop->stop_title);
            free(stop);
        }
        free(section->stops);
        free(section->section_title);
        free(section);
    }
    free(stop_list->sections);
    free(stop_list);
}

StopSection *stop_list_add_section(StopList *stop_list, uint16_t stop_count, char *section_title) {
    StopSection *section = stop_list->sections[stop_list->next_index++] = malloc(sizeof(StopSection));
    section->stop_count = stop_count;
    section->section_title = section_title;
    section->next_index = 0;
    section->stops = malloc(stop_count * sizeof(Stop *));
    return section;
}

Stop *section_add_stop(StopSection *section,
                      char *route_tag,
                      char *route_title,
                      char *direction_tag,
                      char *direction_title,
                      char *stop_tag,
                      char *stop_title,
                      int prediction[2]) {
    Stop *stop = section->stops[section->next_index++] = malloc(sizeof(Stop));
    stop->route_tag = route_tag;
    stop->route_title = route_title;
    stop->direction_tag = direction_tag;
    stop->direction_title = direction_title;
    stop->stop_tag = stop_tag;
    stop->stop_title = stop_title;
    return stop;
}