#include <stdio.h>

#include "zexy.h"

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* down~ : down-code for a signal-object */

/* ------------------------ down~ ----------------------------- */

static t_class *down_class;

typedef struct _down
{
  t_object x_obj;

  t_int old_n;
  t_int new_n;

  t_int factor;
  t_int wantedfactor;

  t_float *buffer;
} t_down;

static t_int *down_perform_inplace(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_down *x = (t_down *) w[3];


  t_int factor = x->factor;
  int n = x->new_n;
  t_float *buf = x->buffer;

  while(n--) {
    *buf++=*in;
    in+=factor;
  }

  buf = x->buffer;
  n=x->new_n;

  while(n--){
    *out++=*buf++;
  }

  return (w+4);
}
static t_int *down_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_down *x = (t_down *) w[3];

  t_int factor = x->factor;
  int n = x->new_n;

  while(n--) {
    *out++=*in;
    in+=factor;
  }

  return (w+4);
}

static void down_dsp(t_down *x, t_signal **sp)
{
  t_float f = sp[0]->s_n/(t_float)x->wantedfactor;

  if (f != (int)f) { 
    int faktor = x->wantedfactor;
    while ((f = sp[0]->s_n/(t_float)faktor) != (int)f) faktor--;

    error("bad downsampling factor %d, setting to %d", x->wantedfactor, faktor);
    x->factor = faktor;
  } else x->factor=x->wantedfactor;


  freebytes(x->buffer, sizeof(t_float)*x->new_n);

  x->old_n=sp[0]->s_n;
  x->new_n=sp[0]->s_n/x->factor;

  sp[0]->s_n=x->new_n;

  x->buffer = getbytes (sizeof(t_float)*x->new_n);

  if (sp[0]->s_vec!=sp[1]->s_vec)dsp_add(down_perform, 3, sp[0]->s_vec, sp[1]->s_vec, x);
  else dsp_add(down_perform_inplace, 3, sp[0]->s_vec, sp[1]->s_vec, x);
}

static void *down_new(t_floatarg f)
{
  t_down *x = (t_down *)pd_new(down_class);
  outlet_new(&x->x_obj, gensym("signal"));

  x->wantedfactor=f;
  if (x->wantedfactor<1)x->wantedfactor=1;

  return (x);
}
static void down_setup(void)
{
  down_class = class_new(gensym("down~"), (t_newmethod)down_new, 0,
			     sizeof(t_down), 0, A_DEFFLOAT, 0);
  class_addmethod(down_class, nullfn, gensym("signal"), 0);
  class_addmethod(down_class, (t_method)down_dsp, gensym("dsp"), 0);
  
  class_sethelpsymbol(down_class, gensym("zexy/down~"));
}

void z_down_setup(void)
{
  down_setup();
}

