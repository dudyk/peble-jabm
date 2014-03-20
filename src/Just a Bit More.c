/*

  Just A Bit More watch.

  A binary clock with day of month.

  Changes from sample: David Kohen

  See:

    <http://en.wikipedia.org/wiki/Binary_clock>

 */
#include "pebble.h"

Window *window;

Layer *display_layer;


#define CIRCLE_RADIUS 12
#define CIRCLE_LINE_THICKNESS 2

void draw_cell(GContext* ctx, GPoint center, bool filled) {
  // Each "cell" represents a binary digit or 0 or 1.

  graphics_context_set_fill_color(ctx, GColorWhite);

  graphics_fill_circle(ctx, center, CIRCLE_RADIUS);

  if (!filled) {
    // This is our ghetto way of drawing circles with a line thickness
    // of more than a single pixel.
    graphics_context_set_fill_color(ctx, GColorBlack);

    graphics_fill_circle(ctx, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
  }

}


#define CIRCLE_PADDING 14 - CIRCLE_RADIUS // Number of padding pixels on each side
#define CELL_SIZE (2 * (CIRCLE_RADIUS + CIRCLE_PADDING)) // One "cell" is the square that contains the circle.
#define SIDE_PADDING (144 - (4 * CELL_SIZE))/2

#define CELLS_PER_ROW 4
#define CELLS_PER_COLUMN 6


GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
  // Cell location (0,0) is upper left, location (4, 6) is lower right.
  return GPoint(SIDE_PADDING + (CELL_SIZE/2) + (CELL_SIZE * x),
		(CELL_SIZE/2) + (CELL_SIZE * y));
}

void draw_cell_column(GContext* ctx, unsigned short number, unsigned short max_rows_to_display, unsigned short cell_column) {
    // This draws a number as a binary by extracting the binary digits from the number and calling the drawing
    // routine to draw the digits themselves.
    for (int cell_row_index = 0; cell_row_index < max_rows_to_display; cell_row_index++) {
        draw_cell(ctx, get_center_point_from_cell_location(cell_column, cell_row_index), (number >> cell_row_index) & 0x1);
    }
}


#define DEFAULT_MAX_ROWS 6
#define DATE_MAX_ROWS 5
#define HOURS_MAX_ROWS clock_is_24h_style()?5:4
#define MINUTES_MAX_ROWS 6
#define SECONDS_MAX_ROWS 6
#define DATE_COL 3
#define HOURS_COL 2
#define MINUTES_COL 1
#define SECONDS_COL 0

unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}


void display_layer_update_callback(Layer *me, GContext* ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned short display_hour = get_display_hour(t->tm_hour);

  draw_cell_column(ctx, t -> tm_mday, DATE_MAX_ROWS, DATE_COL);
  draw_cell_column(ctx, display_hour, HOURS_MAX_ROWS, HOURS_COL);
  draw_cell_column(ctx, t -> tm_min, MINUTES_MAX_ROWS, MINUTES_COL);
  draw_cell_column(ctx, t -> tm_sec, SECONDS_MAX_ROWS, SECONDS_COL);
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(display_layer);
}


static void do_init(void) {
  window = window_create();
  window_stack_push(window, true);

  window_set_background_color(window, GColorBlack);

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  // Init the layer for the display
  display_layer = layer_create(frame);
  layer_set_update_proc(display_layer, &display_layer_update_callback);
  layer_add_child(root_layer, display_layer);

  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
}

static void do_deinit(void) {
  layer_destroy(display_layer);
  window_destroy(window);
}


int main(void) {
  do_init();
  app_event_loop();
  do_deinit();
}
