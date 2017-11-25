#ifndef _ALEPH_BEES_NET_MONOME_H_
#define _ALEPH_BEES_NET_MONOME_H_

#include "types.h"
#include "m_pd.h"
#include "lo/lo.h"

#define MONOME_MAX_LED_BYTES 256
extern u8 defaultLedBuffer[MONOME_MAX_LED_BYTES];
extern u8 *monomeLedBuffer;

typedef void(*monome_handler_t)(void *m, u8 x, u8 y, u8 z);

typedef struct _t_monome {
  t_object x_obj;
  monome_handler_t handler;
  u8 focus;
  u8 opLedBuffer[MONOME_MAX_LED_BYTES];
} t_monome;

extern t_monome* monomeOpFocus;

extern void net_monome_setup (void);
extern void net_monome_init(t_monome *op_monome, monome_handler_t h);

extern void net_monome_set_focus(t_monome* grid, u8 focus);
extern void net_monome_add_focus_methods (t_class *op);
extern void net_monome_grid_clear(void);
extern u8 net_monome_size_x (void);
extern u8 net_monome_size_y (void);
extern  u8 net_monome_xy_idx(u8 x, u8 y);

#endif // h guard
