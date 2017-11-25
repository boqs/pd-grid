#ifndef _OP_STEP_H_
#define _OP_STEP_H_

#include "m_pd.h"
#include "net_monome.h"
#include "types.h"

//--- op_step : monome grid as a step sequencer
typedef struct op_step_struct {
  t_object x_obj;
  op_monome_t monome;
  // inputs: size, step
  volatile io_t size, step;
  // outputs: a,b,c,d
  t_outlet *a;
  t_outlet *b;
  t_outlet *c;
  t_outlet *d;
  t_outlet *mono1;
  t_outlet *pos1;
  t_outlet *mono2;
  t_outlet *pos2;
  // internal:
  s8 s_start, s_end, s_length, s_now, s_cut;
  s8 s_start2, s_end2, s_length2, s_now2, s_cut2;
  u8 steps[4][16];
} op_step_t;

// init
void op_step_init(void* op);
// de-init
void op_step_deinit(void* op);

#endif // header guard
