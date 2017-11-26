#include "mp.h"

// LED intensity levels
#define L2 15
#define L1 9
#define L0 5



static void op_mp_in_step(op_mp_t* op, float f);
static void op_mp_in_size(op_mp_t* op, float f);

/// monome event handler
static void op_mp_handler(t_monome* op_monome, u8 x, u8 y, u8 z);

static void op_mp_redraw(t_monome* op_monome);
static void op_mp_trigger(op_mp_t* op_monome, u8 n);

static u32 rnd(void);



const u8 glyph[8][8] = {{0,0,0,0,0,0,0,0},         // o
       {0,24,24,126,126,24,24,0},     // +
       {0,0,0,126,126,0,0,0},       // -
       {0,96,96,126,126,96,96,0},     // >
       {0,6,6,126,126,6,6,0},       // <
       {0,102,102,24,24,102,102,0},   // * rnd
       {0,120,120,102,102,30,30,0},   // <> up/down
       {0,126,126,102,102,126,126,0}};  // [] return


u32 a1 = 0x19660d;
u32 c1 = 0x3c6ef35f;
u32 x1 = 1234567;  // seed
u32 a2 = 0x19660d;
u32 c2 = 0x3c6ef35f;
u32 x2 = 7654321;  // seed

void cascades_copy_init_u(u8 *dest, u8 *src);
void cascades_copy_init_s(s8 *dest, s8 *src);

void cascades_copy_init_u(u8 *dest, u8 *src) {
  int i;
  for(i=0; i<8; i++) {
    dest[i] = src[i];
  }
}

void cascades_copy_init_s(s8 *dest, s8 *src) {
  int i;
  for(i=0; i<8; i++) {
    dest[i] = src[i];
  }
}

static t_class *op_mp_class;
void *mp_new(t_symbol *s, int argc, t_atom *argv);
void mp_setup (void) {
  net_monome_setup();
  op_mp_class = class_new(gensym("mp"),
			  (t_newmethod)mp_new,
			  0, sizeof(op_mp_t),
			  CLASS_DEFAULT,
			  A_GIMME, 0);
  net_monome_add_focus_methods(op_mp_class);
  class_addbang(op_mp_class, (t_method)op_mp_in_step);
  class_addmethod(op_mp_class,
		  (t_method)op_mp_in_size, gensym("size"), A_DEFFLOAT, 0);
  /* class_sethelpsymbol(counter_class, gensym("help-counter"));   */
}

void *mp_new(t_symbol *s, int argc, t_atom *argv) {
  (void) s;
  (void) argc;
  (void) argv;
  op_mp_t *op = (op_mp_t *)pd_new(op_mp_class);

  net_monome_init(&op->monome, (monome_handler_t)&op_mp_handler);

  int i;
  for(i=0; i < 8; i++) {
    op->outs[i] = outlet_new((t_object *) op, &s_float);
  }

  op->key_count = 0;
  op->mode = 0;
  op->prev_mode = 0;

  op->edit_row = op->key_count = op->mode = op->prev_mode = 0;

  op->size = net_monome_size_x();
  op->dummy = 0;

  s8 positions_init[8] = {3,1,2,2,3,3,5,7};
  cascades_copy_init_s(op->positions, positions_init);
  s8 points_init[8] = {3,1,2,2,3,3,5,7};
  cascades_copy_init_s(op->points, points_init);
  s8 points_save_init[8] = {3,1,2,2,3,3,5,7};
  cascades_copy_init_s(op->points_save, points_save_init);
  u8 triggers_init[8] = {0,0,0,0,0,0,0,0};
  cascades_copy_init_u(op->triggers, triggers_init);
  u8 trig_dests_init[8] = {0,0,0,0,0,0,0,0};
  cascades_copy_init_u(op->trig_dests, trig_dests_init);
  u8 rules_init[8] = {0,0,0,0,0,0,0,0};
  cascades_copy_init_u(op->rules, rules_init);
  u8 rules_dests_init[8] = {0,1,2,3,4,5,6,7};
  cascades_copy_init_u(op->rule_dests, rules_dests_init);

  // init monome drawing
  op_mp_redraw(&(op->monome));
  return op;
}

// de-init
void op_mp_free(void* op) {
  // release focus
  net_monome_deinit((t_monome *) op);
}

//-------------------------------------------------
//----- static function definition

static void op_mp_in_size(op_mp_t* op, float f) {
  if(f < 9.0) op->size = 8;
  else op->size = 16;

  // FIXME: auto-detect size, this doesn't get set in unpickling
  op->XSIZE = op->size;
}

