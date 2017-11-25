#include "step.h"

static void op_step_handler(op_step_t* op, u8 x, u8 y, u8 z);

static void op_step_in_step(op_step_t* op, float v);
static void op_step_in_size(op_step_t* op, float v);
void op_step_focus(op_step_t *op)  {
  net_monome_set_focus(&(op->monome), 1);
  serial_osc_grab_focus();
}

void op_step_unfocus(op_step_t *op) {
  net_monome_set_focus(&(op->monome), 0);
  serial_osc_grab_focus();
}

static t_class *op_step_class;
void *step_new(t_symbol *s, int argc, t_atom *argv);
void step_setup (void) {
  op_step_class = class_new(gensym("step"),
			    (t_newmethod)step_new,
			    0, sizeof(op_step_t),
			    CLASS_DEFAULT,
			    A_GIMME, 0);
  class_addmethod(op_step_class,
		  (t_method)op_step_in_step, gensym("step"), A_DEFFLOAT, 0);
  class_addmethod(op_step_class,
		  (t_method)op_step_in_size, gensym("size"), A_DEFFLOAT, 0);
  class_addmethod(op_step_class, (t_method)op_step_unfocus, gensym("unfocus"), A_DEFFLOAT, 0);
  class_addmethod(op_step_class, (t_method)op_step_focus, gensym("focus"), 0);
}

//----- extern function definition
void *step_new(t_symbol *s, int argc, t_atom *argv) {
  (void) s;
  (void) argc;
  (void) argv;

  u8 i;
  op_step_t *op = (op_step_t *)pd_new(op_step_class);

  op->a = outlet_new(&op->x_obj, &s_float);
  op->b = outlet_new(&op->x_obj, &s_float);
  op->c = outlet_new(&op->x_obj, &s_float);
  op->d = outlet_new(&op->x_obj, &s_float);
  op->mono1 = outlet_new(&op->x_obj, &s_float);
  op->pos1 = outlet_new(&op->x_obj, &s_float);
  op->mono2 = outlet_new(&op->x_obj, &s_float);
  op->pos2 = outlet_new(&op->x_obj, &s_float);

  /* op->super.pickle = (op_pickle_fn) (&op_step_pickle); */
  /* op->super.unpickle = (op_unpickle_fn) (&op_step_unpickle); */

  //--- monome
  net_monome_init(&op->monome, op, (monome_handler_t)op_step_handler);

  // superclass state

  for(i=0;i<16;i++) {
    op->steps[0][i] = 0;
    op->steps[1][i] = 0;
    op->steps[2][i] = 0;
    op->steps[3][i] = 0;
  }

  op->size = monome_size_x();

  op->s_start = 0;
  op->s_end = op->size - 1;
  op->s_length = op->size;
  op->s_now = 0;
  op->s_cut = 0;

  op->s_start2 = 0;
  op->s_end2 = op->size - 1;
  op->s_length2 = op->size;
  op->s_now2 = 0;
  op->s_cut2 = 0;


  // init monome drawing, maybe should clear first
  op->monome.opLedBuffer[monome_xy_idx(0, 0)] = 15;
  op->monome.opLedBuffer[monome_xy_idx(0, 2)] = 15;
  for(i=0;i<op->size;i++) {
    op->monome.opLedBuffer[monome_xy_idx(i, 1)] = 15;
    op->monome.opLedBuffer[monome_xy_idx(i, 3)] = 15;
  }
  return op;
}

// de-init
void op_step_deinit(void* op) {
  // release focus
  net_monome_set_focus(&(((op_step_t*)op)->monome), 0);
}

//-------------------------------------------------
//----- static function definition

static void op_step_in_size(op_step_t* op, float v) {
  if(v < 9) op->size = 8;
  else op->size = 16;
}

static void op_step_in_step(op_step_t* op, float v) {
  s8 i;

  if(op->s_cut == 0) {
    op->monome.opLedBuffer[monome_xy_idx(op->s_now, 0)] = 0;

    if(v > 0) {
      for(i=0;i<v;i++) {
	if(op->s_now == op->s_end) op->s_now = op->s_start;
	else {
	  op->s_now++;
	  if(op->s_now == op->size) op->s_now = 0;
	}
      }
    } else {
      for(i=v;i<0;i++) {
	if(op->s_now == op->s_start) op->s_now = op->s_end;
	else if(op->s_now == 0) op->s_now = op->size - 1;
	else op->s_now--;
      }
    }

    op->monome.opLedBuffer[monome_xy_idx(op->s_now, 0)] = 15;
  }

  if(op->s_cut2 == 0) {
    op->monome.opLedBuffer[monome_xy_idx(op->s_now2, 2)] = 0;

    if(v > 0) {
      for(i=0;i<v;i++) {
	if(op->s_now2 == op->s_end2) op->s_now2 = op->s_start2;
	else {
	  op->s_now2++;
	  if(op->s_now2 == op->size) op->s_now2 = 0;
	}
      }
    } else {
      for(i=v;i<0;i++) {
	if(op->s_now2 == op->s_start2) op->s_now2 = op->s_end2;
	else if(op->s_now2 == 0) op->s_now2 = op->size - 1;
	else op->s_now2--;
      }
    }

    op->monome.opLedBuffer[monome_xy_idx(op->s_now2, 2)] = 15;
  }

  op->s_cut = 0;
  op->s_cut2 = 0;

  outlet_float(op->a, (float)op->steps[0][op->s_now]);
  outlet_float(op->b, (float)op->steps[1][op->s_now]);
  outlet_float(op->c, (float)op->steps[2][op->s_now]);
  outlet_float(op->d, (float)op->steps[3][op->s_now]);

  i = (op->steps[0][op->s_now]) + (op->steps[1][op->s_now] << 1) + (op->steps[2][op->s_now] << 2) + (op->steps[3][op->s_now] << 3);
  outlet_float(op->mono1, (float) i);
  outlet_float(op->pos1, (float) op->s_now);

  i = (op->steps[0][op->s_now2]) + (op->steps[1][op->s_now2] << 1) + (op->steps[2][op->s_now2] << 2) + (op->steps[3][op->s_now2] << 3);
  outlet_float(op->mono2, (float) i);
  outlet_float(op->pos2, (float) op->s_now);
}

