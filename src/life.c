#include <pebble.h>
#include "life.h"

#define screen_width_px (144)
#define screen_height_px (168)
  
#define screen_width_cells (screen_width_px / 2)
#define screen_height_cells (screen_height_px / 2)
  
#define life_state_buffer_size (screen_width_cells * screen_height_cells)
  
#define life_bitmap_size (life_bitmap_stride * screen_height)
  
#define cell_live (0b10000000)
#define cell_dead (0b00000000)
#define cell_neighbours (0b00001111)

// two state buffers so that we can refer to the previous generations' neighbour counts while generating the next.
static uint8_t *state[2];

void init_life() {
  
  for (int i=0;i<2;i++) {
    state[i] = malloc(life_state_buffer_size);
    if (state[i] == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to allocate life state %d", i);
    } 
    else {
      memset(state[i], 0, life_state_buffer_size);
    }
  }
  
  /* temporary until intializing with time is implemented: init with R-pentomino
  
  cells that are on:
     00000
     00110
     01100
     00100
     00000
  
  each bit above is encoded as a byte: the MSB is the actual bit-value;
  the other 7 bits are the live-neighbour count for the cell.
  
  */
  
  
  int pos = screen_width_cells / 2;
  int ix_row = screen_width_cells * (screen_height_cells / 2);
  int x = pos;
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Initializing buffer.");
  
  state[0][ix_row + x++] = cell_dead | 0; state[0][ix_row + x++] = cell_dead | 1; state[0][ix_row + x++] = cell_dead | 2; state[0][ix_row + x++] = cell_dead | 2; state[0][ix_row + x++] = cell_dead | 1;
  ix_row += screen_width_cells; x = pos;
  state[0][ix_row + x++] = cell_dead | 1; state[0][ix_row + x++] = cell_dead | 3; state[0][ix_row + x++] = cell_live | 3; state[0][ix_row + x++] = cell_live | 2; state[0][ix_row + x++] = cell_dead | 1;
  ix_row += screen_width_cells; x = pos;
  state[0][ix_row + x++] = cell_dead | 1; state[0][ix_row + x++] = cell_live | 3; state[0][ix_row + x++] = cell_live | 4; state[0][ix_row + x++] = cell_dead | 4; state[0][ix_row + x++] = cell_dead | 1;
  ix_row += screen_width_cells; x = pos;
  state[0][ix_row + x++] = cell_dead | 1; state[0][ix_row + x++] = cell_dead | 3; state[0][ix_row + x++] = cell_live | 2; state[0][ix_row + x++] = cell_dead | 2; state[0][ix_row + x++] = cell_dead | 0;
  ix_row += screen_width_cells; x = pos;
  state[0][ix_row + x++] = cell_dead | 0; state[0][ix_row + x++] = cell_dead | 1; state[0][ix_row + x++] = cell_dead | 1; state[0][ix_row + x++] = cell_dead | 1; state[0][ix_row + x++] = cell_dead | 0;
    
  APP_LOG(APP_LOG_LEVEL_INFO, "Initialized buffer.");
  
  memcpy(state[1], state[0], life_state_buffer_size);
  
}

static void make_cell_dead(uint8_t *state_pointer) {
  state_pointer[0] &= cell_neighbours;  // mask out state bit, leave only neighbour count
  // update neighbour counts of neighbours
  state_pointer[0 - screen_width_cells - 1] -= 1;
  state_pointer[0 - screen_width_cells + 0] -= 1;
  state_pointer[0 - screen_width_cells + 1] -= 1;
  state_pointer[0 - 1] -= 1;
  state_pointer[0 + 1] -= 1;
  state_pointer[0 + screen_width_cells - 1] -= 1;
  state_pointer[0 + screen_width_cells + 0] -= 1;
  state_pointer[0 + screen_width_cells + 1] -= 1;  
}

static void make_cell_alive(uint8_t *state_pointer) {
  state_pointer[0] |= cell_live;
  // update neighbour counts of neighbours
  state_pointer[0 - screen_width_cells - 1] += 1;
  state_pointer[0 - screen_width_cells + 0] += 1;
  state_pointer[0 - screen_width_cells + 1] += 1;
  state_pointer[0 - 1] += 1;
  state_pointer[0 + 1] += 1;
  state_pointer[0 + screen_width_cells - 1] += 1;
  state_pointer[0 + screen_width_cells + 0] += 1;
  state_pointer[0 + screen_width_cells + 1] += 1;
}

