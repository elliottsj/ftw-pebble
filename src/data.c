#include "data.h"

void stop_list_destroy(StopList *stop_list) {
    if (stop_list == NULL)
        return;

    for (int i = 0; i < stop_list->section_count; i++) {
        StopSection *section = stop_list->sections[i];
        for (int j = 0; j < section->stop_count; j++) {
            Stop *stop = section->stops[j];
            free(stop->route_tag);
            free(stop->route_title);
            free(stop->direction_tag);
            free(stop->direction_title);
            free(stop->prediction);
            free(stop->minutes_label);
            free(stop);
        }
        free(section->stops);
        free(section->stop_tag);
        free(section->stop_title);
        free(section);
    }
    free(stop_list->sections);
    free(stop_list);
}

StopList *stop_list_create(uint16_t section_count) {
    StopList *stop_list = malloc(sizeof(StopList));
    stop_list->section_count = section_count;
    stop_list->sections = malloc(section_count * sizeof(StopSection *));
    return stop_list;
}

StopSection *stop_list_add_section(StopList *stop_list, uint16_t section_index, char *stop_tag, char *stop_title, uint16_t stop_count) {
    StopSection *section = stop_list->sections[section_index] = malloc(sizeof(StopSection));
    section->stop_tag = malloc(strlen(stop_tag) + 1);
    strcpy(section->stop_tag, stop_tag);
    section->stop_title = malloc(strlen(stop_title) + 1);
    strcpy(section->stop_title, stop_title);
    section->stop_count = stop_count;
    section->stops = malloc(stop_count * sizeof(Stop *));
    return section;
}

Stop *section_add_stop(StopSection *section, uint16_t stop_index, char *route_tag, char *route_title, char *direction_tag, char *direction_title) {
    Stop *stop = section->stops[stop_index] = malloc(sizeof(Stop));
    stop->route_tag = malloc(strlen(route_tag) + 1);
    strcpy(stop->route_tag, route_tag);
    stop->route_title = malloc(strlen(route_title) + 1);
    strcpy(stop->route_title, route_title);
    stop->direction_tag = malloc(strlen(direction_tag) + 1);
    strcpy(stop->direction_tag, direction_tag);
    stop->direction_title = malloc(strlen(direction_title) + 1);
    strcpy(stop->direction_title, direction_title);
    return stop;
}

Stop *stop_set_prediction(Stop *stop, char *prediction, char *minutes_label) {
    stop->prediction = malloc(strlen(prediction) + 1);
    strcpy(stop->prediction, prediction);
    stop->prediction = prediction;
    stop->minutes_label = malloc(strlen(minutes_label) + 1);
    strcpy(stop->minutes_label, minutes_label);
    return stop;
}

void dump_stop_list(StopList *stop_list) {
    if (stop_list == NULL)
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Can't dump NULL stop_list");

    APP_LOG(APP_LOG_LEVEL_DEBUG, "====== Dumping stop_list ======");

    for (int i = 0; i < stop_list->section_count; i++) {
        StopSection *section = stop_list->sections[i];
        APP_LOG(APP_LOG_LEVEL_DEBUG, "sections[%d]: {stop_tag: \"%s\", stop_title: \"%s\"}",
                i, section->stop_tag, section->stop_title);
        for (int j = 0; j < section->stop_count; j++) {
            Stop *stop = section->stops[j];
            APP_LOG(APP_LOG_LEVEL_DEBUG, "stops[%d]: {route_tag: \"%s\", route_title: \"%s\",",
                    j, stop->route_tag, stop->route_title);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "direction_tag: \"%s\", direction_title: \"%s\"}",
                    stop->direction_tag, stop->direction_title);
        }
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "======== End stop_list ========");
}