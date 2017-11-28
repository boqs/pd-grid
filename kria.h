#ifndef _OP_KRIA_H_
#define __OP_KRIA_H_

#include "net_monome.h"
#include "types.h"
#define OP_KRIA_POLL_TIME 50

typedef enum {
  mTr, mDur, mNote, mScale, mTrans, mScaleEdit, mPattern
} modes;

typedef enum {
  tTr, tAc, tOct, tDur, tNote, tTrans, tScale
} time_params;

typedef enum {
  modNone, modLoop, modTime
} mod_modes;

#define NUM_PARAMS 7

static const u8 SCALE[49];

typedef struct {
  u8 tr[16];
  u8 ac[16];
  u8 oct[16];
  u8 note[16];
  u8 dur[16];
  u8 sc[16];
  u8 trans[16];

  u8 dur_mul;

  u8 lstart[NUM_PARAMS];
  u8 lend[NUM_PARAMS];
  u8 llen[NUM_PARAMS];
  u8 lswap[NUM_PARAMS];
  time_params tmul[NUM_PARAMS];
  // time_params tdiv[NUM_PARAMS];
} kria_pattern;

// TO 96
typedef struct {
  kria_pattern kp[2][16];
  u8 pscale[7];
  u8 scales[42][7];
} kria_set;

//--- white whale
typedef struct op_kria_struct {
  t_monome monome;
  kria_set k;
  // inputs: mode, focus, step
  s16 clk;
  s16 octave;
  s16 tuning;
  // outputs: a,b,c,d
  t_outlet *voice0;
  t_outlet *voice1;

  t_clock *clock;
} op_kria_t;

// init
void op_kria_init(void* op);
// de-init
void op_kria_deinit(void* op);

#endif // header guard
