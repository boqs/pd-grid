#include "net_monome.h"

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
