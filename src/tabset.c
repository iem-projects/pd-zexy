/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) IOhannes m zm�lnig
 *
 *   1999:forum::f�r::uml�ute:2004
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

/* hack : 2108:forum::f�r::uml�ute:1999 @ iem */

#include "zexy.h"


/* =================== tabset ====================== */

static t_class *tabset_class;

typedef struct _tabset
{
  t_object x_obj;
  t_symbol *x_arrayname;
} t_tabset;

static void tabset_float(t_tabset *x, t_floatarg f)
{
  t_garray *A;
  int npoints;
  t_float *vec;

  if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!garray_getfloatarray(A, &npoints, &vec))
    error("%s: bad template for tabset", x->x_arrayname->s_name);
  else {
    while(npoints--)*vec++=f;
    garray_redraw(A);
  }
}

static void tabset_list(t_tabset *x, t_symbol *s, int argc, t_atom* argv)
{
  t_garray *A;
  int npoints;
  t_float *vec;

  if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!garray_getfloatarray(A, &npoints, &vec))
    error("%s: bad template for tabset", x->x_arrayname->s_name);
  else {
    if (argc>=npoints)
      while(npoints--)*vec++=atom_getfloat(argv++);
    else {
      npoints-=argc;
      while (argc--)*vec++=atom_getfloat(argv++);
      while (npoints--)*vec++=0;
    }
    garray_redraw(A);
  }
}
static void tabset_set(t_tabset *x, t_symbol *s)
{
  x->x_arrayname = s;
}

static void *tabset_new(t_symbol *s)
{
  t_tabset *x = (t_tabset *)pd_new(tabset_class);
  x->x_arrayname = s;

  return (x);
}

static void tabset_helper(void)
{
  post("\n%c tabset - object : set a table with a package of floats", HEARTSYMBOL);
  post("'set <table>'\t: set another table\n"
       "<list>\t\t: set the table"
       "<float>\t\t: set the table to constant float\n");
  post("creation\t: \"tabset <table>\"");
}

void tabset_setup(void)
{
  tabset_class = class_new(gensym("tabset"), (t_newmethod)tabset_new,
			     0, sizeof(t_tabset), 0, A_DEFSYM, 0);
  class_addfloat(tabset_class, (t_method)tabset_float);
  class_addlist (tabset_class, (t_method)tabset_list);
  class_addmethod(tabset_class, (t_method)tabset_set, gensym("set"),
		  A_SYMBOL, 0);

  class_addmethod(tabset_class, (t_method)tabset_helper, gensym("help"), 0);
  class_sethelpsymbol(tabset_class, gensym("zexy/tabset"));
}