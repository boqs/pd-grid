#include "net_monome.h"
#include "lo/lo.h"

static void monome_server_error(int num, const char *m, const char *path);
static int monome_key_handler(const char *path, const char *types, lo_arg ** argv,
			      int argc, void *data, void *user_data);
static int monome_devlist_handler(const char *path, const char *types, lo_arg ** argv,
				  int argc, void *data, void *user_data);
static int monome_size_handler(const char *path, const char *types, lo_arg ** argv,
			       int argc, void *data, void *user_data);
static void monome_send_quadrant (int x, int y, int *testdata);
static void monome_update_128_grid ();
static void serial_osc_grab_focus(void);
static void grid_tick(void *client);
static void net_monome_set_focus(t_monome* grid, u8 focus);

static t_clock *grid_clock;

static int grid_size_x = 16;
static int grid_size_y = 8;

// FIXME defaultLedBuffer should be bundled into monomeOpFocus
u8 defaultLedBuffer[MONOME_MAX_LED_BYTES];
u8 *monomeLedBuffer = defaultLedBuffer;
t_monome* monomeOpFocus = NULL;
static lo_address monome_dev_address;

static lo_server monome_server;

void net_monome_setup (void) {
  if(monome_server == NULL) {
    monome_dev_address = lo_address_new(NULL, "13189");

    monome_server = lo_server_new("6001", monome_server_error);

    lo_server_add_method(monome_server, "/monome/grid/key", "iii",
			 monome_key_handler, NULL);
    lo_server_add_method(monome_server, "/serialosc/device", "ssi",
			 monome_devlist_handler, NULL);
    lo_server_add_method(monome_server, "/sys/size", "ii",
			 monome_size_handler, NULL);
    lo_address a = lo_address_new(NULL, "12002");

    lo_send(a, "/serialosc/list", "si", "localhost", 6001);
    lo_server_recv_noblock(monome_server, 500);
    serial_osc_grab_focus();
    grid_clock = clock_new(NULL, (t_method) grid_tick);
    clock_delay(grid_clock, 10.0);

    int i, j;
    for(i=0; i<16; i++) {
      for(j=0; j<8; j++) {
	defaultLedBuffer[i+j*16] = 15-(i/2 + j);
      }
    }
  }
}

void net_monome_init (t_monome *m, monome_handler_t h) {
  int i;
  for(i=0; i<MONOME_MAX_LED_BYTES; ++i) {
    m->opLedBuffer[i] = 0;
  }
  m->handler = h;
}
void net_monome_deinit (t_monome *m) {
  net_monome_set_focus(m, 0);
}
u8 net_monome_size_x () {
  return grid_size_x;
}
u8 net_monome_size_y () {
  return grid_size_y;
}
u8 net_monome_xy_idx(u8 x, u8 y) {
  return 16 * y + x;
}
static void serial_osc_grab_focus(void) {
  lo_send(monome_dev_address, "/sys/port", "i", 6001);
}

void net_monome_focus(t_monome *m)  {
  net_monome_set_focus(m, 1);
  serial_osc_grab_focus();
}

void net_monome_unfocus(t_monome *m) {
  net_monome_set_focus(m, 0);
  serial_osc_grab_focus();
}

void net_monome_add_focus_methods (t_class *op) {
  class_addmethod((t_class *)op, (t_method)net_monome_unfocus, gensym("unfocus"), A_DEFFLOAT, 0);
  class_addmethod((t_class *)op, (t_method)net_monome_focus, gensym("focus"), 0);
}

void grid_tick(void *client) {
  (void) client;
  double tick_start = clock_getlogicaltime();
  lo_server_recv_noblock(monome_server, 0);

  clock_delay(grid_clock, 10.0 - clock_gettimesince(tick_start));
  monome_update_128_grid();
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
  // XXX hack - this is pretty gross, sorry!  monome quadrant
  // represented in osc by 66 ints - x, y & 64 intensities
  lo_send(monome_dev_address, "/monome/grid/led/level/map", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", x, y, testdata[0], testdata[1], testdata[2], testdata[3], testdata[4], testdata[5], testdata[6], testdata[7], testdata[8], testdata[9], testdata[10], testdata[11], testdata[12], testdata[13], testdata[14], testdata[15], testdata[16], testdata[17], testdata[18], testdata[19], testdata[20], testdata[21], testdata[22], testdata[23], testdata[24], testdata[25], testdata[26], testdata[27], testdata[28], testdata[29], testdata[30], testdata[31], testdata[32], testdata[33], testdata[34], testdata[35], testdata[36], testdata[37], testdata[38], testdata[39], testdata[40], testdata[41], testdata[42], testdata[43], testdata[44], testdata[45], testdata[46], testdata[47], testdata[48], testdata[49], testdata[50], testdata[51], testdata[52], testdata[53], testdata[54], testdata[55], testdata[56], testdata[57], testdata[58], testdata[59], testdata[60], testdata[61], testdata[62], testdata[63]);
}

void net_monome_set_focus(t_monome* op_monome, u8 focus) {
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

void monome_server_error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

int monome_key_handler(const char *path, const char *types, lo_arg ** argv,
		       int argc, void *data, void *user_data) {
  (void) path;
  (void) types;
  (void) data;
  (void) user_data;
  if(argc >= 3 && monomeOpFocus && monomeOpFocus->handler) {
    (*monomeOpFocus->handler)(monomeOpFocus,
			      (u8) argv[0]->i,
			      (u8) argv[1]->i,
			      (u8) argv[2]->i);
  }
  return 1;
}

int monome_size_handler(const char *path, const char *types, lo_arg ** argv,
			int argc, void *data, void *user_data) {
  (void) path;
  (void) types;
  (void) data;
  (void) user_data;
  if(argc >= 2) {
    grid_size_x = argv[0]->i;
    grid_size_y = argv[1]->i;
  }
  return 1;
}

int monome_devlist_handler(const char *path, const char *types, lo_arg ** argv,
			   int argc, void *data, void *user_data) {
  (void) path;
  (void) types;
  (void) data;
  (void) user_data;
  if(argc >= 3) {
    char portno[64];
    sprintf(portno, "%d", argv[2]->i);
    monome_dev_address = lo_address_new(NULL, portno);
    lo_send(monome_dev_address, "sys/info", "i", 6001);
    lo_server_recv_noblock(monome_server, 500);
  }
  return 1;
}
