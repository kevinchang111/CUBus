#include "cubus.h"
Window *window;
MenuLayer *main_menu_layer;
TextLayer *load_layer;
int num_buses = 0;
bool int_connection = true;
bool bt_connection = false;
char bus_out[10][50];
char eta_out[10][20];
bool first_load = true;
int auto_refresh_count = 0;  //keep track of number of autorefresh
static void stops_receiver(DictionaryIterator* iter, void* context);
void bt_connection_handler(bool connected);
void times_receiver(DictionaryIterator* iter, void* context);
static void times_failed(AppMessageResult reason, void *context);
void outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void* context);
static void auto_refresh(void* data);
static void stops_receiver(DictionaryIterator* iter, void* context);
static void msg_receiver(DictionaryIterator* iter, void* context);
static void refresh_times();
static void refresh_stops();
static AppTimer* auto_ref_timer;
static void refresh_data(MenuLayer * menu_layer, MenuIndex* cell_index, void* callback_context);
//static void tick_timer_handler(struct tm* tick_time, TimeUnits units_changed);
void bt_connection_handler(bool connected){
	bt_connection = connected;
	if(connected){
		text_layer_set_text(load_layer, "CONNECTION FOUND");
		DictionaryIterator* it;
		app_message_outbox_begin(&it);
		if(!it){ APP_LOG(APP_LOG_LEVEL_DEBUG, "FAILED init IT");
			return;
		}
		dict_write_cstring(it, 1, "getLocation");
		dict_write_end(it);
		auto_refresh_count = 0;
		app_message_outbox_send();		
		auto_ref_timer = app_timer_register(20000, auto_refresh, NULL); 
	}
	else{
		app_timer_cancel(auto_ref_timer);
		text_layer_set_text(load_layer, "CHECK BLUETOOTH CONNECTION");
		int_connection = false;
	}
}
void times_receiver(DictionaryIterator* iter, void* context){	

	Tuple* headsign = NULL;
	num_buses = dict_find(iter, 1)->value->int32; 
	if(dict_find(iter,2)->value->int32 == 2){
		stops_receiver(iter,context);
		return;	
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "num buses: %d\n", num_buses);
	snprintf(main_stop_name, 50, "%s", dict_find(iter, 3)->value->cstring); 
	static char head_sign_received[50];
	static int eta_received;
	for(int i = 1; i <= num_buses; i++){
		(headsign = dict_find(iter, i*2+2));
		Tuple* eta = dict_find(iter, i*2+3); 
		if(headsign){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "headsign: %s\n", headsign->value->cstring);
			strcpy(head_sign_received, headsign->value->cstring );	
		}else{
			APP_LOG(APP_LOG_LEVEL_DEBUG, "no headsign %d / %d", i, num_buses);
		}
		if(eta){
			eta_received = eta->value->int32;
			APP_LOG(APP_LOG_LEVEL_DEBUG, "ETA GOT: %d", eta_received);
				
		}else{
			APP_LOG(APP_LOG_LEVEL_DEBUG, "no eta");
		}
		if(headsign&&eta){
				snprintf(bus_out[i-1], 50, "%s", head_sign_received);
				if(eta_received == 0) 	snprintf(eta_out[i-1], 20, "DUE");	
				else snprintf(eta_out[i-1], 20, "%d min", eta_received);	
		}
	}
	if(first_load){
		menu_layer_set_click_config_onto_window(main_menu_layer, window);
		first_load = false;
	}
	menu_layer_reload_data(main_menu_layer);
	layer_set_hidden(text_layer_get_layer(load_layer), true);
	layer_set_hidden(menu_layer_get_layer(main_menu_layer), false);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "TIMES RECEIVED COMPLETE");	
	//menu_layer_set_selected_index(main_menu_layer,MenuIndex(0,0) , MenuRowAlignCenter, false);
}
static void times_failed(AppMessageResult reason, void *context){
	APP_LOG(APP_LOG_LEVEL_DEBUG,"FAILED RECEIVE %d\n", (int)reason);
}
void outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void* context){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "FAILED OUTBOX %d\n", (int)reason);
}
static void stops_receiver(DictionaryIterator* iter, void* context){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "STOPS RECEIVED");	
	num_stops = dict_find(iter, 1)->value->int32; 
	APP_LOG(APP_LOG_LEVEL_DEBUG, "num stops: %d\n", num_stops);
	for(int i = 1; i <= num_stops; i++){
		Tuple* stop = dict_find(iter, i+2); 
		if(stop){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "RECEIEVED stop name: %s\n", stop->value->cstring);
			snprintf(stops_list[i-1], 50, "%s", stop->value->cstring);
		}else{
			APP_LOG(APP_LOG_LEVEL_DEBUG, "no stops %d / %d", i, num_stops);
			return;
		}
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "STOPS RECEIVED COMPLETE");	
	refresh_times();
}	
static void msg_receiver(DictionaryIterator* iter, void* context){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "MSG RECEIVED");	
	int connect = dict_find(iter, 1)->value->int32; 
	if(connect == -1) {
		int_connection = false;	
		APP_LOG(APP_LOG_LEVEL_DEBUG, "no connection");
		text_layer_set_text(load_layer, "No Internet Connection");
		if(first_load){
			menu_layer_set_click_config_onto_window(main_menu_layer, window);
			first_load = false;
		}
		return;
	}
	int_connection = true;
	int type = dict_find(iter, 2)->value->int32;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "RECEIVED TYPE %d", type);	
	if(type == 1) times_receiver(iter, context);
	else if(type == 2) stops_receiver(iter, context);
	else{APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: BAD MSG");}	
}
static void refresh_times(){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "begin REFRESH_TIMES");
	DictionaryIterator* it;
	int r = (int)app_message_outbox_begin(&it);
	dict_write_cstring(it, 1, "refreshTimes");
	if(!it) {APP_LOG(APP_LOG_LEVEL_DEBUG, "FAILED initialize iterator %d", r); 
		app_message_outbox_send();
		return;}
	dict_write_end(it);
	app_message_outbox_send();	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "refresh done");	
}
static void refresh_stops(){
		APP_LOG(APP_LOG_LEVEL_DEBUG, "ref_stops");
		DictionaryIterator* it;
		int r = (int)app_message_outbox_begin(&it);
		if(!it) {APP_LOG(APP_LOG_LEVEL_DEBUG, "REF_STOPS : FAILED initialize iterator %d", r); 
		app_message_outbox_send();
		return;}

		dict_write_cstring(it, 1, "getLocation");
		dict_write_end(it);
		app_message_outbox_send();	
}
static void refresh_data(MenuLayer * menu_layer, MenuIndex* cell_index, void* callback_context){
	layer_set_hidden(text_layer_get_layer(load_layer), false);
	layer_set_hidden(menu_layer_get_layer(main_menu_layer), true);
	text_layer_set_text(load_layer, "Refreshing...");
	if(!bt_connection) 
	{
		text_layer_set_text(load_layer, "CHECK BLUETOOTH CONNECTION");
		return;
	}
	if(!int_connection){
		APP_LOG(APP_LOG_LEVEL_DEBUG, "reconnecting");
		refresh_stops();
		return;
	}
	refresh_times();
	if(auto_refresh_count > 30){
		auto_ref_timer = app_timer_register(20000, auto_refresh, NULL); 
	}
	auto_refresh_count = 0;
}
static void auto_refresh(void* data){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "AUTOREFRESH");
	if(auto_refresh_count++ >= 30){//stop auto_refresh after 10min
		return;
	}
	if(!bt_connection){
		 return;
	}
	if(!int_connection) refresh_stops(); 
	else refresh_times();	
	auto_ref_timer = app_timer_register(20000, auto_refresh, NULL); 
}
/*static void tick_timer_handler(struct tm* tick_time, TimeUnits units_changed){
	refresh_times();
	if(++auto_refresh_count >= 10){
		tick_timer_service_unsubscribe();
	}
}*/
static void draw_main_menu_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data){
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, main_stop_name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), layer_get_bounds(cell_layer), GTextOverflowModeFill , GTextAlignmentCenter, NULL);
} 
static uint16_t main_menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void *data){
	APP_LOG(APP_LOG_LEVEL_DEBUG, "menu_get_rows : %d\n", num_buses);
	if(num_buses == 0) return 1;
	return num_buses;  

}
static void draw_main_menu_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context){
	if(num_buses == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "NO BUSES", NULL,NULL);
		return; 
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bus_out[%d] : %s\n", cell_index->row, bus_out[cell_index->row]);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "eta_out[%d] : %s\n", cell_index->row, eta_out[cell_index->row]);
	menu_cell_basic_draw(ctx, cell_layer, bus_out[cell_index->row], eta_out[cell_index->row],NULL); 
}
static int16_t main_menu_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* callback_context){
	return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
}
static void window_load(Window *window) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "load window"); 
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	main_menu_layer = menu_layer_create(bounds);
	menu_layer_set_callbacks(main_menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_rows = main_menu_get_num_rows_callback,
		.draw_row = draw_main_menu_row_callback,
		.get_header_height = main_menu_header_height_callback,
		.draw_header = draw_main_menu_header_callback,
		.select_click = refresh_data,
		.select_long_click = push_stops_menu
	});
	layer_add_child(window_layer, menu_layer_get_layer(main_menu_layer));
	layer_set_hidden(menu_layer_get_layer(main_menu_layer), true);
	menu_layer_set_highlight_colors(main_menu_layer, GColorFromRGB(150,113,200), GColorFromRGB(255,255,255));  
	
	load_layer = text_layer_create(GRect(0, 62, bounds.size.w, 30));
 	text_layer_set_text_alignment(load_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(load_layer));
	text_layer_set_text(load_layer, "Loading...");	
	bt_connection = connection_service_peek_pebble_app_connection();	
	if(!bt_connection) 
	text_layer_set_text(load_layer, "CHECK BLUETOOTH CONNECTION");	
	auto_ref_timer = app_timer_register(20000, auto_refresh, NULL);//START AUTO_REFRESH 
}
static void stops_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(stops_window);
  GRect bounds = layer_get_bounds(window_layer);
	stops_menu_layer = menu_layer_create(bounds);
	menu_layer_set_callbacks(stops_menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_rows = stops_get_num_rows_callback,
		.draw_row = draw_stops_menu_row_callback,
		.get_header_height = stops_header_height_callback,
		.draw_header = draw_stops_menu_header_callback,
		.select_click = change_stop,
	});	
	menu_layer_set_click_config_onto_window(stops_menu_layer, stops_window);
	layer_add_child(window_layer, menu_layer_get_layer(stops_menu_layer));
	menu_layer_set_highlight_colors(stops_menu_layer, GColorFromRGB(100,30,255), GColorFromRGB(255,255,255));  	
}
static void window_unload(Window *window) {
  text_layer_destroy(load_layer);
	menu_layer_destroy(main_menu_layer);
	
}
static void stops_window_unload(Window *window){
	menu_layer_destroy(stops_menu_layer);
	
}
static void init(void) {
//register handlers
	connection_service_subscribe((ConnectionHandlers){
		.pebble_app_connection_handler = bt_connection_handler
	});
	app_message_register_inbox_dropped(times_failed);
	app_message_register_inbox_received(msg_receiver);
	app_message_register_outbox_failed(outbox_failed);	
	//tick_timer_service_subscribe(MINUTE_UNIT, tick_timer_handler);
//create windows
  stops_window = window_create(); 
  window_set_window_handlers(stops_window, (WindowHandlers) {
    .load = stops_window_load,
    .unload = stops_window_unload,
  });
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
	window_stack_push(window, true); // push window
	app_message_open(1000,100); // start appmsg
}

static void deinit(void) {
 	 window_destroy(window);
	window_destroy(stops_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
