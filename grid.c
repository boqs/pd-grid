#include "m_pd.h"
#include "lo/lo.h"
#include "net_monome.h"
 
static t_class *grid_class;
static lo_server monome_server;
/* extern int shared_var; */
t_clock *grid_clock;

typedef struct _grid {  
  t_object  x_obj;
  op_monome_t monome;
  t_outlet *button_out;
} t_grid;
void monome_server_error(int num, const char *m, const char *path);
int monome_key_handler(const char *path, const char *types, lo_arg ** argv,
		       int argc, void *data, void *user_data);
void monome_testmap (int x, int y);
void monome_testbang (void);

void* always_update_128_grid( void* argument );
void* always_poll_timers( void* argument );
void monome_send_quadrant (int x, int y, int *testdata);
void monome_update_128_grid ();
void serial_osc_grab_focus(void);

void serial_osc_grab_focus(void) {
  lo_address a = lo_address_new(NULL, "13188");
  lo_send(a, "/sys/port", "i", 6001);
}

void monome_update_128_grid () {
  int i, j;
  int testdata[64];
  // FIXME add this early out maybe???
  /* if(!monomeFrameDirty) { */
  /*   return; */
  /* } */
  /* monomeFrameDirty = 0; */
  for(i=0; i<8; i++) {
    for(j=0; j<8; j++) {
      testdata[i+j*8] = (int)monomeLedBuffer[i+j*16];
    }
  }
  monome_send_quadrant(0, 0, testdata);
  for(i=0; i<8; i++) {
    for(j=0; j<8; j++) {
      testdata[i+j*8] = (int)monomeLedBuffer[i+8+j*16];
    }
  }
  monome_send_quadrant(8, 0, testdata);
}