static void op_step_handler(op_step_t* op, u8 x, u8 y, u8 z) {
  op_monome_t *op_monome = &op->monome;
  u8 i;

  // only care about key-downs
  if(z) {
    // row 0 = postion cut, set start point
    if(y==0) {
      op->s_start = x;
      op_monome->opLedBuffer[monome_xy_idx(op->s_now, 0)] = 0;
      op->s_now = x;
      op_monome->opLedBuffer[monome_xy_idx(op->s_now, 0)] = 15;

      op->s_end = op->s_start + op->s_length;
      if(op->s_end > (op->size-1)) op->s_end -= op->size;

      if(op->s_end >= op->s_start)
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 1)] = (i >= op->s_start && i <= op->s_end) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 1)] = (i >= op->s_start || i <= op->s_end) * 15;
	}
      }

      op->s_cut = 1;

    // row 1 = change loop point
    } else if(y==1) {
      op->s_end = x;
      op->s_length = op->s_end - op->s_start;
      if(op->s_length < 0) op->s_length += op->size;

      if(op->s_end >= op->s_start)
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 1)] = (i >= op->s_start && i <= op->s_end) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 1)] = (i >= op->s_start || i <= op->s_end) * 15;
	}
      }


    // set loop start 2
    } else if(y==2) {
      op->s_start2 = x;
      op_monome->opLedBuffer[monome_xy_idx(op->s_now2, 2)] = 0;
      op->s_now2 = x;
      op_monome->opLedBuffer[monome_xy_idx(op->s_now2, 2)] = 15;

      op->s_end2 = op->s_start2 + op->s_length2;
      if(op->s_end2 > (op->size-1)) op->s_end2 -= op->size;

      if(op->s_end2 >= op->s_start2)
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 3)] = (i >= op->s_start2 && i <= op->s_end2) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 3)] = (i >= op->s_start2 || i <= op->s_end2) * 15;
	}
      }

      op->s_cut2 = 1;

    // row 3 = change loop point 2
    } else if(y==3) {
      op->s_end2 = x;
      op->s_length2 = op->s_end2 - op->s_start2;
      if(op->s_length2 < 0) op->s_length2 += op->size;

      if(op->s_end2 >= op->s_start2)
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 3)] = (i >= op->s_start2 && i <= op->s_end2) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[monome_xy_idx(i, 3)] = (i >= op->s_start2 || i <= op->s_end2) * 15;
	}
      }

    // rows 4-7: set steps
    } else if(y>3 && y<8) {
      op->steps[y-4][x] ^= 1;
      op_monome->opLedBuffer[monome_xy_idx(x, y)] = op->steps[y-4][x] * 15;
    }
  }
}



/* // pickle / unpickle */
/* u8* op_step_pickle(op_step_t* mgrid, u8* dst) { */
/*   dst = pickle_io(mgrid->focus, dst); */
/*   dst = pickle_io(mgrid->size, dst); */
/*   u32 *step_state = (u32*)&mgrid->s_start; */
/*   while ((u8*)step_state <= (u8*) &(mgrid->steps[3][15])) { */
/*     dst = pickle_32(*step_state, dst); */
/*     step_state +=1; */
/*   } */
/*   int i; */
/*   for (i=0; i < 256; i++) { */
/*     *dst = mgrid->monome.opLedBuffer[i]; */
/*     dst++; */
/*   } */

/*   /// no state...??? */
/*   return dst; */
/* } */

/* const u8* op_step_unpickle(op_step_t* mgrid, const u8* src) { */
/*   src = unpickle_io(src, (u32*)&(mgrid->focus)); */
/*   src = unpickle_io(src, (u32*)&(mgrid->size)); */
/*   u32 *step_state = (u32*)&mgrid->s_start; */
/*   while ((u8*)step_state <= (u8*) &(mgrid->steps[3][15])) { */
/*     src = unpickle_32(src, step_state); */
/*     step_state +=1; */
/*   } */
/*   int i; */
/*   for (i=0; i < 256; i++) { */
/*     mgrid->monome.opLedBuffer[i] = *src; */
/*     src++; */
/*   } */

/*   if( mgrid->focus > 0) { */
/*     net_monome_set_focus( &(mgrid->monome), 1); */
/*   } */
/*   return src; */
/* } */
