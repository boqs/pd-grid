#include "m_pd.h"
#include "net_monome.h"
#include "lo/lo.h"
typedef struct _grid {
  t_object  x_obj;
  op_monome_t monome;
  t_outlet *button_out;
} t_grid;

static lo_server monome_server;
static void monome_server_error(int num, const char *m, const char *path);
static int monome_key_handler(const char *path, const char *types, lo_arg ** argv,
			      int argc, void *data, void *user_data);
static void monome_send_quadrant (int x, int y, int *testdata);
static void monome_update_128_grid ();
static void serial_osc_grab_focus(void);
 
static t_class *grid_class;

t_clock *grid_clock;


void grid_led(t_grid *op, t_floatarg x, t_floatarg y, t_floatarg z)  {
  int z_16 = (int) z;
  if(z_16 < 0) {
    z = 0;
  } else if (z_16 >= 16) {
    z = 15;
  }
  op->monome.opLedBuffer[(int) x + (int) y * 16] = z_16;
}

void grid_tick(void *client) {
  (void) client;
  double tick_start = clock_getlogicaltime();
  lo_server_recv_noblock(monome_server, 0);

  clock_delay(grid_clock, 10.0 - clock_gettimesince(tick_start));
  monome_update_128_grid();
}

void grid_focus(t_grid *op)  {
  net_monome_set_focus(&(op->monome), 1);
  serial_osc_grab_focus();
}

void grid_unfocus(t_grid *op) {
  net_monome_set_focus(&(op->monome), 0);
  serial_osc_grab_focus();
}

void serial_osc_grab_focus(void) {
  lo_address a = lo_address_new(NULL, "13188");
  lo_send(a, "/sys/port", "i", 6001);
}

void add_focus_methods (t_class *c) {
  class_addmethod(c, (t_method)grid_unfocus, gensym("unfocus"), A_DEFFLOAT, 0);
  class_addmethod(c, (t_method)grid_focus, gensym("focus"), 0);
}

void monome_update_128_grid () {
  int i, j;
  int testdata[64];
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

void net_monome_set_focus(op_monome_t* op_monome, u8 focus) {
  if(focus > 0) {
    if((monomeOpFocus != NULL) && (monomeOpFocus != op_monome)) {
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

void raw_button_handler(void* op, u8 x, u8 y, u8 z) {
  t_grid *grid_op = (t_grid *) op;
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
  t_grid *x = (t_grid *)pd_new(grid_class);
  net_monome_init(&(x->monome), x, raw_button_handler);

  net_monome_set_focus(&(x->monome), 1);
  
  x->button_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;  
}

void grid_setup(void) {
  grid_class = class_new(gensym("grid"),  
			    (t_newmethod)grid_new,  
			    0, sizeof(t_grid),  
			    CLASS_DEFAULT,  
			    A_GIMME, 0);  
  add_focus_methods( grid_class);
  class_addmethod(grid_class,  
		  (t_method)grid_led, gensym("led"),  
		  A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);  
 
  int i, j;
  for(i=0; i<16; i++) {
    for(j=0; j<8; j++) {
      defaultLedBuffer[i+j*16] = 15-(i/2 + j);
    }
  }
  serial_osc_grab_focus();
  monome_server = lo_server_new("6001", monome_server_error);

  lo_server_add_method(monome_server, "/monome/grid/key", "iii",
		       monome_key_handler, NULL);

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
  (void) path;
  (void) types;
  (void) argc;
  (void) data;
  (void) user_data;
  if(monomeOpFocus && monomeOpFocus->handler) {
    (*monomeOpFocus->handler)(monomeOpFocus->op, (u8) argv[0]->i,
			      (u8) argv[1]->i,
			      (u8) argv[2]->i);
  }
  return 1;
}
u8 monome_size_x () {
  // FIXME for now we only are compatible with 128 grid
  return 16;
}
u8 monome_size_y () {
  // FIXME for now we only are compatible with 128 grid
  return 8;
}
u8 monome_xy_idx(u8 x, u8 y) {
  return 16 * y + x;
}
