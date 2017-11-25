/* net_monome.c
   bees
   aleph

   monome d\river <-> operator glue layer

   keeps pointers to handler functions,
   manages operator focus
*/

/// assf

#include "net_monome.h"

// device-connected flag
//// FIXME: in future, multiple devices.
bool monomeConnect = 0;


// dummy handler
static void dummyHandler(void* op, u8 x, u8 y, u8 z) { return; }

//---------------------------------
// extern variables, initialized here.
u8 defaultLedBuffer[MONOME_MAX_LED_BYTES];
u8 *monomeLedBuffer = defaultLedBuffer;
/* extern bool monomeFrameDirty; */

// FIXME: a similar problem to avr32_lib/monome.c
// we are assuming there is only one connected device,
// but of course we would like to implement hub support
// and allow for multiple devices in the future.

// this would means that multiple operators would be mapped
// arbitrarily to different sources! oy...

op_monome_t* monomeOpFocus = NULL;






















/// TODO: tilt and key press should be decoupled from grid/ring??? yeah probly. 
/// specialized operators are not really appropriate,
//// have system create additional instances of adc/sw ?? hm
//monome_handler_t monome_grid_tilt_handler = NULL;
//monome_handler_t monome_ring_key_handler = NULL;

// set focus
void net_monome_set_focus(op_monome_t* op_monome, u8 focus) {
  if(focus > 0) {
    // aha... set_focus is getting called twice in a row on scene load
    /// (once on _init, once in _unpickle.)
    // we don't want the second call to *unset* focus!
    //    if(monomeOpFocus != NULL ) {
    if((monomeOpFocus != NULL) && (monomeOpFocus != op_monome)) {
      /// stealing focus, inform the previous holder
      monomeOpFocus->focus = 0;
    }
    monomeLedBuffer = op_monome->opLedBuffer;
    monomeOpFocus = op_monome;
    op_monome->focus = 1;
  } else {
    monomeLedBuffer = defaultLedBuffer;
    monomeOpFocus = NULL;
    op_monome->focus = 0;
  }
}

void net_monome_init(op_monome_t *op_monome, void *op, monome_handler_t h) {
  op_monome->handler = h;
  op_monome->op = op;
  int i;
  for(i=0; i<MONOME_MAX_LED_BYTES; ++i) {
    op_monome->opLedBuffer[i] = 0;
  }
}

// clear LEDs on grid
void net_monome_grid_clear(void) {
  int i;
  if(monomeConnect) {
    for(i=0; i<MONOME_MAX_LED_BYTES; ++i) {
      monomeLedBuffer[i] = 0;
    }
    /* monome_grid_refresh(); */
  }
}

// set operator attributes from connected grid device .. ??
void net_monome_set_attributes() {
  //... TODO... 
}

/* void net_monome_connect(void) { */
/*   if(monomeConnect != 1) { */
/*     monomeConnect = 1; */
/*     timers_set_monome(); */
/*   } else { */
/*     // already connected... oops? */
/*   } */
/* } */

/* // disconnect */
/* void net_monome_disconnect(void) { */
/*   monomeConnect = 0; */
/*   timers_unset_monome(); */
/* } */
