/*
	the long waited for swap~-object that does a byte swap
	of course, we unfortunately have to quantize the float-signal to 16bit (to get bytes)

	1110:forum::für::umläute:1999
  */

#include "zexy.h"
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ swap~ ----------------------------- */
#define FLOAT2SHORT 32768.
#define SHORT2FLOAT 1./32768.

static t_class *swap_class;

typedef struct _swap
{
  t_object x_obj;
  int swapper;
} t_swap;

static void swap_float(t_swap *x, t_floatarg f)
{
  x->swapper = (f != 0);
}

static void swap_bang(t_swap *x)
{
  x->swapper ^= 1;
}

static t_int *swap_perform(t_int *w)
{
  t_swap	*x = (t_swap *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);


  if (x->swapper) 
    while (n--) {
      short dummy = FLOAT2SHORT * *in++;
      *out++ = SHORT2FLOAT * (short)( ((dummy & 0xFF) << 8) | ((dummy & 0xFF00) >> 8) );
    }
  else while (n--) *out++ = *in++;
  
  return (w+5);
}

static void swap_dsp(t_swap *x, t_signal **sp)
{
  dsp_add(swap_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void swap_helper(void)
{
  post("\n%c swap~-object for byteswapping a signal", HEARTSYMBOL);
  post("<1/0>  : turn the swapper on/off\n"
       "'bang' : toggle the swapper on/off\n"
       "'help' : view this\n"
       "signal~");
  post("outlet : signal~");
}

static void *swap_new()
{
  t_swap *x = (t_swap *)pd_new(swap_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->swapper = 1;
  return (x);
}

void z_swaptilde_setup(void)
{
  swap_class = class_new(gensym("swap~"), (t_newmethod)swap_new, 0,
			 sizeof(t_swap), 0, A_DEFFLOAT, 0);
  class_addmethod(swap_class, nullfn, gensym("signal"), 0);
  class_addmethod(swap_class, (t_method)swap_dsp, gensym("dsp"), 0);
  
  class_addfloat(swap_class, swap_float);
  class_addbang(swap_class, swap_bang);
  
  class_addmethod(swap_class, (t_method)swap_helper, gensym("help"), 0);
  class_sethelpsymbol(swap_class, gensym("zexy/swap~"));
}

/* ------------------------ blockmirror~ ----------------------------- */

/* mirrors a signalblock around it's center:
   {x[0], x[1], ... x[n-1]} --> {x[n-1], x[n-2], ... x[0]}
*/

static t_class *blockmirror_class;

typedef struct _blockmirror
{
  t_object x_obj;
  int doit;
  int blocksize;
  t_float *blockbuffer;
} t_blockmirror;

static void blockmirror_float(t_blockmirror *x, t_floatarg f)
{
  x->doit = (f != 0);
}

static t_int *blockmirror_perform(t_int *w)
{
  t_blockmirror	*x = (t_blockmirror *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  if (x->doit) {
    if (in==out){
      int N=n;
      t_float *dummy=x->blockbuffer;
      while(n--)*dummy++=*in++;
      dummy--;
      while(N--)*out++=*dummy--;
    } else {
      in+=n-1;
      while(n--)*out++=*in--;
    }
  } else while (n--) *out++ = *in++;
  return (w+5);
}

static void blockmirror_dsp(t_blockmirror *x, t_signal **sp)
{
  if (x->blocksize<sp[0]->s_n){
    if(x->blockbuffer)freebytes(x->blockbuffer, sizeof(t_float)*x->blocksize);
    x->blocksize = sp[0]->s_n;
    x->blockbuffer = getbytes(sizeof(t_float)*x->blocksize);
  }
  dsp_add(blockmirror_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void blockmirror_helper(void)
{
  post("\n%c blockmirror~-object for reverting a signal", HEARTSYMBOL);
  post("'help' : view this\n"
       "signal~");
  post("outlet : signal~");
}

static void *blockmirror_new()
{
  t_blockmirror *x = (t_blockmirror *)pd_new(blockmirror_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->doit = 1;
  x->blocksize=0;
  return (x);
}

void blockmirror_setup(void)
{
  blockmirror_class = class_new(gensym("blockmirror~"), (t_newmethod)blockmirror_new, 0,
			 sizeof(t_blockmirror), 0, A_DEFFLOAT, 0);
  class_addmethod(blockmirror_class, nullfn, gensym("signal"), 0);
  class_addmethod(blockmirror_class, (t_method)blockmirror_dsp, gensym("dsp"), 0);
  
  class_addfloat(blockmirror_class, blockmirror_float);
  
  class_addmethod(blockmirror_class, (t_method)blockmirror_helper, gensym("help"), 0);
  class_sethelpsymbol(blockmirror_class, gensym("zexy/blockmirror~"));
}

/* ------------------------ blockswap~ ----------------------------- */

/* swaps a signalblock around it's center:
   {x[0], x[1], ... x[n-1]} --> {x[n-1], x[n-2], ... x[0]}
*/

static t_class *blockswap_class;

typedef struct _blockswap
{
  t_object x_obj;
  int doit;
  int blocksize;
  t_float *blockbuffer;
} t_blockswap;

static void blockswap_float(t_blockswap *x, t_floatarg f)
{
  x->doit = (f != 0);
}

static t_int *blockswap_perform(t_int *w)
{
  t_blockswap	*x = (t_blockswap *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int N = (int)(w[4]);
  int N2=N/2;
  if (x->doit) {
    int n=N2;
    t_float *dummy=x->blockbuffer;
    while(n--)*dummy++=*in++;
    n=N-N2;
    while(n--)*out++=*in++;
    dummy=x->blockbuffer;
    n=N2;    
    while(n--)*out++=*dummy++;
  } else while (N--) *out++=*in++;
  return (w+5);
}

static void blockswap_dsp(t_blockswap *x, t_signal **sp)
{
  if (x->blocksize*2<sp[0]->s_n){
    if(x->blockbuffer)freebytes(x->blockbuffer, sizeof(t_float)*x->blocksize);
    x->blocksize = sp[0]->s_n/2;
    x->blockbuffer = getbytes(sizeof(t_float)*x->blocksize);
  }
  dsp_add(blockswap_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void blockswap_helper(void)
{
  post("\n%c blockswap~-object for blockwise-swapping of a signal ", HEARTSYMBOL);
  post("'help' : view this\n"
       "signal~");
  post("outlet : signal~");
}

static void *blockswap_new()
{
  t_blockswap *x = (t_blockswap *)pd_new(blockswap_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->doit = 1;
  x->blocksize=0;
  return (x);
}

void blockswap_setup(void)
{
  blockswap_class = class_new(gensym("blockswap~"), (t_newmethod)blockswap_new, 0,
			 sizeof(t_blockswap), 0, A_DEFFLOAT, 0);
  class_addmethod(blockswap_class, nullfn, gensym("signal"), 0);
  class_addmethod(blockswap_class, (t_method)blockswap_dsp, gensym("dsp"), 0);
  
  class_addfloat(blockswap_class, blockswap_float);
  
  class_addmethod(blockswap_class, (t_method)blockswap_helper, gensym("help"), 0);
  class_sethelpsymbol(blockswap_class, gensym("zexy/blockswap~"));
}


/* ----------------------------------------------------------------- */

void z_swap_setup(void){
  z_swaptilde_setup();
  blockswap_setup();
  blockmirror_setup();
}

