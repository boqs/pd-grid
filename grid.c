#include "net_monome.h"

typedef struct _grid {
  t_monome monome;
  t_outlet *button_out;
} t_grid;

static t_class *grid_class;


static void grid_led(t_grid *op, t_floatarg x, t_floatarg y, t_floatarg z)  {
  int z_16 = (int) z;
  if(z_16 < 0) {
    z = 0;
  } else if (z_16 >= 16) {
    z = 15;
  }
  op->monome.opLedBuffer[(int) x + (int) y * 16] = z_16;
}

static void grid_key_handler(t_grid *grid_op, u8 x, u8 y, u8 z) {
  t_atom but[3];
  but[0].a_type = A_FLOAT;
  but[0].a_w.w_float = (float) x;
  but[1].a_type = A_FLOAT;
  but[1].a_w.w_float = (float)y;
  but[2].a_type = A_FLOAT;
  but[2].a_w.w_float = (float) z;
  outlet_anything(grid_op->button_out, gensym("key"),
		  3, but);
}

void *grid_new(t_symbol *s, int argc, t_atom *argv) {
  (void) argc;
  (void) argv;
  (void) s;
  t_grid *grid_obj = (t_grid *) pd_new(grid_class);

  net_monome_init(&grid_obj->monome, (monome_handler_t)grid_key_handler);
  
  grid_obj->button_out = outlet_new((t_object *)grid_obj, &s_float);

  return (void *)grid_obj;  
}

void grid_setup(void) {
  net_monome_setup();
  grid_class = class_new(gensym("grid"),  
			    (t_newmethod)grid_new,  
			    0, sizeof(t_grid),  
			    CLASS_DEFAULT,  
			    A_GIMME, 0);  
  net_monome_add_focus_methods(grid_class);
  class_addmethod(grid_class,  
		  (t_method)grid_led, gensym("led"),  
		  A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);  
 
  int i, j;
  for(i=0; i<16; i++) {
    for(j=0; j<8; j++) {
      defaultLedBuffer[i+j*16] = 15-(i/2 + j);
    }
  }
}

void grid_free(void* op) {
  // release focus
  net_monome_deinit((t_monome*)op);
}


