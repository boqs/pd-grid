#include "step.h"

static void op_step_handler(op_step_t *op, u8 x, u8 y, u8 z);
static void op_step_in_step(op_step_t* op, float v);
static void op_step_in_bang(op_step_t* op);
static void op_step_in_size(op_step_t* op, float v);

// pickles
void op_step_pickle(op_step_t* mgrid, FILE *f);
void op_step_unpickle(op_step_t* mgrid, FILE *f);

static t_class *op_step_class;
void *step_new(t_symbol *s, int argc, t_atom *argv);
void step_free(void* op);
void step_setup (void) {
  net_monome_setup();
  op_step_class = class_new(gensym("step"),
			    (t_newmethod)step_new,
			    (t_method)step_free,
			    sizeof(op_step_t),
			    CLASS_DEFAULT,
			    A_GIMME, 0);
  class_addbang(op_step_class, (t_method)op_step_in_bang);
  class_addmethod(op_step_class,
		  (t_method)op_step_in_step, gensym("step"), A_DEFFLOAT, 0);
  class_addmethod(op_step_class,
		  (t_method)op_step_in_size, gensym("size"), A_DEFFLOAT, 0);
  net_monome_add_focus_methods(op_step_class);
}

//----- extern function definition
void *step_new(t_symbol *s, int argc, t_atom *argv) {
  (void) s;
  (void) argc;
  (void) argv;

  u8 i;
  op_step_t *op = (op_step_t *)pd_new(op_step_class);
  
  op->tr = outlet_new((t_object *) op, &s_list);
  op->mono = outlet_new((t_object *) op, &s_list);
  op->pos = outlet_new((t_object *) op, &s_list);

  /* op->super.pickle = (op_pickle_fn) (&op_step_pickle); */
  /* op->super.unpickle = (op_unpickle_fn) (&op_step_unpickle); */

  //--- monome
  net_monome_init(&op->monome, (monome_handler_t)op_step_handler,
		  (monome_pickle_t)&op_step_pickle,
		  (monome_pickle_t)&op_step_unpickle);

  // superclass state

  for(i=0;i<16;i++) {
    op->steps[0][i] = 0;
    op->steps[1][i] = 0;
    op->steps[2][i] = 0;
    op->steps[3][i] = 0;
  }

  op->size = net_monome_size_x();

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
  op->monome.opLedBuffer[net_monome_xy_idx(0, 0)] = 15;
  op->monome.opLedBuffer[net_monome_xy_idx(0, 2)] = 15;
  for(i=0;i<op->size;i++) {
    op->monome.opLedBuffer[net_monome_xy_idx(i, 1)] = 15;
    op->monome.opLedBuffer[net_monome_xy_idx(i, 3)] = 15;
  }
  return op;
}

// de-init
void step_free(void* op) {
  // release focus
  net_monome_deinit((t_monome*)op);
}

//-------------------------------------------------
//----- static function definition

static void op_step_in_size(op_step_t* op, float v) {
  if(v < 9) op->size = 8;
  else op->size = 16;
}

static void op_step_in_bang(op_step_t *op) {
  op_step_in_step(op, 1.0);
}

static void op_step_in_step(op_step_t* op, float v) {
  op->step = v;
  s8 i;

  if(op->s_cut == 0) {
    op->monome.opLedBuffer[net_monome_xy_idx(op->s_now, 0)] = 0;

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

    op->monome.opLedBuffer[net_monome_xy_idx(op->s_now, 0)] = 15;
  }

  if(op->s_cut2 == 0) {
    op->monome.opLedBuffer[net_monome_xy_idx(op->s_now2, 2)] = 0;

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

    op->monome.opLedBuffer[net_monome_xy_idx(op->s_now2, 2)] = 15;
  }

  op->s_cut = 0;
  op->s_cut2 = 0;

  
  t_atom stepOut[2];
  stepOut[0].a_type = A_FLOAT;
  stepOut[1].a_type = A_FLOAT;

  int k;
  for(k=0; k < 4; k++) {
    if(op->steps[k][op->s_now]) {
      stepOut[0].a_w.w_float = 0.0;
      stepOut[1].a_w.w_float = (float) k;
      outlet_list(op->tr, &s_list, 2, stepOut);
    }
  }
  for(k=0; k < 4; k++) {
    if(op->steps[k][op->s_now2]) {
      stepOut[0].a_w.w_float = 1.0;
      stepOut[1].a_w.w_float = (float) k;
      outlet_list(op->tr, &s_list, 2, stepOut);
    }
  }
  i = (op->steps[0][op->s_now]) + (op->steps[1][op->s_now] << 1) + (op->steps[2][op->s_now] << 2) + (op->steps[3][op->s_now] << 3);
  stepOut[0].a_w.w_float = (float) i;
  i = (op->steps[0][op->s_now2]) + (op->steps[1][op->s_now2] << 1) + (op->steps[2][op->s_now2] << 2) + (op->steps[3][op->s_now2] << 3);
  stepOut[1].a_w.w_float = (float) i;
  outlet_list(op->mono, &s_list, 2, stepOut);

  stepOut[0].a_w.w_float = (float) op->s_now;
  stepOut[1].a_w.w_float = (float) op->s_now2;
  outlet_list(op->pos, &s_list, 2, stepOut);
}