void seed_life(GBitmap *bitmap) {
  // turn on cells which are under set pixels
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Reseeding life.");
  
  uint8_t *state_pointer = state[1];
  uint8_t *bmp = (uint8_t *)bitmap->addr;
  uint8_t *bmp_pointer1 = bmp;
  uint8_t *bmp_pointer2 = bmp + bitmap->row_size_bytes;
  
  uint16_t stride_extra = (bitmap->row_size_bytes - (screen_width_px / 8));
  
  uint16_t count = 0;
  
  for (int row=0; row < screen_height_cells; row++) {
    for (int col=0; col < screen_width_cells; col += 4) {
      
      // we're processing 4 cells, which contains 2x2 pixels each
      // the cell should be turned on if it's currently off and at least 2 of the 4 pixels under it are on.
      
      // the most likely case is that the pixels are all off:
      if ((*bmp_pointer1 != 0) || (*bmp_pointer2 != 0)) {
        
        count++;
        
        // easy mode: let's just turn on all cells that are dead,
        // based on the fact that there's at least one pixel alive under one of them.
        for (int i = 0; i < 4; i++) {
          if ((state_pointer[i] & cell_live) == 0) {
            make_cell_alive(state_pointer + i);
          }
        }
      }
      
      bmp_pointer1++;
      bmp_pointer2++;
      state_pointer += 4;
    }
    // account for stride and skip a row (because we're doubling them)
    bmp_pointer1 += stride_extra + bitmap->row_size_bytes;
    bmp_pointer2 += stride_extra + bitmap->row_size_bytes;
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Chunks alived: %u", count);
  
  // copy to other buffer
  memcpy(state[0], state[1], life_state_buffer_size);
}

void live_life(GBitmap* output) {
  uint8_t *bmp = (uint8_t *)output->addr;
  // assume bitmap is screen_width * screen_height - todo: assert this
  
  // for now: just render state to bitmap.
  
  uint8_t *state_pointer1 = state[0];
  uint8_t *state_pointer2 = state[1];
  uint8_t *bmp_pointer1 = bmp;
  uint8_t *bmp_pointer2 = bmp + output->row_size_bytes;
  
  uint16_t stride_extra = (output->row_size_bytes - (screen_width_px / 8));
  
  time_t seconds1;
  uint16_t millis1 = time_ms(&seconds1, NULL);
  
  for (int row=0; row < screen_height_cells; row++) {
    for (int col=0; col < screen_width_cells; col += 4) {
      // generate the pixels for these cells
      uint8_t pixels =
        ((state_pointer1[0] & cell_live) >> 7 << 0) +
        ((state_pointer1[0] & cell_live) >> 7 << 1) +
        ((state_pointer1[1] & cell_live) >> 7 << 2) +
        ((state_pointer1[1] & cell_live) >> 7 << 3) +
        ((state_pointer1[2] & cell_live) >> 7 << 4) +
        ((state_pointer1[2] & cell_live) >> 7 << 5) +
        ((state_pointer1[3] & cell_live) >> 7 << 6) +
        ((state_pointer1[3] & cell_live) >> 7 << 7);

      bmp_pointer1[0] = pixels;
      bmp_pointer2[0] = pixels;

      // check if all 4 bytes we're currently processing are entirely zeros:
      // no live cells, all with zero neighbours
      if ((*(uint32_t *)state_pointer1) != 0) {
        // something was non-zero: perform life algorithm
        // for now: easy-mode - ignore cells that we might need to wrap for.
        if ((row != 0) && (col != 0) && (row != (screen_height_cells-1)) && (col != (screen_width_cells-1))) {
          for (int i = 0; i < 4; i++) {
            uint8_t neighbours = state_pointer1[i] & cell_neighbours;              
            if (neighbours > 8) {
              APP_LOG(APP_LOG_LEVEL_ERROR, "Erroneous neighbour count! %u", neighbours);
            }
            if ((state_pointer1[i] & cell_live) != 0) {
              // cell was live
              if ((neighbours < 2) || (neighbours > 3)) {
                // death by loneliness / overcrowding
                make_cell_dead(state_pointer2 + i);
              }
            }
            else {
              // cell was dead
              if (neighbours == 3) {
                // new cell born!                
                make_cell_alive(state_pointer2 + i);
              }
            }
          }     // damn that's a lot of closing braces, let's extract a method here, hm?
        }        
      }
      
      bmp_pointer1++;
      bmp_pointer2++;    
      state_pointer1 += 4;
      state_pointer2 += 4;
    }
    // account for stride and skip a row (because we're doubling them)
    bmp_pointer1 += stride_extra + output->row_size_bytes;
    bmp_pointer2 += stride_extra + output->row_size_bytes;
  }
  
  // copy altered buffer for next iteration
  memcpy(state[0], state[1], life_state_buffer_size);
  
  time_t seconds2;
  uint16_t millis2 = time_ms(&seconds2, NULL);
  
  seconds2 -= seconds1;
  seconds1 = 0;
  uint16_t render_time_ms = ((seconds2 * 1000) + millis2) - (millis1);
  if (render_time_ms > 15) {
    APP_LOG(APP_LOG_LEVEL_INFO, "render took a long time: %u ms", render_time_ms);
  }
  
}



void deinit_life() {
  free(state[0]);
  free(state[1]);
}
