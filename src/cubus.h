#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#include <pebble.h>
extern Window *window;
extern Window *stops_window;
extern TextLayer *load_layer;
//static TextLayer *eta_layer;
extern MenuLayer *main_menu_layer;
extern MenuLayer *stops_menu_layer;

extern char bus_out[10][50];
extern char eta_out[10][20];
extern int num_buses;

extern int num_stops;
extern char main_stop_name[50];
extern char stops_list[10][50]; 

void push_stops_menu(MenuLayer *menu_layer, MenuIndex * cell_index, void *callback_context);
void draw_stops_menu_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context);
void draw_stops_menu_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data);
uint16_t stops_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void *data);
int16_t stops_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* callback_context);
void outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void* context);
void change_stop(MenuLayer * menu_layer, MenuIndex* cell_index, void* callback_context);
void times_receiver(DictionaryIterator* iter, void* context);
#endif
