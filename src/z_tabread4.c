
/* ---------- tabread4: control, interpolating ------------------------ */
/* hack : 2108:forum::für::umläute:1999 @ iem */

#include "zexy.h"


/* =================== tabdump ====================== */

static t_class *tabdump_class;

typedef struct _tabdump
{
  t_object x_obj;
  t_symbol *x_arrayname;
} t_tabdump;

static void tabdump_bang(t_tabdump *x, t_float findex)
{
  t_garray *A;
  int npoints;
  t_float *vec;

  if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!garray_getfloatarray(A, &npoints, &vec))
    error("%s: bad template for tabdump", x->x_arrayname->s_name);
  else
    {
      int n;
      t_atom *atombuf = (t_atom *)getbytes(sizeof(t_atom)*npoints);

      for (n = 0; n < npoints; n++) SETFLOAT(&atombuf[n], vec[n]);
      outlet_list(x->x_obj.ob_outlet, &s_list, npoints, atombuf);
    }
}


static void tabdump_set(t_tabdump *x, t_symbol *s)
{
  x->x_arrayname = s;
}

static void *tabdump_new(t_symbol *s)
{
  t_tabdump *x = (t_tabdump *)pd_new(tabdump_class);
  x->x_arrayname = s;
  outlet_new(&x->x_obj, &s_list);

  return (x);
}

static void tabdump_helper(void)
{
  post("\n%c tabdump - object : dumps a table as a package of floats", HEARTSYMBOL);
  post("'set <table>'\t: read out another table\n"
       "'bang'\t\t: dump the table\n"
       "outlet\t\t: table-data as package of floats");
  post("creation\t: \"tabdump <table>\"");

}

static void tabdump_setup(void)
{
  tabdump_class = class_new(gensym("tabdump"), (t_newmethod)tabdump_new,
			     0, sizeof(t_tabdump), 0, A_DEFSYM, 0);
  class_addbang(tabdump_class, (t_method)tabdump_bang);
  class_addmethod(tabdump_class, (t_method)tabdump_set, gensym("set"),
		  A_SYMBOL, 0);

  class_addmethod(tabdump_class, (t_method)tabdump_helper, gensym("help"), 0);
  class_sethelpsymbol(tabdump_class, gensym("zexy/tabdump"));
}


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

static void tabset_setup(void)
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


/* =================== tabread4 ====================== */

static t_class *tabread4_class;

typedef struct _tabread
{
  t_object x_obj;
  t_symbol *x_arrayname;
} t_tabread;

static void tabread4_float(t_tabread *x, t_float findex)
{
  t_garray *A;
  int npoints;
  t_float *vec;

  if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!garray_getfloatarray(A, &npoints, &vec))

    error("%s: bad template for tabread", x->x_arrayname->s_name);
  else
    {
      float a, b, c, d, cminusb, frac, out;
      int index = (int)findex, max_index = npoints - 1;

      if (index < 0) index = 0, frac = 0.;
      else if (index > max_index) index = max_index, frac = 0.;
      else frac = findex - index;

      a = vec[index-1];
      b = vec[index];
      c = vec[index+1];
      d = vec[index+2];
      cminusb = c - b;

      out = 	(t_float)(b + frac * (cminusb - 0.5 * (frac-1.) * (
	        (a - d + 3.0 * cminusb) * frac + 
		(b - a - cminusb)
		)
	      ));

      outlet_float(x->x_obj.ob_outlet, (npoints ? out : 0));
    }
}


static void tabread4_set(t_tabread *x, t_symbol *s)
{
  x->x_arrayname = s;
}

static void *tabread4_new(t_symbol *s)
{
  t_tabread *x = (t_tabread *)pd_new(tabread4_class);
  x->x_arrayname = s;
  outlet_new(&x->x_obj, &s_float);
  
  post("zexy/tabread4: this is obsolete, since tabread4 is now included into pd");
  post("maybe you should go for an upgrade...");
  return (x);
}

static void tabread4_helper(void)
{
  post("\n%c tabread4 - object : reads out a table (4point-interpolation-support)", HEARTSYMBOL);
  post("'set <table>': read out another table\n"
       "<position>   : read from table at specified position\n"
       "outlet\t:  table-data at specified position (evt. interpolated)");
  post("creation:  \"tabread4 <table>\"");

}

static void tabread4_setup(void)
{
  tabread4_class = class_new(gensym("tabread4"), (t_newmethod)tabread4_new,
			     0, sizeof(t_tabread), 0, A_DEFSYM, 0);
  class_addfloat(tabread4_class, (t_method)tabread4_float);
  class_addmethod(tabread4_class, (t_method)tabread4_set, gensym("set"),
		  A_SYMBOL, 0);

  class_addmethod(tabread4_class, (t_method)tabread4_helper, gensym("help"), 0);
  class_sethelpsymbol(tabread4_class, gensym("zexy/tabread4"));
}

void z_tabread4_setup(void)
{
  tabread4_setup();
  tabdump_setup();
  tabset_setup();
}
