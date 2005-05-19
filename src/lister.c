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

/* 2305:forum::für::umläute:2001 */


#include "zexy.h"
#include <string.h>

/* ------------------------- list ------------------------------- */

/* this is for packages, what "float" is for floats */

static t_class *mypdlist_class;

static void mypdlist_secondlist(t_mypdlist *x, t_symbol *s, int argc, t_atom *argv)
{
  if (argc) {
    if (x->x_n != argc) {
      freebytes(x->x_list, x->x_n * sizeof(t_atom));
      x->x_n = argc;
      x->x_list = copybytes(argv, argc * sizeof(t_atom));
    } else memcpy(x->x_list, argv, argc * sizeof(t_atom));
  }
}

static void mypdlist_list(t_mypdlist *x, t_symbol *s, int argc, t_atom *argv)
{
  if (x->x_n != argc) {
    freebytes(x->x_list, x->x_n * sizeof(t_atom));
    x->x_n = argc;
    x->x_list = copybytes(argv, argc * sizeof(t_atom));
  } else memcpy(x->x_list, argv, argc * sizeof(t_atom));
  
  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->x_n, x->x_list);
}
static void mypdlist_bang(t_mypdlist *x)
{ outlet_list(x->x_obj.ob_outlet, gensym("list"), x->x_n, x->x_list);}

static void mypdlist_free(t_mypdlist *x)
{ freebytes(x->x_list, x->x_n * sizeof(t_atom)); }

static void *mypdlist_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mypdlist *x = (t_mypdlist *)pd_new(mypdlist_class);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("lst2"));

  x->x_n = 0;
  x->x_list = 0;

  mypdlist_secondlist(x, gensym("list"), argc, argv);

  return (x);
}

void lister_setup(void)
{
  mypdlist_class = class_new(gensym("lister"), (t_newmethod)mypdlist_new, 
                             (t_method)mypdlist_free, sizeof(t_mypdlist), 0, A_GIMME, 0);
  /* i don't know how to get this work with name=="list" !!! */

  class_addcreator((t_newmethod)mypdlist_new, gensym("l"), A_GIMME, 0);

  class_addbang    (mypdlist_class, mypdlist_bang);
  class_addlist    (mypdlist_class, mypdlist_list);
  class_addmethod  (mypdlist_class, (t_method)mypdlist_secondlist, gensym("lst2"), A_GIMME, 0);

  class_sethelpsymbol(mypdlist_class, gensym("zexy/lister"));
  zexy_register("lister");
}
void l_setup(void)
{
  lister_setup();
}

void z_lister_setup(void)
{
  lister_setup();
}
