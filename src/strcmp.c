/*
 *  strcmp    : compare 2 lists as if they were strings
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
#include "zexy_strndup.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------- strcmp ------------------------------- */

/* compare 2 lists ( == for lists) */

static t_class *strcmp_class = NULL;
static t_class *strcmp_proxy_class = NULL;

typedef struct _strcmp {
  t_object x_obj;
  struct _strcmp_proxy *x_proxy;

  t_binbuf *bbuf1, *bbuf2;
  char *str1, *str2;
  int n1, n2;
} t_strcmp;

typedef struct _strcmp_proxy {
  t_pd p_pd;
  t_strcmp *p_master;
  t_inlet *p_in;
} t_strcmp_proxy;

static void strcmp_bang(t_strcmp *x)
{
  int result = 0;
  if (x->str1) {
    if (x->str2) {
      result = strcmp(x->str1, x->str2);
    } else {
      result = *x->str1;
    }
  } else {
    if (x->str2) {
      result = -*x->str2;
    } else {
      result = 0;
    }
  }

  outlet_float(x->x_obj.ob_outlet, result);
}

static void list2binbuf(
    t_binbuf **bbuf, int *n, char **str, int argc, t_atom *argv)
{
  int i = 0;
  char *s = 0;
  if (*str && *n) {
    freebytes(*str, *n);
  }

  binbuf_clear(*bbuf);
  binbuf_add(*bbuf, argc, argv);
  binbuf_gettext(*bbuf, str, n);
  i = *n;
  s = *str;

  if (' ' == s[i]) {
    s[i] = 0;
  }
}

static void strcmp_list(
    t_strcmp *x, t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  list2binbuf(&x->bbuf1, &x->n1, &x->str1, argc, argv);
  strcmp_bang(x);
}
static void strcmp_symbol(t_strcmp *x, t_symbol *s)
{
  if (x->str1 && x->n1) {
    freebytes(x->str1, x->n1);
  }
  x->str1 = zexy_strndup(s->s_name, MAXPDSTRING);
  x->n1 = strnlen(x->str1, MAXPDSTRING);
  strcmp_bang(x);
}

static void strcmp_secondlist(
    t_strcmp *x, t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  list2binbuf(&x->bbuf2, &x->n2, &x->str2, argc, argv);
}
static void strcmp_secondsymbol(t_strcmp *x, t_symbol *s)
{
  if (x->str2 && x->n2) {
    freebytes(x->str2, x->n2);
  }
  x->str2 = zexy_strndup(s->s_name, MAXPDSTRING);
  x->n2 = strnlen(x->str2, MAXPDSTRING);
}

static void strcmp_proxy_list(
    t_strcmp_proxy *y, t_symbol *s, int argc, t_atom *argv)
{
  strcmp_secondlist(y->p_master, s, argc, argv);
}
static void strcmp_proxy_symbol(t_strcmp_proxy *y, t_symbol *s)
{
  if (s) {
    strcmp_secondsymbol(y->p_master, s);
  }
}

static void *strcmp_new(t_symbol *UNUSED(s), int argc, t_atom *argv)
{
  t_strcmp *x = (t_strcmp *)pd_new(strcmp_class);

  x->x_proxy = (t_strcmp_proxy *)pd_new(strcmp_proxy_class);
  x->x_proxy->p_master = x;
  x->x_proxy->p_in = inlet_new((t_object *)x, (t_pd *)x->x_proxy, 0, 0);

  outlet_new(&x->x_obj, 0);

  x->bbuf1 = binbuf_new();
  x->bbuf2 = binbuf_new();

  x->str1 = 0;
  x->str2 = 0;
  x->n1 = 0;
  x->n2 = 0;

  if (argc) {
    strcmp_secondlist(x, gensym("list"), argc, argv);
  }

  return (x);
}

static void strcmp_free(t_strcmp *x)
{
  binbuf_free(x->bbuf1);
  binbuf_free(x->bbuf2);
  if (x->str1 && x->n1) {
    freebytes(x->str1, x->n1);
  }
  if (x->str2 && x->n2) {
    freebytes(x->str2, x->n2);
  }

  inlet_free(x->x_proxy->p_in);
  x->x_proxy->p_master = 0;
  pd_free(&x->x_proxy->p_pd);
}

static void strcmp_help(t_strcmp *UNUSED(x))
{
  post("\n" HEARTSYMBOL " strcmp\t\t:: compare to lists as strings");
}

ZEXY_SETUP void strcmp_setup(void)
{
  strcmp_class =
      zexy_new("strcmp", strcmp_new, strcmp_free, t_strcmp, CLASS_DEFAULT, "*");

  class_addbang(strcmp_class, strcmp_bang);
  class_addsymbol(strcmp_class, strcmp_symbol);
  class_addlist(strcmp_class, strcmp_list);

  strcmp_proxy_class = zexy_new(
      "strcmp proxy", 0, 0, t_strcmp_proxy, CLASS_PD | CLASS_NOINLET, "");
  class_addsymbol(strcmp_proxy_class, strcmp_proxy_symbol);
  class_addlist(strcmp_proxy_class, strcmp_proxy_list);
  zexy_addmethod(strcmp_class, (t_method)strcmp_help, "help", "");
  zexy_register("strcmp");
}