void monome_send_quadrant (int x, int y, int *testdata) {
  lo_address a = lo_address_new(NULL, "13188");
  // XXX hack - this is pretty gross, sorry!  monome quadrant
  // represented in osc by 66 ints - x, y & 64 intensities
  lo_send(a, "/monome/grid/led/level/map", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", x, y, testdata[0], testdata[1], testdata[2], testdata[3], testdata[4], testdata[5], testdata[6], testdata[7], testdata[8], testdata[9], testdata[10], testdata[11], testdata[12], testdata[13], testdata[14], testdata[15], testdata[16], testdata[17], testdata[18], testdata[19], testdata[20], testdata[21], testdata[22], testdata[23], testdata[24], testdata[25], testdata[26], testdata[27], testdata[28], testdata[29], testdata[30], testdata[31], testdata[32], testdata[33], testdata[34], testdata[35], testdata[36], testdata[37], testdata[38], testdata[39], testdata[40], testdata[41], testdata[42], testdata[43], testdata[44], testdata[45], testdata[46], testdata[47], testdata[48], testdata[49], testdata[50], testdata[51], testdata[52], testdata[53], testdata[54], testdata[55], testdata[56], testdata[57], testdata[58], testdata[59], testdata[60], testdata[61], testdata[62], testdata[63]);
}

void monome_testbang () {
  lo_address a = lo_address_new(NULL, "13188");
  int i, j;
  for(i=0; i<16; i++) {
    for(j=0; j<8; j++) {
      lo_send(a, "/monome/grid/led/level/set", "iii", i, j, i/2+j);
    }
  }
}

void grid_tick(t_grid *client) {
  /* post("bees tick"); */
  double tick_start = clock_getlogicaltime();
  // FIXME put something back in to do this job...
  /* process_timers(); */
  lo_server_recv_noblock(monome_server, 0);

  // FIXME where's the event loop in pd!?
  /* static event_t e; */
  /* while( event_next(&e) ) { */
  /*   (app_event_handlers)[e.type](e.data); */
  /* } */

  clock_delay(grid_clock, 10.0 - clock_gettimesince(tick_start));
  monome_update_128_grid();
}
 
/* void grid_bang(t_grid *x)   */
/* {   */
/*   t_float f=x->i_count; */
/*   t_int step = x->step; */
/*   x->i_count+=step; */
 
/*   if (x->i_down-x->i_up) { */
/*     if ((step>0) && (x->i_count > x->i_up)) { */
/*       x->i_count = x->i_down; */
/*       outlet_bang(x->b_out); */
/*     } else if (x->i_count < x->i_down) { */
/*       x->i_count = x->i_up; */
/*       outlet_bang(x->b_out); */
/*     } */
/*   } */
 
/*   outlet_float(x->f_out, f); */
/* } */
 
/* void grid_reset(t_grid *x)   */
/* {   */
/* } */  
 
/* void grid_set(t_grid *x, t_floatarg f)   */
/* {   */
/* }   */
 
void grid_led(t_grid *op, t_floatarg x, t_floatarg y, t_floatarg z)  {
  printf("LED bang = %f,%f,%f\n", x, y, z);
  int z_16 = (int) z;
  if(z_16 < 0) {
    z = 0;
  } else if (z_16 >= 16) {
    z = 15;
  }
  op->monome.opLedBuffer[(int) x + (int) y * 16] = z_16;
}

void raw_button_handler(void* op, u8 x, u8 y, u8 z) {
  t_grid *grid_op = (t_grid *) op;
  printf("raw_button_handler plumbing test: %d, %d, %d\n");
  outlet_float(grid_op->button_out, x);
}

void *grid_new(t_symbol *s, int argc, t_atom *argv) {
  (void) argc;
  (void) argv;
  (void) s;
  /* shared_var++; */
  /* printf("shared_var = %d\n", shared_var); */
  t_grid *x = (t_grid *)pd_new(grid_class);
  net_monome_init(&(x->monome), x, raw_button_handler);

  // XXX FIXME
  // don't grab focus like this here, receive a focus message instead
  net_monome_set_focus(&(x->monome), 1);
  
  /* t_float f1=0, f2=0;   */
 
  /* switch(argc){   */
  /* default:   */
  /* case 3:   */
  /*   x->step=atom_getfloat(argv+2); */
  /*   break; */
  /* case 2:   */
  /*   f2=atom_getfloat(argv+1); */
  /*   break; */
  /* case 1:   */
  /*   f1=atom_getfloat(argv);   */
  /*   break;   */
  /* case 0:   */
  /*   break;   */
  /* }   */
  /* if (argc<2)f2=f1;   */
 
  /* x->i_down = (f1<f2)?f1:f2;   */
  /* x->i_up   = (f1>f2)?f1:f2;   */
 
  /* x->i_count=x->i_down;   */
 
  /* inlet_new(&x->x_obj, &x->x_obj.ob_pd,   */
  /* 	    gensym("list"), gensym("bound"));   */
  /* floatinlet_new(&x->x_obj, &x->step);   */
 
  x->button_out = outlet_new(&x->x_obj, &s_float);
  /* x->b_out = outlet_new(&x->x_obj, &s_bang);   */
 
  return (void *)x;  
}
 
void grid_setup(void) {
  grid_class = class_new(gensym("grid"),  
			    (t_newmethod)grid_new,  
			    0, sizeof(t_grid),  
			    CLASS_DEFAULT,  
			    A_GIMME, 0);  
 
  /* class_addbang (grid_class, grid_bang); */
  /* class_addmethod(grid_class,   */
  /* 		  (t_method)grid_reset, gensym("reset"), 0);   */
  /* class_addmethod(grid_class,   */
  /* 		  (t_method)grid_set, gensym("set"),   */
  /* 		  A_DEFFLOAT, 0);   */
  /* class_addmethod(grid_class,   */
  /* 		  (t_method)grid_bound, gensym("bound"),   */
  /* 		  A_DEFFLOAT, A_DEFFLOAT, 0);   */
  class_addmethod(grid_class,  
		  (t_method)grid_led, gensym("led"),  
		  A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);  
 
  /* class_sethelpsymbol(grid_class, gensym("help-grid")); */


  // copied over from pd_bees
  monome_testbang();
  int i, j;
  for(i=0; i<16; i++) {
    for(j=0; j<8; j++) {
      defaultLedBuffer[i+j*16] = 15-(i/2 + j);
    }
  }
  monome_update_128_grid(NULL);
  serial_osc_grab_focus();
  monome_server = lo_server_new("6001", monome_server_error);

  /* add method that will match any path and args */
  lo_server_add_method(monome_server, "/monome/grid/key", "iii",
		       monome_key_handler, NULL);
  // replace BEES' timer_add with some simple thingy
  /* timer_add(&monomeGridLedTimer, 20, &monome_update_128_grid, NULL); */

  grid_clock = clock_new(NULL, (t_method) grid_tick);
  clock_delay(grid_clock, 10.0);

}

void monome_server_error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}
int monome_key_handler(const char *path, const char *types, lo_arg ** argv,
		       int argc, void *data, void *user_data) {
  // FIXME do some useful shit
  /* int i; */
  if(argv[2]->i > 0) {
    printf("keydown: %d,%d\n", (int)argv[0]->i, (int)argv[1]->i);
  } else {
    printf("keyup: %d,%d\n", (int)argv[0]->i, (int)argv[1]->i);
  }
  if(monomeOpFocus && monomeOpFocus->handler) {
    (*monomeOpFocus->handler)(monomeOpFocus->op, (u8) argv[0]->i,
			      (u8) argv[1]->i,
			      (u8) argv[2]->i);
  }
  return 1;
}
