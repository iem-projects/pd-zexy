#include <stdio.h>

#include "zexy.h"

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* skeleton~ : skeleton-code for a signal-object */

/* ------------------------ skeleton~ ----------------------------- */

static t_class *skeleton_class;

typedef struct _skeleton
{
  t_object x_obj;

} t_skeleton;


static t_int *skeleton_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);
  t_skeleton *x = (t_skeleton *) w[4];

  while (n--) {
    *in++=*out++;
  }

  return (w+5);
}

static void skeleton_dsp(t_skeleton *x, t_signal **sp)
{
  dsp_add(skeleton_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n,x);
}



static void *skeleton_new()
{
  t_skeleton *x = (t_skeleton *)pd_new(skeleton_class);
  outlet_new(&x->x_obj, gensym("signal"));

  return (x);
}

void z_skeleton_tilde_setup(void)
{
  skeleton_class = class_new(gensym("skeleton~"), (t_newmethod)skeleton_new, 0,
			     sizeof(t_skeleton), 0, A_DEFFLOAT, 0);
  class_addmethod(skeleton_class, nullfn, gensym("signal"), 0);
  class_addmethod(skeleton_class, (t_method)skeleton_dsp, gensym("dsp"), 0);
  
  class_sethelpsymbol(skeleton_class, gensym("zexy/skeleton~"));
}

