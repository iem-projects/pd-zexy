/******************************************************
 *
 * zexy - implementation file
 *
 * (c) 2006 Tim Blechmann
 *
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

#include "zexy.h"

typedef struct _absgn
{
  t_object x_obj;
  float x_f;
} t_absgn;


/* ------------------------ sigABSGN~ ----------------------------- */

static t_class *sigABSGN_class;

static t_int *sigABSGN_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_float *out2 = (t_float *)(w[3]);
  int n = (int)(w[4]);
  
  while (n--)
  {
      t_float val = *in++;
      *out++ = fabsf(val);

      if (val>0.) *out2++=1.;
      else if (val<0.) *out2++=-1.;
      else *out2++=0.;
  }

  
  return (w+5);
}

#ifdef __SSE__
static long l_bitmask[]   ={0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
static long l_sgnbitmask[]={0x80000000, 0x80000000, 0x80000000, 0x80000000};
static t_int *sigABSGN_performSSE(t_int *w)
{
  __m128 *in = (__m128 *)(w[1]);
  __m128 *out1 = (__m128 *)(w[2]);
  __m128 *out2 = (__m128 *)(w[3]);
  int n = (int)(w[4])>>3;
  
  const __m128 bitmask= _mm_loadu_ps((float*)l_bitmask);
  const __m128 sgnmask= _mm_loadu_ps((float*)l_sgnbitmask);
  const __m128 zero   = _mm_setzero_ps();
  const __m128 one    = _mm_set1_ps(1.f);
  
  do {
    __m128 val, val2, xmm0, xmm1, xmm2, xmm3;  
    val=in[0];
    xmm0   = _mm_cmpneq_ps(val, zero);   /* mask for non-zeros */
    xmm1   = _mm_and_ps   (val, sgnmask);/* sign (without value) */
    xmm0   = _mm_and_ps   (xmm0, one);   /* (abs) value: (val==0.f)?0.f:1.f */
    out1[0]= _mm_and_ps   (val, bitmask);/* abs: set sign-bit to "+" */
    out2[0]= _mm_or_ps    (xmm1, xmm0);  /* merge sign and value */

    val2=in[1];
    xmm2   = _mm_cmpneq_ps(val2, zero);   /* mask for non-zeros */
    xmm3   = _mm_and_ps   (val2, sgnmask);/* sign (without value) */
    xmm2   = _mm_and_ps   (xmm2, one);    /* (abs) value: (val==0.f)?0.f:1.f */
    out1[1]= _mm_and_ps   (val2, bitmask);/* abs: set sign-bit to "+" */
    out2[1]= _mm_or_ps    (xmm3, xmm2);   /* merge sign and value */

    in  +=2;
    out1+=2;
    out2+=2;
  }
  while (--n);

  return (w+5);
}
#endif /* __SSE__ */

static void sigABSGN_dsp(t_absgn *x, t_signal **sp)
{
#ifdef __SSE__
  if(
     Z_SIMD_CHKBLOCKSIZE(sp[0]->s_n)&&
     Z_SIMD_CHKALIGN(sp[0]->s_vec)&&
     Z_SIMD_CHKALIGN(sp[1]->s_vec)&&
     Z_SIMD_CHKALIGN(sp[2]->s_vec)
     )
    {
      dsp_add(sigABSGN_performSSE, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
    } else
#endif
    {
      dsp_add(sigABSGN_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
    }
}

static void sigABSGN_helper(void)
{
  post("\n%c absgn~ \t\t:: absolute value and sign of a signal", HEARTSYMBOL);
  post("         \t\t   copyright (c) Tim Blechmann 2006");
}

static void *sigABSGN_new(void)
{
  t_absgn *x = (t_absgn *)pd_new(sigABSGN_class);
  x->x_f=0.f;

  outlet_new(&x->x_obj, gensym("signal"));
  outlet_new(&x->x_obj, gensym("signal"));

  return (x);
}

void absgn_tilde_setup(void)
{
  sigABSGN_class = class_new(gensym("absgn~"), (t_newmethod)sigABSGN_new, 0,
			   sizeof(t_absgn), 0, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(sigABSGN_class, t_absgn, x_f);
  class_addmethod(sigABSGN_class, (t_method)sigABSGN_dsp, gensym("dsp"), 0);
  
  class_addmethod(sigABSGN_class, (t_method)sigABSGN_helper, gensym("help"), 0);
  class_sethelpsymbol(sigABSGN_class, gensym("zexy/sigbinops+"));

  zexy_register("absgn~");
}

void z_absgn__setup(void)
{
  absgn_tilde_setup();
}
