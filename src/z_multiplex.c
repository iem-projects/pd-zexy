
/* 1509:forum::für::umläute:2000 */

/*
  demux :  multiplex the input to a specified output  
to do::  mux   :  demultiplex a specified input to the output
*/

#include "zexy.h"
#include <stdio.h>

/* ------------------------- demux ------------------------------- */

/*
  a demultiplexer
*/

static t_class *demux_class;

typedef struct _demux
{
  t_object x_obj;

  int n_out;
  t_outlet **out, *selected;


} t_demux;

static void demux_select(t_demux *x, t_float f)
{
  int n = ( (f<0) || (f>x->n_out) ) ? 0 : f;
  x->selected = x->out[n];
}

static void demux_list(t_demux *x, t_symbol *s, int argc, t_atom *argv)
{
  switch (argc) {
  case 0:
    outlet_bang(x->selected);
    break;
  case 1:
    switch (argv->a_type) {
    case A_FLOAT:
      outlet_float(x->selected, atom_getfloat(argv));
      break;
    case A_SYMBOL:
      outlet_symbol(x->selected, atom_getsymbol(argv));
      break;
    case A_POINTER:
       outlet_pointer(x->selected, argv->a_w.w_gpointer);
       break;
    default:
      outlet_list(x->selected, s, argc, argv);
    }
    break;
  default:
    outlet_list(x->selected, s, argc, argv);
  }
}
static void demux_any(t_demux *x, t_symbol *s, int argc, t_atom *argv)
{
  outlet_anything(x->selected, s, argc, argv);
}

static void *demux_new(t_symbol *s, int argc, t_atom *argv)
{
  t_demux *x = (t_demux *)pd_new(demux_class);
  int n = (argc < 2)?2:argc;

  x->n_out = n - 1;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("select"));
  x->out = (t_outlet **)getbytes(n * sizeof(t_outlet *));

  for (n=0; n<=x->n_out; n++) {
    x->out[n] = outlet_new(&x->x_obj, 0);
  }

  x->selected = x->out[0];

  return (x);
}

static void demux_setup(void)
{
  demux_class = class_new(gensym("demultiplex"), (t_newmethod)demux_new,
			      0, sizeof(t_demux), 0, A_GIMME,  0);
  class_addcreator((t_newmethod)demux_new, gensym("demux"), A_GIMME, 0);
  
  class_addanything (demux_class, demux_any);
  class_addlist     (demux_class, demux_list);

  class_addmethod   (demux_class, (t_method)demux_select, gensym("select"), A_DEFFLOAT, 0);

  class_sethelpsymbol(demux_class, gensym("zexy/demultiplex"));
}


#ifdef MUX
/* ------------------------- mux ------------------------------- */

/*
  a multiplexer
*/

static t_class *mux_class;

typedef struct _mux
{
  t_object x_obj;

  int n_in;
  t_inlet **in, *selected;
} t_mux;

static void mux_select(t_mux *x, t_float f)
{
  int n = ( (f<0) || (f>x->n_in) ) ? 0 : f;
}

static void mux_incoming(t_mux *x, t_symbol *s, int argc, t_atom *argv)
{
  error("symbol @ %x", s);
}

static void *mux_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mux *x = (t_mux *)pd_new(mux_class);

  int n = (argc < 2)?2:argc;

  x->n_in = n;
  x->in = (t_inlet **)getbytes(x->n_in * sizeof(t_inlet *));

  for (n = 0; n<x->n_in; n++) {
    char name[8];
    int i = 8;

    while (i--) name[i]=0;

    sprintf(name, "inlet%d", n);

    x->in[n] = inlet_new        (&x->x_obj, &x->x_obj.ob_pd, &s_list, gensym(name));
    class_addmethod (mux_class, (t_method)mux_incoming, gensym(name), A_GIMME, 0);
  }
  

  outlet_new(&x->x_obj, 0);
  return (x);
}

static void mux_setup(void)
{
  mux_class = class_new(gensym("multiplex"), (t_newmethod)mux_new,
			      0, sizeof(t_mux), 0, A_GIMME,  0);
  class_addcreator((t_newmethod)mux_new, gensym("mux"), A_GIMME, 0);
 
  class_addfloat   (mux_class, (t_method)mux_select);

  class_sethelpsymbol(mux_class, gensym("zexy/multiplex"));
}
#endif

void z_multiplex_setup(void)
{
  demux_setup();
#ifdef MUX
  mux_setup();
#endif
}
