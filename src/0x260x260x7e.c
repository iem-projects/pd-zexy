/*
 * &&~: logical AND for signals
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

/* ------------------------ logical~ ----------------------------- */

/* ----------------------------- andand_tilde ----------------------------- */
static t_class *andand_tilde_class = NULL, *scalarandand_tilde_class = NULL;

typedef struct _andand_tilde {
  t_object x_obj;
  t_float x_f;
} t_andand_tilde;

typedef struct _scalarandand_tilde {
  t_object x_obj;
  t_float x_f;
  t_float x_g; /* inlet value */
} t_scalarandand_tilde;

static void *andand_tilde_new(t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  if (argc > 1) {
    post("&&~: extra arguments ignored");
  }
  if (argc) {
    t_scalarandand_tilde *x =
        (t_scalarandand_tilde *)pd_new(scalarandand_tilde_class);
    floatinlet_new(&x->x_obj, &x->x_g);
    x->x_g = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
  } else {
    t_andand_tilde *x = (t_andand_tilde *)pd_new(andand_tilde_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
  }
}

static t_int *andand_tilde_perform(t_int *w)
{
  t_sample *in1 = (t_sample *)(w[1]);
  t_sample *in2 = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);
  while (n--) {
    int f = (int)*in1++;
    int g = (int)*in2++;
    *out++ = (f && g);
  }
  return (w + 5);
}

static t_int *andand_tilde_perf8(t_int *w)
{
  t_sample *in1 = (t_sample *)(w[1]);
  t_sample *in2 = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in1 += 8, in2 += 8, out += 8) {
    int f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
    int f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

    int g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
    int g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];

    out[0] = f0 && g0;
    out[1] = f1 && g1;
    out[2] = f2 && g2;
    out[3] = f3 && g3;
    out[4] = f4 && g4;
    out[5] = f5 && g5;
    out[6] = f6 && g6;
    out[7] = f7 && g7;
  }
  return (w + 5);
}

static t_int *scalarandand_tilde_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  int f = *(t_float *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);
  while (n--) {
    *out++ = (int)*in++ && f;
  }
  return (w + 5);
}

static t_int *scalarandand_tilde_perf8(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  int g = *(t_float *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in += 8, out += 8) {
    int f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
    int f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

    out[0] = f0 && g;
    out[1] = f1 && g;
    out[2] = f2 && g;
    out[3] = f3 && g;
    out[4] = f4 && g;
    out[5] = f5 && g;
    out[6] = f6 && g;
    out[7] = f7 && g;
  }
  return (w + 5);
}

#ifdef __SSE__
static int l_bitmask[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};

static t_int *andand_tilde_performSSE(t_int *w)
{
  __m128 *in1 = (__m128 *)(w[1]);
  __m128 *in2 = (__m128 *)(w[2]);
  __m128 *out = (__m128 *)(w[3]);
  int n = (int)(w[4]) >> 4;

  const __m128 bitmask =
      _mm_loadu_ps((float *)l_bitmask); /* for getting the absolute value */
  const __m128 one = _mm_set1_ps(1.f);

  while (n--) {
    __m128 xmm0, xmm1, xmm2;
    xmm0 = _mm_and_ps(in1[0], bitmask); /* =abs(f); */
    xmm1 = _mm_and_ps(in2[0], bitmask);
    xmm0 = _mm_cmpge_ps(
        xmm0, one); /* =(abs(f)>=1.0)=i (a weird cast to integer) */
    xmm1 = _mm_cmpge_ps(xmm1, one);
    xmm2 = _mm_and_ps(xmm0, xmm1);  /* =(i0&&i1) */
    out[0] = _mm_and_ps(xmm2, one); /* 0xfffffff -> 1.0 */

    xmm0 = _mm_and_ps(in1[1], bitmask);
    xmm1 = _mm_and_ps(in2[1], bitmask);
    xmm0 = _mm_cmpge_ps(xmm0, one);
    xmm1 = _mm_cmpge_ps(xmm1, one);
    xmm2 = _mm_and_ps(xmm0, xmm1);
    out[1] = _mm_and_ps(xmm2, one);

    xmm0 = _mm_and_ps(in1[2], bitmask);
    xmm1 = _mm_and_ps(in2[2], bitmask);
    xmm0 = _mm_cmpge_ps(xmm0, one);
    xmm1 = _mm_cmpge_ps(xmm1, one);
    xmm2 = _mm_and_ps(xmm0, xmm1);
    out[2] = _mm_and_ps(xmm2, one);

    xmm0 = _mm_and_ps(in1[3], bitmask);
    xmm1 = _mm_and_ps(in2[3], bitmask);
    xmm0 = _mm_cmpge_ps(xmm0, one);
    xmm1 = _mm_cmpge_ps(xmm1, one);
    xmm2 = _mm_and_ps(xmm0, xmm1);
    out[3] = _mm_and_ps(xmm2, one);

    in1 += 4;
    in2 += 4;
    out += 4;
  }

  return (w + 5);
}
static t_int *scalarandand_tilde_performSSE(t_int *w)
{
  __m128 *in = (__m128 *)(w[1]);
  __m128 *out = (__m128 *)(w[3]);
  float f = *(t_float *)(w[2]);
  __m128 scalar = _mm_set1_ps(f);
  int n = (int)(w[4]) >> 4;

  const __m128 bitmask =
      _mm_loadu_ps((float *)l_bitmask); /* for getting the absolute value */
  const __m128 one = _mm_set1_ps(1.f);

  scalar = _mm_and_ps(scalar, bitmask);
  scalar = _mm_cmpge_ps(scalar, one);

  while (n--) {
    __m128 xmm0, xmm1;
    xmm0 = _mm_and_ps(in[0], bitmask); /* =abs(f); */
    xmm0 = _mm_cmpge_ps(
        xmm0, one); /* =(abs(f)>=1.0)=i (a weird cast to integer) */
    xmm0 = _mm_and_ps(xmm0, scalar); /* =(i0&&i1) */
    out[0] = _mm_and_ps(xmm0, one);  /* 0xfffffff -> 1.0 */

    xmm1 = _mm_and_ps(in[1], bitmask);
    xmm1 = _mm_cmpge_ps(xmm1, one);
    xmm1 = _mm_and_ps(xmm1, scalar);
    out[1] = _mm_and_ps(xmm1, one);

    xmm0 = _mm_and_ps(in[2], bitmask);
    xmm0 = _mm_cmpge_ps(xmm0, one);
    xmm0 = _mm_and_ps(xmm0, scalar);
    out[2] = _mm_and_ps(xmm0, one);

    xmm1 = _mm_and_ps(in[3], bitmask);
    xmm1 = _mm_cmpge_ps(xmm1, one);
    xmm1 = _mm_and_ps(xmm1, scalar);
    out[3] = _mm_and_ps(xmm1, one);

    in += 4;
    out += 4;
  }
  return (w + 5);
}
#endif /* __SSE__ */

