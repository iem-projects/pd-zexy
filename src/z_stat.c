#include "zexy.h"
#include <math.h>

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#define sqrtf sqrt
#endif

/* mean :: the mean of a list of floats */

static t_class *mean_class;

typedef struct _mean
{
  t_object x_obj;
} t_mean;

static void mean_list(t_mean *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float factor = 1./argc;
  t_float sum = 0;

  while(argc--)sum+=atom_getfloat(argv++);

  outlet_float(x->x_obj.ob_outlet,sum*factor);
}

static void *mean_new(void)
{
  t_mean *x = (t_mean *)pd_new(mean_class);

  outlet_new(&x->x_obj, gensym("float"));

  return (x);
}

static void mean_help(void)
{
  post("mean\t:: calculate the mean of a list of floats");
}

static void mean_setup(void)
{
  mean_class = class_new(gensym("mean"), (t_newmethod)mean_new, 0,
			 sizeof(t_mean), 0, A_DEFFLOAT, 0);

  class_addlist(mean_class, (t_method)mean_list);
  class_addmethod(mean_class, (t_method)mean_help, gensym("help"), 0);

  class_sethelpsymbol(mean_class, gensym("zexy/mean"));
}

/* minmax :: get minimum and maximum of a list */

static t_class *minmax_class;

typedef struct _minmax
{
  t_object x_obj;
  t_float min;
  t_float max;

  t_outlet *mino, *maxo;
} t_minmax;

static void minmax_bang(t_minmax *x)
{
  outlet_float(x->maxo,x->max);
  outlet_float(x->mino,x->min);
}

static void minmax_list(t_minmax *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float min = atom_getfloat(argv++);
  t_float max=min;
  argc--;

  while(argc--){
    t_float f = atom_getfloat(argv++);
    if (f<min)min=f;
    else if (f>max)max=f;
  }

  x->min=min;
  x->max=max;

  minmax_bang(x);
}

static void *minmax_new(void)
{
  t_minmax *x = (t_minmax *)pd_new(minmax_class);

  x->mino=outlet_new(&x->x_obj, gensym("float"));
  x->maxo=outlet_new(&x->x_obj, gensym("float"));

  x->min = x->max = 0;

  return (x);
}

static void minmax_help(void)
{
  post("minmax\t:: get minimum and maximum of a list of floats");
}

static void minmax_setup(void)
{
  minmax_class = class_new(gensym("minmax"), (t_newmethod)minmax_new, 0,
			 sizeof(t_minmax), 0, A_DEFFLOAT, 0);

  class_addlist(minmax_class, (t_method)minmax_list);
  class_addbang(minmax_class, (t_method)minmax_bang);
  class_addmethod(minmax_class, (t_method)minmax_help, gensym("help"), 0);

  class_sethelpsymbol(minmax_class, gensym("zexy/minmax"));
}

/* length :: get minimum and maximum of a list */

static t_class *length_class;

typedef struct _length
{
  t_object x_obj;
} t_length;

static void length_list(t_length *x, t_symbol *s, int argc, t_atom *argv)
{
  outlet_float(x->x_obj.ob_outlet, (t_float)argc);
}
static void length_any(t_length *x, t_symbol *s, int argc, t_atom *argv)
{
  outlet_float(x->x_obj.ob_outlet, (t_float)argc+1);
}

static void *length_new(void)
{
  t_length *x = (t_length *)pd_new(length_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

static void length_setup(void)
{
  length_class = class_new(gensym("length"), (t_newmethod)length_new, 0,
			 sizeof(t_length), 0, A_DEFFLOAT, 0);

  class_addlist(length_class, (t_method)length_list);
  class_addanything(length_class, (t_method)length_any);
  //  class_addbang(length_class, (t_method)length_bang);

  class_sethelpsymbol(length_class, gensym("zexy/length"));
}

/* global setup routine */

void z_stat_setup(void)
{
  mean_setup();
  minmax_setup();
  length_setup();
}
