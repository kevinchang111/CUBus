#include <cubus.h>

Window *stops_window;
MenuLayer *stops_menu_layer;
int num_stops = 0;
char main_stop_name[50];
char stops_list[10][50]; 

void push_stops_menu(MenuLayer *menu_layer, MenuIndex * cell_index, void *callback_context){
	window_stack_push(stops_window, true);
	menu_layer_reload_data(stops_menu_layer);

}
void draw_stops_menu_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data){
	
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, "NEARBY STOPS", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), layer_get_bounds(cell_layer), GTextOverflowModeWordWrap , GTextAlignmentCenter, NULL);
}
 
uint16_t stops_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void *data){
	if(num_stops == 0) return 1;
	return num_stops;  
}

void draw_stops_menu_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context){
	if(num_stops == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "NO STOPS NEARBY", NULL,NULL);
		return; 
	}
	menu_cell_basic_draw(ctx, cell_layer, stops_list[cell_index->row], NULL,NULL); 
}

int16_t stops_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* callback_context){
	return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
}

void change_stop(MenuLayer * menu_layer, MenuIndex* cell_index, void* callback_context){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "changeStop"); 
	app_message_register_inbox_received(times_receiver);
	window_stack_remove(stops_window, true);
	layer_set_hidden(text_layer_get_layer(load_layer), false);
	layer_set_hidden(menu_layer_get_layer(main_menu_layer), true);
	text_layer_set_text(load_layer, "Refreshing...");
	DictionaryIterator* it;
	app_message_outbox_begin(&it);
	if(!it) return;
	dict_write_cstring(it, 1, "changeStop");
	dict_write_uint8(it, 2, cell_index->row);
	dict_write_end(it);
	AppMessageResult r = app_message_outbox_send();	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "done changeStop"); 
}
