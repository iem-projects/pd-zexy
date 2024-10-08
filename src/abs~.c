/*
 * abs~: absolute value for signals
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

typedef struct _abs {
  t_object x_obj;
  t_float x_f;
} t_abs;

/* ------------------------ sigABS~ ----------------------------- */

static t_class *sigABS_class = NULL;

static t_int *sigABS_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);

  while (n--) {
    *out++ = Z_FABS(*in++);
  }

  return (w + 4);
}

#ifdef __SSE__
static int l_bitmask[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
static t_int *sigABS_performSSE(t_int *w)
{
  __m128 *in = (__m128 *)(w[1]);
  __m128 *out = (__m128 *)(w[2]);
  int n = (int)(w[3]) >> 4;

  __m128 bitmask = _mm_loadu_ps((float *)l_bitmask);

  while (n--) {
    out[0] = _mm_and_ps(in[0], bitmask);
    out[1] = _mm_and_ps(in[1], bitmask);
    out[2] = _mm_and_ps(in[2], bitmask);
    out[3] = _mm_and_ps(in[3], bitmask);

    in += 4;
    out += 4;
  }
#  if 0
  /*
   * handwritten SSE-code by tim blechmann
   *
   * JMZ: the above (using intrinsics) is a little bit slower
   * but still about 4* as fast as the generic code
   * i prefer using intrinsics as i don't have to learn how to
   * assembler
   */
  asm(
    ".section .rodata         \n"
    ".align   16              \n"
    "2:                       \n"
    ".long    2147483647      \n" /* bitmask */
    ".long    2147483647      \n" /* 0x7fffffff */
    ".long    2147483647      \n"
    ".long    2147483647      \n"

    ".text                    \n"

    "movaps  (2b), %%xmm0     \n" /* xmm0 = bitmask */
    "shrl    $4, %2           \n"

    /* loop: *dest = abs(*src) */
    "1:                       \n"
    "movaps  (%0,%3), %%xmm1  \n"
    "andps   %%xmm0, %%xmm1   \n"
    "movaps  %%xmm1, (%1,%3)  \n"

    "movaps  16(%0,%3), %%xmm2\n"
    "andps   %%xmm0, %%xmm2   \n"
    "movaps  %%xmm2, 16(%1,%3)\n"

    "movaps  32(%0,%3), %%xmm3\n"
    "andps   %%xmm0, %%xmm3   \n"
    "movaps  %%xmm3, 32(%1,%3)\n"

    "movaps  48(%0,%3), %%xmm4\n"
    "andps   %%xmm0, %%xmm4   \n"
    "movaps  %%xmm4, 48(%1,%3)\n"

    "addl    $64, %3          \n"
    "loop    1b               \n"
    :
    :"r"(in), "r"(out), "c"(n), "r"(0)
    :"%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4"
  );
#  endif /*0*/

  return (w + 4);
}
#endif /* __SSE__ */

static void sigABS_dsp(t_abs *UNUSED(x), t_signal **sp)
{
#ifdef __SSE__
  if (Z_SIMD_CHKBLOCKSIZE(sp[0]->s_n) && Z_SIMD_CHKALIGN(sp[0]->s_vec) &&
      Z_SIMD_CHKALIGN(sp[1]->s_vec) && ZEXY_TYPE_EQUAL(t_sample, float) &&
      zexy_testSSE(sigABS_perform, sigABS_performSSE, 1, 1)) {
    dsp_add(sigABS_performSSE, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
  } else
#endif
  {
    dsp_add(sigABS_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
  }
}

static void sigABS_helper(void)
{
  post("\n" HEARTSYMBOL " abs~ \t\t:: absolute value of a signal");
}

static void *sigABS_new(void)
{
  t_abs *x = (t_abs *)pd_new(sigABS_class);
  x->x_f = 0.f;
  outlet_new(&x->x_obj, gensym("signal"));

  return (x);
}

ZEXY_SETUP void abs_tilde_setup(void)
{
  sigABS_class = zexy_new("abs~", sigABS_new, 0, t_abs, CLASS_DEFAULT, "");
  CLASS_MAINSIGNALIN(sigABS_class, t_abs, x_f);
  zexy_addmethod(sigABS_class, (t_method)sigABS_dsp, "dsp", "!");

  zexy_addmethod(sigABS_class, (t_method)sigABS_helper, "help", "");
  class_sethelpsymbol(sigABS_class, gensym("zigbinops"));

  zexy_register("abs~");
}