static void andand_tilde_dsp(t_andand_tilde *UNUSED(x), t_signal **sp)
{
  t_sample *in1 = sp[0]->s_vec;
  t_sample *in2 = sp[1]->s_vec;
  t_sample *out = sp[2]->s_vec;

  int n = sp[0]->s_n;

#ifdef __SSE__
  if (Z_SIMD_CHKBLOCKSIZE(n) && Z_SIMD_CHKALIGN(in1) && Z_SIMD_CHKALIGN(in2) &&
      Z_SIMD_CHKALIGN(out) && ZEXY_TYPE_EQUAL(t_sample, float)) {
    dsp_add(andand_tilde_performSSE, 4, in1, in2, out, n);
  } else
#endif
      if (n & 7) {
    dsp_add(andand_tilde_perform, 4, in1, in2, out, n);
  } else {
    dsp_add(andand_tilde_perf8, 4, in1, in2, out, n);
  }
}

static void scalarandand_tilde_dsp(t_scalarandand_tilde *x, t_signal **sp)
{
  t_sample *in = sp[0]->s_vec;
  t_sample *out = sp[1]->s_vec;
  int n = sp[0]->s_n;

#ifdef __SSE__
  if (Z_SIMD_CHKBLOCKSIZE(n) && Z_SIMD_CHKALIGN(in) && Z_SIMD_CHKALIGN(out) &&
      ZEXY_TYPE_EQUAL(t_sample, float)) {
    dsp_add(scalarandand_tilde_performSSE, 4, in, &x->x_g, out, n);
  } else
#endif
      if (n & 7) {
    dsp_add(scalarandand_tilde_perform, 4, in, &x->x_g, out, n);
  } else {
    dsp_add(scalarandand_tilde_perf8, 4, in, &x->x_g, out, n);
  }
}

static void andand_tilde_help(t_object *UNUSED(x))
{
  post("\n" HEARTSYMBOL " &&~\t\t:: logical AND operation on 2 signals");
}

ZEXY_SETUP void setup_0x260x260x7e(void)
{
  andand_tilde_class =
      zexy_new("&&~", andand_tilde_new, 0, t_andand_tilde, CLASS_DEFAULT, "*");
  zexy_addmethod(andand_tilde_class, (t_method)andand_tilde_dsp, "dsp", "!");
  CLASS_MAINSIGNALIN(andand_tilde_class, t_andand_tilde, x_f);
  zexy_addmethod(andand_tilde_class, (t_method)andand_tilde_help, "help", "");
  class_sethelpsymbol(andand_tilde_class, gensym("zigbinops"));

  scalarandand_tilde_class =
      zexy_new("&&~", 0, 0, t_scalarandand_tilde, CLASS_DEFAULT, "");
  CLASS_MAINSIGNALIN(scalarandand_tilde_class, t_scalarandand_tilde, x_f);
  zexy_addmethod(
      scalarandand_tilde_class, (t_method)scalarandand_tilde_dsp, "dsp", "!");
  zexy_addmethod(
      scalarandand_tilde_class, (t_method)andand_tilde_help, "help", "");
  class_sethelpsymbol(scalarandand_tilde_class, gensym("zigbinops"));

  zexy_register("&&~");
}

#ifndef ZEXY_LIBRARY
void setup(void)
{
  setup_0x260x260x7e();
}
#endif
