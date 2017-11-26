#ifndef _OP_MP_H_
#define _OP_MP_H_

#include "net_monome.h"
#include "types.h"

//--- op_step : monome grid as a complex cascade counter
typedef struct op_mp_struct {
  t_monome monome;
  // inputs: mode, focus, step
  s16 size;
  s16 dummy;

  // internal:
  s8 positions[8];
  s8 points[8];
  s8 points_save[8];
  u8 triggers[8];
  u8 trig_dests[8];
  u8 rules[8];
  u8 rule_dests[8];

  u8 edit_row;
  u8 key_count;
  u8 mode;
  u8 prev_mode;

  u8 XSIZE;

  t_outlet *outs[8];

} op_mp_t;

// init
void op_mp_init(void* op);
// de-init
void op_mp_deinit(void* op);

#endif // header guard
