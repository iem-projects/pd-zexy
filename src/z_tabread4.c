
/* ---------- tabread4: control, interpolating ------------------------ */
/* hack : 2108:forum::für::umläute:1999 @ iem */

#include "zexy.h"


/* =================== tabdump ====================== */

static t_class *tabdump_class;

typedef struct _tabdump
{
  t_object x_obj;
  t_symbol *x_arrayname;
  t_int startindex, stopindex;
} t_tabdump;

static void tabdump_bang(t_tabdump *x)
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
      t_atom *atombuf;

      int start=x->startindex;
      int stop =x->stopindex;
      if(start<0||start>stop)start=0;
      if(stop<start||stop>npoints)stop=npoints;
      npoints=stop-start;

      atombuf = (t_atom *)getbytes(sizeof(t_atom)*npoints);
      for (n = 0; n < npoints; n++) SETFLOAT(&atombuf[n], vec[start+n]);
      outlet_list(x->x_obj.ob_outlet, &s_list, npoints, atombuf);
      freebytes(atombuf,sizeof(t_atom)*npoints);
    }
}

static void tabdump_list(t_tabdump *x, t_symbol*s,int argc, t_atom*argv)
{
  int a,b;
  switch(argc){
  case 2:
    a=atom_getint(argv);
    b=atom_getint(argv+1);
    x->startindex=(a<b)?a:b;
    x->stopindex =(a>b)?a:b;
    tabdump_bang(x);
    break;
  default:
    error("tabdump: list must be 2 floats (is %d atoms)", argc);
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
  x->startindex=0;
  x->stopindex=-1;
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
  class_addlist(tabdump_class, (t_method)tabdump_list);

  class_addmethod(tabdump_class, (t_method)tabdump_set, gensym("set"),
		  A_SYMBOL, 0);

  class_addmethod(tabdump_class, (t_method)tabdump_helper, gensym("help"), 0);
  class_sethelpsymbol(tabdump_class, gensym("zexy/tabdump"));
}

/* =================== tabminmax ====================== */

static t_class *tabminmax_class;

typedef struct _tabminmax
{
  t_object x_obj;
  t_outlet*min_out, *max_out;
  t_symbol *x_arrayname;
  t_int startindex, stopindex;
} t_tabminmax;

static void tabminmax_bang(t_tabminmax *x)
{
  t_garray *A;
  int npoints;
  t_float *vec;

  if (!(A = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    error("%s: no such array", x->x_arrayname->s_name);
  else if (!garray_getfloatarray(A, &npoints, &vec))
    error("%s: bad template for tabminmax", x->x_arrayname->s_name);
  else
    {
      int n;
      t_atom atombuf[2];
      t_float min, max;
      int mindex, maxdex;

      int start=x->startindex;
      int stop =x->stopindex;
      if(start<0||start>stop)start=0;
      if(stop<start||stop>npoints)stop=npoints;
      npoints=stop-start;

      min=vec[start];
      max=vec[start];

      mindex=start;
      maxdex=start;
      
      for (n = 1; n < npoints; n++){
	t_float val=vec[start+n];
	if(val<min){
	  mindex=start+n;
	  min=val;
	}
	if(val>max){
	  maxdex=start+n;
	  max=val;
	}
      }

      SETFLOAT(atombuf, max);
      SETFLOAT(atombuf+1, maxdex);
      outlet_list(x->max_out, &s_list, 2, atombuf);

      SETFLOAT(atombuf, min);
      SETFLOAT(atombuf+1, mindex);
      outlet_list(x->min_out, &s_list, 2, atombuf);
    }
}

static void tabminmax_list(t_tabminmax *x, t_symbol*s,int argc, t_atom*argv)
{
  int a,b;
  switch(argc){
  case 2:
    a=atom_getint(argv);
    b=atom_getint(argv+1);
    x->startindex=(a<b)?a:b;
    x->stopindex =(a>b)?a:b;
    tabminmax_bang(x);
    break;
  default:
    error("tabminmax: list must be 2 floats (is %d atoms)", argc);
  }
}

static void tabminmax_set(t_tabminmax *x, t_symbol *s)
{
  x->x_arrayname = s;
}

static void *tabminmax_new(t_symbol *s)
{
  t_tabminmax *x = (t_tabminmax *)pd_new(tabminmax_class);
  x->x_arrayname = s;
  x->startindex=0;
  x->stopindex=-1;
  x->min_out=outlet_new(&x->x_obj, &s_list);
  x->max_out=outlet_new(&x->x_obj, &s_list);

  return (x);
}

static void tabminmax_helper(void)
{
  post("\n%c tabminmax - object : dumps a table as a package of floats", HEARTSYMBOL);
  post("'set <table>'\t: read out another table\n"
       "'bang'\t\t: get min and max of the table\n"
       "outlet\t\t: table-data as package of floats");
  post("creation\t: \"tabminmax <table>\"");

}

static void tabminmax_setup(void)
{
  tabminmax_class = class_new(gensym("tabminmax"), (t_newmethod)tabminmax_new,
			     0, sizeof(t_tabminmax), 0, A_DEFSYM, 0);
  class_addbang(tabminmax_class, (t_method)tabminmax_bang);
  class_addlist(tabminmax_class, (t_method)tabminmax_list);

  class_addmethod(tabminmax_class, (t_method)tabminmax_set, gensym("set"),
		  A_SYMBOL, 0);

  class_addmethod(tabminmax_class, (t_method)tabminmax_helper, gensym("help"), 0);
  class_sethelpsymbol(tabminmax_class, gensym("zexy/tabminmax"));
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


void z_tabread4_setup(void)
{
  tabdump_setup();
  tabminmax_setup();
  tabset_setup();
}