static void op_mp_in_step(op_mp_t* op, float f) {
  (void) f;
  u8 i;

  // clear last round
  for(i=0;i<8;i++)
    op->triggers[i] = 0;

  // main
  op_mp_trigger(op, 0);

  // ensure bounds, output triggers
  for(i=0;i<8;i++) {
    if(op->positions[i] < 0)
      op->positions[i] = 0;
    else if(op->positions[i] > op->points[i])
      op->positions[i] = op->points[i];

    // send out
    if(op->triggers[i]) {
      outlet_float(op->outs[i], 1.0);
    }
  }

  op_mp_redraw(&op->monome);
}

static void op_mp_trigger(op_mp_t *op, u8 n) {
  u8 m;

  op->positions[n]--;

  // ****** the trigger # check is so we don't cause a trigger/rules multiple times per NEXT
  // a rules-based jump to position-point does not current cause a trigger. should it?
  if(op->positions[n] < 0 && op->triggers[n] == 0) {
    op->triggers[n]++;
  
    if(op->rules[n] == 1) {     // inc
      if(op->points[op->rule_dests[n]] < (op->XSIZE-1)) {
        op->points[op->rule_dests[n]]++;
        op->positions[op->rule_dests[n]] = op->points[op->rule_dests[n]];
      }
    }
    else if(op->rules[n] == 2) {  // dec
      if(op->points[op->rule_dests[n]] > 0) {
        op->points[op->rule_dests[n]]--;
        op->positions[op->rule_dests[n]] = op->points[op->rule_dests[n]];
      }
    }
    else if(op->rules[n] == 3) {  // max
      op->points[op->rule_dests[n]] = (op->XSIZE-1);
      op->positions[op->rule_dests[n]] = op->points[op->rule_dests[n]];
    }
    else if(op->rules[n] == 4) {  // min
      op->points[op->rule_dests[n]] = 0;
      op->positions[op->rule_dests[n]] = op->points[op->rule_dests[n]];
    }
    else if(op->rules[n] == 5) {  // rnd
      op->points[op->rule_dests[n]] = rnd() % op->XSIZE;
      

      op->positions[op->rule_dests[n]] = op->points[op->rule_dests[n]];
    }
    else if(op->rules[n] == 6) {  // up/down
      op->points[op->rule_dests[n]] += rnd() % 3;
      op->points[op->rule_dests[n]]--;


      if(op->points[op->rule_dests[n]] < 0) op->points[op->rule_dests[n]] = 0;
      else if(op->points[op->rule_dests[n]] > (op->XSIZE-1)) op->points[op->rule_dests[n]] = op->XSIZE-1;
      op->positions[op->rule_dests[n]] = op->points[op->rule_dests[n]];  
 
   }
    else if(op->rules[n] == 7) {  // return
      op->points[op->rule_dests[n]] = op->points_save[op->rule_dests[n]];
    }


    //reset
    op->positions[n] += op->points[n] + 1;

    //triggers
    for(m=0;m<8;m++)
      if((op->trig_dests[n] & (1<<m)) != 0)
        op_mp_trigger(op, m);
        // post("\ntrigger",n," -> ", m);
  }
}

// process monome key input
static void op_mp_handler(t_monome* op_monome, u8 x, u8 y, u8 z) {
  op_mp_t *op = (op_mp_t *) op_monome;
  u8 dirty_grid = 0;


  op->prev_mode = op->mode;

  // mode check
  if(x == 0) {
    op->key_count += (z<<1)-1;

    if(op->key_count == 1 && z == 1)
      op->mode = 1; 
    else if(op->key_count == 0)
      op->mode = 0;

    if(z == 1 && op->mode == 1) {
      op->edit_row = y;
      dirty_grid = 1;
      // post("edit row:",op->edit_row);
    }
  }
  else if(x == 1 && op->mode != 0) {
    if(op->mode == 1 && z == 1)
      op->mode = 2;
    else if(op->mode == 2 && z == 0)
      op->mode = 1;
  }
  else if(op->mode == 0 && z == 1) {
    op->points[y] = x;
    op->points_save[y] = x;
    op->positions[y] = x;
    dirty_grid = 1;
  }
  else if(op->mode == 1 && z == 1) {
    if(y != op->edit_row) {    // filter out self-triggering
      op->trig_dests[op->edit_row] ^= (1<<y);
      dirty_grid = 1;
      // post("\ntrig_dests", op->edit_row, ":", op->trig_dests[op->edit_row]);
    }
  }
  else if(op->mode == 2 && z == 1) {
    if(x > 1 && x < 6) {
      op->rule_dests[op->edit_row] = y;
      dirty_grid = 1;
      // post("\nrule_dests", op->edit_row, ":", op->rule_dests[op->edit_row]);
    }
    else if(x > 5) {
      op->rules[op->edit_row] = y;
      dirty_grid = 1;
      // post("\nrules", op->edit_row, ":", op->rules[op->edit_row]);
    }
  }

  if(op->mode != op->prev_mode) {
    dirty_grid = 1;
    // post("\nnew mode", mode);
  }

  if(dirty_grid)
    op_mp_redraw(op_monome);

}

