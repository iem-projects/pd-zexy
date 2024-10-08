/*
 * [.]: scala multiplication
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

static t_class *scalmul_class = NULL;
static t_class *scalmul_scal_class = NULL;

typedef struct _scalmul {
  t_object x_obj;

  t_int n1, n2;

  t_float *buf1, *buf2;

  t_float f;
} t_scalmul;

static void scalmul_lst2(
    t_scalmul *x, t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  t_float *fp;
  if (x->n2 != argc) {
    freebytes(x->buf2, x->n2 * sizeof(t_float));
    x->n2 = argc;
    x->buf2 = (t_float *)getbytes(sizeof(t_float) * x->n2);
  };
  fp = x->buf2;
  while (argc--) {
    *fp++ = atom_getfloat(argv++);
  }
}

static void scalmul_lst(
    t_scalmul *x, t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  t_float *fp;
  t_atom *ap;
  int n;

  if (argc) {
    if (x->n1 != argc) {
      freebytes(x->buf1, x->n1 * sizeof(t_float));
      x->n1 = argc;
      x->buf1 = (t_float *)getbytes(sizeof(t_float) * x->n1);
    };
    fp = x->buf1;
    while (argc--) {
      *fp++ = atom_getfloat(argv++);
    }
  }

  if (x->n1 * x->n2 == 1) {
    outlet_float(x->x_obj.ob_outlet, *x->buf1 * *x->buf2);
    return;
  }
  if (x->n1 == 1) {
    t_atom *a;
    int i = x->n2;
    t_float f = *x->buf1;
    fp = x->buf2;
    n = x->n2;
    ap = (t_atom *)getbytes(sizeof(t_atom) * n);
    a = ap;
    while (i--) {
      SETFLOAT(a, *fp++ * f);
      a++;
    }
  } else if (x->n2 == 1) {
    t_float f = *x->buf2;
    t_atom *a;
    int i = x->n1;
    n = x->n1;
    ap = (t_atom *)getbytes(sizeof(t_atom) * n);
    a = ap;
    fp = x->buf1;
    while (i--) {
      SETFLOAT(a, *fp++ * f);
      a++;
    }
  } else {
    t_atom *a;
    int i;
    t_float *fp2 = x->buf2;
    fp = x->buf1;
    n = x->n1;
    if (x->n1 != x->n2) {
      post("scalar multiplication: truncating vectors to the same length");
      if (x->n2 < x->n1) {
        n = x->n2;
      }
    }
    ap = (t_atom *)getbytes(sizeof(t_atom) * n);
    a = ap;
    i = n;
    while (i--) {
      SETFLOAT(a, *fp++ * *fp2++);
      a++;
    }
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), n, ap);
  freebytes(ap, sizeof(t_atom) * n);
}
static void scalmul_free(t_scalmul *x)
{
  freebytes(x->buf1, sizeof(t_float) * x->n1);
  freebytes(x->buf2, sizeof(t_float) * x->n2);
}

static void *scalmul_new(t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  t_scalmul *x;
  if (argc - 1) {
    x = (t_scalmul *)pd_new(scalmul_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym(""));
  } else {
    x = (t_scalmul *)pd_new(scalmul_scal_class);
  }

  outlet_new(&x->x_obj, 0);

  x->n1 = 1;
  x->buf1 = (t_float *)getbytes(sizeof(t_float));
  *x->buf1 = 0;

  if (argc) {
    scalmul_lst2(x, gensym("list"), argc, argv);
  } else {
    x->n2 = 1;
    x->buf2 = (t_float *)getbytes(sizeof(t_float));
    *x->buf2 = 0;
  }

  if (argc == 1) {
    floatinlet_new(&x->x_obj, x->buf2);
  }

  return (x);
}

static void scalmul_help(t_scalmul *UNUSED(x))
{
  post("\n" HEARTSYMBOL " .\t\t:: scalar multiplication (in-product)");
}

ZEXY_SETUP void setup_0x2e(void)
{
  scalmul_class =
      zexy_new(".", scalmul_new, scalmul_free, t_scalmul, CLASS_DEFAULT, "*");
  class_addlist(scalmul_class, scalmul_lst);
  zexy_addmethod(scalmul_class, (t_method)scalmul_lst2, "", "*");
  zexy_addmethod(scalmul_class, (t_method)scalmul_help, "help", "");

  scalmul_scal_class =
      zexy_new(".", 0, scalmul_free, t_scalmul, CLASS_DEFAULT, "");
  class_addlist(scalmul_scal_class, scalmul_lst);
  zexy_addmethod(scalmul_scal_class, (t_method)scalmul_help, "help", "");

  class_sethelpsymbol(scalmul_class, gensym("scalarmult"));
  class_sethelpsymbol(scalmul_scal_class, gensym("scalarmult"));
  zexy_register(".");
}

#ifndef ZEXY_LIBRARY
void setup(void)
{
  setup_0x2e();
}
#endif
