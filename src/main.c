#include <pebble.h>
#include "life.h"
  
static Window *window;
static Layer *screen;
static char timestring[] = "00:00 AM";

static AppTimer *timer;

static void draw_time(char *timestring, GContext *ctx) {  
  graphics_context_set_text_color(ctx, GColorWhite);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
    GRect(0, 53, 144, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
    GRect(0, 55, 144, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
    GRect(0, 54, 142, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
    GRect(2, 54, 142, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  graphics_context_set_text_color(ctx, GColorBlack);
  
  graphics_draw_text(
    ctx, timestring, 
    fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK),
    GRect(0, 54, 144, 100),
    GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static bool reseed = true;

static void screen_update(Layer *layer, GContext *ctx) {
  
  // assume this layer is full-screen
    
  if (reseed) {
    APP_LOG(APP_LOG_LEVEL_INFO, "reseedery");
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
    draw_time(timestring, ctx);
  }
  
  GBitmap *bmp = graphics_capture_frame_buffer(ctx);
  
  if (reseed) {
    seed_life(bmp);
    //graphics_release_frame_buffer(ctx, bmp);  // temp
    //return;
    reseed = false;
  }
  
  live_life(bmp);
  
  graphics_release_frame_buffer(ctx, bmp);
  
  // debug; don't draw time
  //return;
  
  // draw the time over the top    
  
  
  // todo: render to a separate layer most of the time?
  draw_time(timestring, ctx);  
}

static void handle_timer(void *data) {
  timer = app_timer_register(33, handle_timer, NULL);
  layer_mark_dirty(screen);
}

static void update_time_string(struct tm *tick_time) {  
  clock_copy_time_string(timestring, sizeof("00:00 AM"));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {  
  update_time_string(tick_time);
  // seed life buffer with time
  reseed = true;
}

static void init() {
  window = window_create();

  screen = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(screen, screen_update);
  
  layer_add_child(window_get_root_layer(window), screen);
  window_stack_push(window, true);
  
  init_life();
  
  // initialize time string, then start updating it every minute
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
  update_time_string(t);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);  
  
  // start animating
  handle_timer(NULL);
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

