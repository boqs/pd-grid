#ifndef _ALEPH_BEES_NET_MONOME_H_
#define _ALEPH_BEES_NET_MONOME_H_

#include "types.h"

#define MONOME_MAX_LED_BYTES 256
extern u8 defaultLedBuffer[MONOME_MAX_LED_BYTES];
extern u8 *monomeLedBuffer;

typedef void(*monome_handler_t)(void* op, u8 x, u8 y, u8 z);

typedef struct _op_monome {
  monome_handler_t handler;
  u8 focus;
  u8 opLedBuffer[MONOME_MAX_LED_BYTES];
  // pointer to operator parent class
  void* op;
} op_monome_t;

extern op_monome_t* monomeOpFocus;

void net_monome_init(op_monome_t *op_monome, void *op, monome_handler_t h);

extern void net_monome_set_focus(op_monome_t* grid, u8 focus);

extern void net_monome_grid_clear(void);

#endif // h guard