// redraw monome grid
static void op_mp_redraw(t_monome* op_monome) {
  op_mp_t *op = (op_mp_t *) op_monome;
  u8 i1, i2, i3;

  // clear grid
  for(i1=0;i1<128;i1++)
    op_monome->opLedBuffer[i1] = 0;

  // SET POSITIONS
  if(op->mode == 0) {
    for(i1=0;i1<8;i1++) {
      for(i2=op->positions[i1];i2<=op->points[i1];i2++)
        op_monome->opLedBuffer[i1*16 + i2] = L1;

      op_monome->opLedBuffer[i1*16 + op->positions[i1]] = L2;
    }
  }
  // SET ROUTING
  else if(op->mode == 1) {
    op_monome->opLedBuffer[op->edit_row * 16] = L1;
    op_monome->opLedBuffer[op->edit_row * 16 + 1] = L1;

    for(i1=0;i1<8;i1++) {
      if((op->trig_dests[op->edit_row] & (1<<i1)) != 0) {
        for(i2=0;i2<=op->points[i1];i2++)
          op_monome->opLedBuffer[i1*16 + i2] = L2;
      }
      op_monome->opLedBuffer[i1*16 + op->positions[i1]] = L0;
    }
  }
  // SET RULES
  else if(op->mode == 2) {
    op_monome->opLedBuffer[op->edit_row * 16] = L1;
    op_monome->opLedBuffer[op->edit_row * 16 + 1] = L1;

    for(i1=2;i1<7;i1++)
      op_monome->opLedBuffer[op->rule_dests[op->edit_row] * 16 + i1] = L2;

    for(i1=8;i1<16;i1++)
      op_monome->opLedBuffer[op->rules[op->edit_row] * 16 + i1] = L0;

    for(i1=0;i1<8;i1++) 
      op_monome->opLedBuffer[i1*16 + op->positions[i1]] = L0;

    for(i1=0;i1<8;i1++) {
      i3 = glyph[op->rules[op->edit_row]][i1];
      for(i2=0;i2<8;i2++) {
        if((i3 & (1<<i2)) != 0)
          op_monome->opLedBuffer[i1*16 + 8 + i2] = L2;
      }
    }

    op_monome->opLedBuffer[op->rules[op->edit_row] * 16 + 7] = L2;
  }

}


static u32 rnd() {
  x1 = x1 * c1 + a1;
  x2 = x2 * c2 + a2;
  return (x1>>16) | (x2>>16);
}




/* // pickle / unpickle */
/* u8* op_mp_pickle(op_mp_t* mgrid, u8* dst) { */
/*   dst = pickle_io(mgrid->focus, dst); */
/*   dst = pickle_io(mgrid->size, dst); */
/*   u32 *mp_state = (u32*)mgrid->positions; */
/*   while ((u8*)mp_state < (u8*) &(mgrid->edit_row)) { */
/*     dst = pickle_32(*mp_state, dst); */
/*     mp_state +=1; */
/*   } */
/*   /// no state...??? */
/*   return dst; */
/* } */

/* const u8* op_mp_unpickle(op_mp_t* mgrid, const u8* src) { */
/*   src = unpickle_io(src, (u32*)&(mgrid->focus)); */
/*   // FIXME should probably auto-detect grid size here:::: */
/*   src = unpickle_io(src, (u32*)&(mgrid->size)); */
/*   u32 *mp_state = (u32*)mgrid->positions; */
/*   while ((u8*)mp_state < (u8*) &(mgrid->edit_row)) { */
/*     src = unpickle_32(src, mp_state); */
/*     mp_state +=1; */
/*   } */
/*   if(mgrid->focus > 0) { */
/*     net_monome_set_focus( &(mgrid->monome), 1); */
/*   } */
/*   return src; */
/* } */
