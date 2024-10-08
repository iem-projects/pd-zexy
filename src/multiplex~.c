/*
 * mux~ : multiplex a specified signal to the output
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

#include "zexy.h"

static t_class *mux_tilde_class = NULL;

typedef struct _mux {
  t_object x_obj;

  int input;

  int n_in;
  t_sample **in;
} t_mux;

static void mux_tilde_input(t_mux *x, t_floatarg f)
{
  if ((f >= 0) && (f < x->n_in)) {
    x->input = f;
  } else {
    pd_error(
        x, "multiplex~: %d is channel out of range (0..%d)", (int)f, x->n_in);
  }
}

static t_int *mux_tilde_perform(t_int *w)
{
  t_mux *x = (t_mux *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);

  t_sample *in = x->in[x->input];

  while (n--) {
    *out++ = *in++;
  }

  return (w + 4);
}

static void mux_tilde_dsp(t_mux *x, t_signal **sp)
{
  int n = 0;
  t_sample **dummy = x->in;

  for (n = 0; n < x->n_in; n++) {
    *dummy++ = sp[n]->s_vec;
  }

  dsp_add(mux_tilde_perform, 3, x, sp[n]->s_vec, sp[0]->s_n);
}

static void mux_tilde_helper(void)
{
  post("\n" HEARTSYMBOL
       " multiplex~\t:: multiplex a one of various signals to one outlet");
  post("<#out>\t : the inlet-number (counting from 0) witch is routed to the "
       "outlet"
       "'help'\t : view this");
  post("creation : \"mux~ [arg1 [arg2...]]\"\t: the number of arguments equals "
       "the number of inlets\n");
}

static void mux_tilde_free(t_mux *x)
{
  freebytes(x->in, x->n_in * sizeof(t_sample *));
}

static void *mux_tilde_new(t_symbol *UNUSED(s), int argc, t_atom *UNUSED(argv))
{
  t_mux *x = (t_mux *)pd_new(mux_tilde_class);
  int i;
  if (!argc) {
    argc = 2;
  }
  x->n_in = argc;
  x->input = 0;

  argc--;
  while (argc--) {
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
  }

  x->in = (t_sample **)getbytes(x->n_in * sizeof(t_sample *));
  i = x->n_in;
  while (i--) {
    x->in[i] = 0;
  }

  outlet_new(&x->x_obj, gensym("signal"));

  return (x);
}
static t_class *zclass_setup(const char *name)
{
  t_class *c =
      zexy_new(name, mux_tilde_new, mux_tilde_free, t_mux, CLASS_DEFAULT, "*");

  /* ouch, that hurts... */
  class_addfloat(c, mux_tilde_input);

  zexy_addmethod(c, (t_method)mux_tilde_dsp, "dsp", "!");
  zexy_addmethod(c, (t_method)nullfn, "signal", "");

  zexy_addmethod(c, (t_method)mux_tilde_helper, "help", "");
  return c;
}
static void dosetup()
{
  zexy_register("multiplex~");
  mux_tilde_class = zclass_setup("multiplex~");
  zclass_setup("mux~");
}
ZEXY_SETUP void multiplex_tilde_setup(void)
{
  dosetup();
}
void mux_tilde_setup(void)
{
  dosetup();
}
