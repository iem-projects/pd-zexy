/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   1999:forum::für::umläute:2004
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

#include "zexy.h"
#include <stdlib.h>
#include <string.h>

/*
 * strcmp    : compare 2 lists as if they were strings
*/

/* ------------------------- strcmp ------------------------------- */

/* compare 2 lists ( == for lists) */

static t_class *strcmp_class;

typedef struct _strcmp
{
  t_object x_obj;

  t_binbuf *bbuf1, *bbuf2;
} t_strcmp;

static void strcmp_bang(t_strcmp *x)
{
  char *str1=0, *str2=0;
  int n1=0, n2=0;
  int result = 0;

  binbuf_gettext(x->bbuf1, &str1, &n1);
  binbuf_gettext(x->bbuf2, &str2, &n2);

  result = strcmp(str1, str2);

  freebytes(str1, n1);
  freebytes(str2, n2);

  outlet_float(x->x_obj.ob_outlet, result);
}

static void strcmp_secondlist(t_strcmp *x, t_symbol *s, int argc, t_atom *argv)
{
  binbuf_clear(x->bbuf2);
  binbuf_add(x->bbuf2, argc, argv);
}

static void strcmp_list(t_strcmp *x, t_symbol *s, int argc, t_atom *argv)
{
  binbuf_clear(x->bbuf1);
  binbuf_add(x->bbuf1, argc, argv);
  
  strcmp_bang(x);
}

static void *strcmp_new(t_symbol *s, int argc, t_atom *argv)
{
  t_strcmp *x = (t_strcmp *)pd_new(strcmp_class);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("lst2"));

  x->bbuf1 = binbuf_new();
  x->bbuf2 = binbuf_new();


  strcmp_secondlist(x, gensym("list"), argc, argv);

  return (x);
}

static void strcmp_free(t_strcmp *x)
{
  binbuf_free(x->bbuf1);
  binbuf_free(x->bbuf2);
}


void strcmp_setup(void)
{
  strcmp_class = class_new(gensym("strcmp"), (t_newmethod)strcmp_new, 
			 (t_method)strcmp_free, sizeof(t_strcmp), 0, A_GIMME, 0);

  class_addbang    (strcmp_class, strcmp_bang);
  class_addlist    (strcmp_class, strcmp_list);
  class_addmethod  (strcmp_class, (t_method)strcmp_secondlist, gensym("lst2"), A_GIMME, 0);

  class_sethelpsymbol(strcmp_class, gensym("zexy/strcmp"));
  zexy_register("strcmp");
}
