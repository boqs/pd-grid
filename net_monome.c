#include "net_monome.h"
#include "lo/lo.h"

u8 defaultLedBuffer[MONOME_MAX_LED_BYTES];
u8 *monomeLedBuffer = defaultLedBuffer;
op_monome_t* monomeOpFocus = NULL;
void net_monome_init(op_monome_t *op_monome, void *op, monome_handler_t h) {
  op_monome->handler = h;
  op_monome->op = op;
  int i;
  for(i=0; i<MONOME_MAX_LED_BYTES; ++i) {
    op_monome->opLedBuffer[i] = 0;
  }
}

void net_monome_grid_clear(void) {
  int i;
  for(i=0; i<MONOME_MAX_LED_BYTES; ++i) {
    monomeLedBuffer[i] = 0;
  }
}
u8 monome_size_x () {
  // FIXME for now we only are compatible with 128 grid
  return 16;
}
u8 monome_size_y () {
  // FIXME for now we only are compatible with 128 grid
  return 8;
}
u8 monome_xy_idx(u8 x, u8 y) {
  return 16 * y + x;
}

void serial_osc_grab_focus(void) {
  lo_address a = lo_address_new(NULL, "13188");
  lo_send(a, "/sys/port", "i", 6001);
}

