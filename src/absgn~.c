/*
 * absgn~: combined absolute value and sign of a signal
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

#include "zexySIMD.h"

typedef struct _absgn {
  t_object x_obj;
  t_float x_f;
} t_absgn;

/* ------------------------ sigABSGN~ ----------------------------- */

static t_class *sigABSGN_class = NULL;

static t_int *sigABSGN_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  t_sample *out2 = (t_sample *)(w[3]);
  int n = (int)(w[4]);

  while (n--) {
    t_sample val = *in++;
    *out++ = Z_FABS(val);

    if (val > 0.) {
      *out2++ = 1.;
    } else if (val < 0.) {
      *out2++ = -1.;
    } else {
      *out2++ = 0.;
    }
  }

  return (w + 5);
}

#ifdef __SSE__
static int l_bitmask[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
static int l_sgnbitmask[] = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
static t_int *sigABSGN_performSSE(t_int *w)
{
  __m128 *in = (__m128 *)(w[1]);
  __m128 *out1 = (__m128 *)(w[2]);
  __m128 *out2 = (__m128 *)(w[3]);
  int n = (int)(w[4]) >> 3;

  const __m128 bitmask = _mm_loadu_ps((float *)l_bitmask);
  const __m128 sgnmask = _mm_loadu_ps((float *)l_sgnbitmask);
  const __m128 zero = _mm_setzero_ps();
  const __m128 one = _mm_set1_ps(1.f);

  do {
    __m128 val, val2, xmm0, xmm1, xmm2, xmm3;
    val = in[0];
    xmm0 = _mm_cmpneq_ps(val, zero);    /* mask for non-zeros */
    xmm1 = _mm_and_ps(val, sgnmask);    /* sign (without value) */
    xmm0 = _mm_and_ps(xmm0, one);       /* (abs) value: (val==0.f)?0.f:1.f */
    out1[0] = _mm_and_ps(val, bitmask); /* abs: set sign-bit to "+" */
    out2[0] = _mm_or_ps(xmm1, xmm0);    /* merge sign and value */

    val2 = in[1];
    xmm2 = _mm_cmpneq_ps(val2, zero);    /* mask for non-zeros */
    xmm3 = _mm_and_ps(val2, sgnmask);    /* sign (without value) */
    xmm2 = _mm_and_ps(xmm2, one);        /* (abs) value: (val==0.f)?0.f:1.f */
    out1[1] = _mm_and_ps(val2, bitmask); /* abs: set sign-bit to "+" */
    out2[1] = _mm_or_ps(xmm3, xmm2);     /* merge sign and value */

    in += 2;
    out1 += 2;
    out2 += 2;
  } while (--n);

  return (w + 5);
}
#endif /* __SSE__ */

static void sigABSGN_dsp(t_absgn *UNUSED(x), t_signal **sp)
{
#ifdef __SSE__
  if (ZEXY_TYPE_EQUAL(t_sample,
          float) && /*  currently SSE2 code is only for float (not for double) */
      Z_SIMD_CHKBLOCKSIZE(sp[0]->s_n) &&
      Z_SIMD_CHKALIGN(sp[0]->s_vec) && Z_SIMD_CHKALIGN(sp[1]->s_vec) &&
      Z_SIMD_CHKALIGN(sp[2]->s_vec) &&
      zexy_testSSE(sigABSGN_perform, sigABSGN_performSSE, 1, 2)) {
    dsp_add(sigABSGN_performSSE, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
        sp[0]->s_n);
  } else
#endif
  {
    dsp_add(sigABSGN_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
        sp[0]->s_n);
  }
}

static void sigABSGN_helper(void)
{
  post("\n" HEARTSYMBOL " absgn~ \t\t:: absolute value and sign of a signal");
  post("         \t\t   copyright (c) Tim Blechmann 2006");
}

static void *sigABSGN_new(void)
{
  t_absgn *x = (t_absgn *)pd_new(sigABSGN_class);
  x->x_f = 0.f;

  outlet_new(&x->x_obj, gensym("signal"));
  outlet_new(&x->x_obj, gensym("signal"));

  return (x);
}

ZEXY_SETUP void absgn_tilde_setup(void)
{
  sigABSGN_class =
      zexy_new("absgn~", sigABSGN_new, 0, t_absgn, CLASS_DEFAULT, "");
  CLASS_MAINSIGNALIN(sigABSGN_class, t_absgn, x_f);
  zexy_addmethod(sigABSGN_class, (t_method)sigABSGN_dsp, "dsp", "!");

  zexy_addmethod(sigABSGN_class, (t_method)sigABSGN_helper, "help", "");
  class_sethelpsymbol(sigABSGN_class, gensym("zigbinops"));

  zexy_register("absgn~");
}
