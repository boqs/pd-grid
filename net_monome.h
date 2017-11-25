#ifndef _ALEPH_BEES_NET_MONOME_H_
#define _ALEPH_BEES_NET_MONOME_H_

// common
#include "types.h"

#define MONOME_MAX_LED_BYTES 256
extern u8 defaultLedBuffer[MONOME_MAX_LED_BYTES];
extern u8 *monomeLedBuffer;
/* extern bool monomeFrameDirty; */

// handler function type
typedef void(*monome_handler_t)(void* op, u8 x, u8 y, u8 z);

// abstract superclass for monome operators
// has event handler and focus flag
typedef struct _op_monome {
  // handler function, will connect to app event handler
  monome_handler_t handler;
  // focus flag
  u8 focus;
  u8 opLedBuffer[MONOME_MAX_LED_BYTES];
  // pointer to operator subclass
  void* op;
} op_monome_t;

//---------------------
//----- variables

extern op_monome_t* monomeOpFocus;

// device-connected flag
extern bool monomeConnect;

//------------------------
//----- functions

// initialize
void net_monome_init(op_monome_t *op_monome, void *op, monome_handler_t h);

// set/release focus
extern void net_monome_set_focus(op_monome_t* grid, u8 focus);

// set operator attributes from connected grid device
extern void net_monome_set_attributes( /* ...???... */void);

// clear LEDs on grid
extern void net_monome_grid_clear(void);

// connect
extern void net_monome_connect(void);
// disconnect
extern void net_monome_disconnect(void);

#endif // h guard
