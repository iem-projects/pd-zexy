/*
 * mulitplex   :  multiplex a specified input to the output
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 1903:forum::für::umläute:2005 */

/*
 * THINK: should the selector-inlet be the first or the last ???
 * pros/cons:
 *  the 1st inlet being the selector is not consistent with pd (hot/cold)
 *   but it since the hot inlet is selectable, the whole object is not really
 *   consistent
 *  numbering would have to start with 1 (for the 1st not-leftmost inlet)
 * if the selector is rightmost this would mean: cold is right(most), hot is (somewhere) left
 * numbering would be ok
 *
 * conclusio: make the selector rightmost
 *
 */

#include "zexy.h"
#include <stdio.h>

/* ------------------------- mux ------------------------------- */

/*
  a multiplexer
*/

static t_class *mux_class = NULL;
static t_class *muxproxy_class = NULL;

typedef struct _mux {
  t_object x_obj;
  struct _muxproxy **x_proxy;

  int i_count;
  t_float f_selected;
  t_inlet **in;
} t_mux;

typedef struct _muxproxy {
  t_pd p_pd;
  t_mux *p_master;
  int id;
} t_muxproxy;

static void mux_anything(t_muxproxy *y, t_symbol *s, int argc, t_atom *argv)
{
  t_mux *x = y->p_master;
  if (y->id == (int)x->f_selected) {
    outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
  }
}

static void *mux_new(t_symbol *UNUSED(s), int argc, t_atom *UNUSED(argv))
{
  int n = (argc < 2) ? 2 : argc;
  t_mux *x = (t_mux *)pd_new(mux_class);

  x->f_selected = 0;
  x->i_count = n;
  x->in = (t_inlet **)getbytes(x->i_count * sizeof(t_inlet *));
  x->x_proxy = (t_muxproxy **)getbytes(x->i_count * sizeof(t_muxproxy *));

  for (n = 0; n < x->i_count; n++) {
    x->x_proxy[n] = (t_muxproxy *)pd_new(muxproxy_class);
    x->x_proxy[n]->p_master = x;
    x->x_proxy[n]->id = n;
    x->in[n] = inlet_new((t_object *)x, (t_pd *)x->x_proxy[n], 0, 0);
  }

  floatinlet_new(&x->x_obj, &x->f_selected);

  outlet_new(&x->x_obj, 0);
  return (x);
}

static void mux_free(t_mux *x)
{
  const int count = x->i_count;

  if (x->in && x->x_proxy) {
    int n = 0;
    for (n = 0; n < count; n++) {
      if (x->in[n]) {
        inlet_free(x->in[n]);
      }
      x->in[n] = 0;
      if (x->x_proxy[n]) {
        t_muxproxy *y = x->x_proxy[n];
        y->p_master = 0;
        y->id = 0;
        pd_free(&y->p_pd);
      }
      x->x_proxy[n] = 0;
    }
    freebytes(x->in, x->i_count * sizeof(t_inlet *));
    freebytes(x->x_proxy, x->i_count * sizeof(t_muxproxy *));
  }

  /* pd_free(&y->p_pd); */
}
static t_class *zclass_setup(const char *name)
{
  t_class *c = zexy_new(name, mux_new, mux_free, t_mux, CLASS_NOINLET, "*");
  return c;
}
static void dosetup()
{
  zexy_register("multiplex");
  mux_class = zclass_setup("multiplex");
  zclass_setup("mux");
  muxproxy_class = zexy_new(
      "multiplex proxy", 0, 0, t_muxproxy, CLASS_PD | CLASS_NOINLET, "");
  class_addanything(muxproxy_class, mux_anything);
}
ZEXY_SETUP void multiplex_setup(void)
{
  dosetup();
}
void mux_setup(void)
{
  dosetup();
}
