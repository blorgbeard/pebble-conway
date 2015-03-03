#include <pebble.h>
#include "life.h"
  
Window *window;
Layer *screen;

static void draw_time(char *timestring, GContext *ctx) {  
  graphics_context_set_text_color(ctx, GColorWhite);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
    GRect(0, 53, 144, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
    GRect(0, 55, 144, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
    GRect(0, 54, 142, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
    GRect(2, 54, 142, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
    GRect(0, 54, 144, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void screen_update(Layer *layer, GContext *ctx) {
  
  // assume this layer is full-screen
  
  GBitmap *bmp = graphics_capture_frame_buffer(ctx);
  
  live_life(bmp);
  
  graphics_release_frame_buffer(ctx, bmp);
  
  // debug; don't draw time
  //return;
  
  // draw the time over the top    
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  static char timestring[] = "00:00\0";

  if(clock_is_24h_style()) {
    strftime(timestring, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(timestring, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  // todo: render to a separate layer most of the time?
  draw_time(timestring, ctx);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(screen);
}

static void init() {
  window = window_create();

  screen = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(screen, screen_update);
  
  layer_add_child(window_get_root_layer(window), screen);
  window_stack_push(window, true);
  
  init_life();
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  layer_destroy(screen);
  window_destroy(window);
  deinit_life();
}

int main() {
  init();
  app_event_loop();
  deinit();
}