static void op_step_handler(op_step_t *op, u8 x, u8 y, u8 z) {
  t_monome *op_monome = &op->monome;
  u8 i;

  // only care about key-downs
  if(z) {
    // row 0 = postion cut, set start point
    if(y==0) {
      op->s_start = x;
      op_monome->opLedBuffer[net_monome_xy_idx(op->s_now, 0)] = 0;
      op->s_now = x;
      op_monome->opLedBuffer[net_monome_xy_idx(op->s_now, 0)] = 15;

      op->s_end = op->s_start + op->s_length;
      if(op->s_end > (op->size-1)) op->s_end -= op->size;

      if(op->s_end >= op->s_start)
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 1)] = (i >= op->s_start && i <= op->s_end) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 1)] = (i >= op->s_start || i <= op->s_end) * 15;
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
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 1)] = (i >= op->s_start && i <= op->s_end) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 1)] = (i >= op->s_start || i <= op->s_end) * 15;
	}
      }


    // set loop start 2
    } else if(y==2) {
      op->s_start2 = x;
      op_monome->opLedBuffer[net_monome_xy_idx(op->s_now2, 2)] = 0;
      op->s_now2 = x;
      op_monome->opLedBuffer[net_monome_xy_idx(op->s_now2, 2)] = 15;

      op->s_end2 = op->s_start2 + op->s_length2;
      if(op->s_end2 > (op->size-1)) op->s_end2 -= op->size;

      if(op->s_end2 >= op->s_start2)
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 3)] = (i >= op->s_start2 && i <= op->s_end2) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 3)] = (i >= op->s_start2 || i <= op->s_end2) * 15;
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
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 3)] = (i >= op->s_start2 && i <= op->s_end2) * 15;
	}
      else {
	for(i=0;i<op->size;i++) {
	  op_monome->opLedBuffer[net_monome_xy_idx(i, 3)] = (i >= op->s_start2 || i <= op->s_end2) * 15;
	}
      }

    // rows 4-7: set steps
    } else if(y>3 && y<8) {
      op->steps[y-4][x] ^= 1;
      op_monome->opLedBuffer[net_monome_xy_idx(x, y)] = op->steps[y-4][x] * 15;
    }
  }
}



/* // pickle / unpickle */
void op_step_pickle(op_step_t* mgrid, FILE *f) {
  fwrite(&mgrid->size, sizeof(mgrid->size), 1, f);
  fwrite(&mgrid->step, sizeof(mgrid->size), 1, f);
  fwrite(&mgrid->s_start, sizeof(s8), 1, f);
  fwrite(&mgrid->s_end, sizeof(s8), 1, f);
  fwrite(&mgrid->s_length, sizeof(s8), 1, f);
  fwrite(&mgrid->s_now, sizeof(s8), 1, f);
  fwrite(&mgrid->s_cut, sizeof(s8), 1, f);
  fwrite(&mgrid->s_start2, sizeof(s8), 1, f);
  fwrite(&mgrid->s_end2, sizeof(s8), 1, f);
  fwrite(&mgrid->s_length2, sizeof(s8), 1, f);
  fwrite(&mgrid->s_now2, sizeof(s8), 1, f);
  fwrite(&mgrid->s_cut2, sizeof(s8), 1, f);
  fwrite(mgrid->steps, sizeof(s8), 4 * 16, f);

  fwrite(mgrid->monome.opLedBuffer, sizeof(u8), MONOME_MAX_LED_BYTES, f);
}

void op_step_unpickle(op_step_t* mgrid, FILE *f) {
  fread(&mgrid->size, sizeof(mgrid->size), 1, f);
  fread(&mgrid->step, sizeof(mgrid->size), 1, f);
  fread(&mgrid->s_start, sizeof(s8), 1, f);
  fread(&mgrid->s_end, sizeof(s8), 1, f);
  fread(&mgrid->s_length, sizeof(s8), 1, f);
  fread(&mgrid->s_now, sizeof(s8), 1, f);
  fread(&mgrid->s_cut, sizeof(s8), 1, f);
  fread(&mgrid->s_start2, sizeof(s8), 1, f);
  fread(&mgrid->s_end2, sizeof(s8), 1, f);
  fread(&mgrid->s_length2, sizeof(s8), 1, f);
  fread(&mgrid->s_now2, sizeof(s8), 1, f);
  fread(&mgrid->s_cut2, sizeof(s8), 1, f);
  fread(mgrid->steps, sizeof(s8), 4 * 16, f);

  fread(mgrid->monome.opLedBuffer, sizeof(u8), MONOME_MAX_LED_BYTES, f);
}
