/* 2305:forum::für::umläute:2001 */

/* connective objects */

/*
  segregate : segregate atoms by their types
  nop       : a do-nothing, pass-everything
  lister    : the same as "float" for floats but for packages
  a2l       : convert "anything" to "list"
  list2int  : cast all floats of a list to integers
  glue      : glue to lists together (append,...)
  .         : scalar mult
  TODO      : any
*/

#include "zexy.h"


#ifdef NT
#include <string.h>
#endif

/* -------------------- segregate ------------------------------ */

/*
 sorts the input by type ::
   known types are (in order of their outlets ::
   BANG, FLOAT, SYMBOL, LIST, POINTER, ANYTHING
*/

static t_class *segregate_class;

typedef struct _segregate
{
  t_object x_obj;
  
  t_outlet *bang_out, *float_out, *symbol_out, *list_out, *pointer_out, *any_out;
} t_segregate;

static void segregate_bang(t_segregate *x)
{ outlet_bang(x->bang_out); }

static void segregate_float(t_segregate *x, t_float f)
{ outlet_float(x->float_out, f); }

static void segregate_symbol(t_segregate *x, t_symbol *s)
{ outlet_symbol(x->symbol_out, s); }

static void segregate_pointer(t_segregate *x, t_gpointer *gp)
{ outlet_pointer(x->pointer_out, gp); }

static void segregate_list(t_segregate *x, t_symbol *s, int argc, t_atom *argv)
{ outlet_list(x->list_out, s, argc, argv); }

static void segregate_anything(t_segregate *x, t_symbol *s, int argc, t_atom *argv)
{ outlet_anything(x->any_out, s, argc, argv); }

static void *segregate_new(t_symbol *s)
{
  t_segregate *x = (t_segregate *)pd_new(segregate_class);

  x->bang_out    = outlet_new(&x->x_obj, &s_bang);
  x->float_out   = outlet_new(&x->x_obj, &s_float);
  x->symbol_out  = outlet_new(&x->x_obj, &s_symbol);
  x->list_out    = outlet_new(&x->x_obj, &s_list);
  x->pointer_out = outlet_new(&x->x_obj, &s_pointer);
  x->any_out     = outlet_new(&x->x_obj, 0);

  return (x);
}

static void segregate_setup(void)
{
  segregate_class = class_new(gensym("segregate"), (t_newmethod)segregate_new, 
			      0, sizeof(t_segregate), 0, 0);
  
  class_addbang(segregate_class, segregate_bang);
  class_addfloat(segregate_class, (t_method)segregate_float);
  class_addsymbol(segregate_class, segregate_symbol);
  class_addpointer(segregate_class, segregate_pointer);
  class_addlist(segregate_class, segregate_list);
  class_addanything(segregate_class, segregate_anything);

  class_sethelpsymbol(segregate_class, gensym("zexy/segregate"));
}

/* ------------------------- nop ------------------------------- */

/* a no-operation - just pass through what you get in */

static t_class *nop_class;

typedef struct _nop
{
  t_object x_obj;
} t_nop;

static void nop_anything(t_nop *x, t_symbol *s, int argc, t_atom *argv)
{ outlet_anything(x->x_obj.ob_outlet, s, argc, argv);}

static void nop_list(t_nop *x, t_symbol *s, int argc, t_atom *argv)
{ outlet_list(x->x_obj.ob_outlet, s, argc, argv);}

static void nop_float(t_nop *x, t_floatarg f)
{ outlet_float(x->x_obj.ob_outlet, f);}

static void nop_symbol(t_nop *x, t_symbol *s)
{  outlet_symbol(x->x_obj.ob_outlet, s);}

static void nop_pointer(t_nop *x, t_gpointer *gp)
{  outlet_pointer(x->x_obj.ob_outlet, gp);}

static void nop_bang(t_nop *x)
{  outlet_bang(x->x_obj.ob_outlet);}

static void *nop_new(void)
{
  t_nop *x = (t_nop *)pd_new(nop_class);
  outlet_new(&x->x_obj, 0);
  return (x);
}

static void nop_setup(void)
{
  nop_class = class_new(gensym("nop"), (t_newmethod)nop_new, 
			      0, sizeof(t_nop), 0, 0);

  class_addbang    (nop_class, nop_bang);
  class_addfloat   (nop_class, nop_float);
  class_addsymbol  (nop_class, nop_symbol);
  class_addpointer (nop_class, nop_pointer);
  class_addlist    (nop_class, nop_list);
  class_addanything(nop_class, nop_anything);

  class_sethelpsymbol(nop_class, gensym("zexy/nop"));
}




/* ------------------------- a2l ------------------------------- */

/* convert anythings to lists, pass through the rest */

static t_class *a2l_class;

typedef struct _a2l
{
  t_object x_obj;
} t_a2l;

static void a2l_anything(t_a2l *x, t_symbol *s, int argc, t_atom *argv)
{
  int n = argc+1;
  t_atom *cur, *alist = (t_atom *)getbytes(n * sizeof(t_atom));

  cur = alist;
  SETSYMBOL(cur, s);
  cur++;

  memcpy(cur, argv, argc * sizeof(t_atom));

  outlet_list(x->x_obj.ob_outlet, gensym("list"), n, alist);

  freebytes(alist, n * sizeof(t_atom));

}

static void a2l_list(t_a2l *x, t_symbol *s, int argc, t_atom *argv)
{ outlet_list(x->x_obj.ob_outlet, s, argc, argv);}

static void a2l_float(t_a2l *x, t_floatarg f)
{ outlet_float(x->x_obj.ob_outlet, f);}

static void a2l_symbol(t_a2l *x, t_symbol *s)
{  outlet_symbol(x->x_obj.ob_outlet, s);}

static void a2l_pointer(t_a2l *x, t_gpointer *gp)
{  outlet_pointer(x->x_obj.ob_outlet, gp);}

static void a2l_bang(t_a2l *x)
{  outlet_bang(x->x_obj.ob_outlet);}

static void *a2l_new(void)
{
  t_a2l *x = (t_a2l *)pd_new(a2l_class);
  outlet_new(&x->x_obj, 0);
  return (x);
}

static void a2l_setup(void)
{
  
  a2l_class = class_new(gensym("any2list"), (t_newmethod)a2l_new, 
			      0, sizeof(t_a2l), 0, 0);
  class_addcreator((t_newmethod)a2l_new, gensym("a2l"), 0);


  class_addbang    (a2l_class, a2l_bang);
  class_addfloat   (a2l_class, a2l_float);
  class_addsymbol  (a2l_class, a2l_symbol);
  class_addpointer (a2l_class, a2l_pointer);
  class_addlist    (a2l_class, a2l_list);
  class_addanything(a2l_class, a2l_anything);

  class_sethelpsymbol(a2l_class, gensym("zexy/any2list"));
}

/* ------------------------- list ------------------------------- */

/* this is for packages, what "float" is for floats */

static t_class *mypdlist_class;

typedef struct _mypdlist
{
  t_object x_obj;

  int x_n;
  t_atom *x_list;
} t_mypdlist;

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

static void mypdlist_setup(void)
{
  mypdlist_class = class_new(gensym("lister"), (t_newmethod)mypdlist_new, 
			 (t_method)mypdlist_free, sizeof(t_mypdlist), 0, A_GIMME, 0);
  /* i don't know how to get this work with name=="list" !!! */

  class_addcreator((t_newmethod)mypdlist_new, gensym("l"), A_GIMME, 0);

  class_addbang    (mypdlist_class, mypdlist_bang);
  class_addlist    (mypdlist_class, mypdlist_list);
  class_addmethod  (mypdlist_class, (t_method)mypdlist_secondlist, gensym("lst2"), A_GIMME, 0);

  class_sethelpsymbol(mypdlist_class, gensym("zexy/lister"));
}

/* ------------------------- list2int ------------------------------- */

/* cast each float of a list (or anything) to integer */

static t_class *list2int_class;

static void list2int_any(t_mypdlist *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap;
  if (x->x_n != argc) {
    freebytes(x->x_list, x->x_n * sizeof(t_atom));
    x->x_n = argc;
    x->x_list = copybytes(argv, argc * sizeof(t_atom));
  } else memcpy(x->x_list, argv, argc * sizeof(t_atom));
  ap = x->x_list;
  while(argc--){
    if(ap->a_type == A_FLOAT)ap->a_w.w_float=(int)ap->a_w.w_float;
    ap++;
  }
  outlet_anything(x->x_obj.ob_outlet, s, x->x_n, x->x_list);
}
static void list2int_bang(t_mypdlist *x)
{  outlet_bang(x->x_obj.ob_outlet);}
static void list2int_float(t_mypdlist *x, t_float f)
{  outlet_float(x->x_obj.ob_outlet, (int)f);}
static void list2int_symbol(t_mypdlist *x, t_symbol *s)
{  outlet_symbol(x->x_obj.ob_outlet, s);}
static void list2int_pointer(t_mypdlist *x, t_gpointer *p)
{  outlet_pointer(x->x_obj.ob_outlet, p);}

static void *list2int_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mypdlist *x = (t_mypdlist *)pd_new(list2int_class);
  outlet_new(&x->x_obj, 0);
  x->x_n = 0;
  x->x_list = 0;
  return (x);
}

static void list2int_setup(void)
{
  list2int_class = class_new(gensym("list2int"), (t_newmethod)list2int_new, 
			 (t_method)mypdlist_free, sizeof(t_mypdlist), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)list2int_new, gensym("l2i"), A_GIMME, 0);
  class_addanything(list2int_class, list2int_any);
  class_addlist(list2int_class, list2int_any);
  class_addbang(list2int_class, list2int_bang);
  class_addfloat(list2int_class, list2int_float);
  class_addsymbol(list2int_class, list2int_symbol);
  class_addpointer(list2int_class, list2int_pointer);
  class_sethelpsymbol(list2int_class, gensym("zexy/list2int"));
}

/* ------------------------- glue ------------------------------- */

/* glue 2 lists together (append) */

static t_class *glue_class;

typedef struct _glue
{
  t_object x_obj;

  t_atom *ap2, *ap;
  t_int n1, n2, n;

  t_int changed;
} t_glue;

static void glue_lst2(t_glue *x, t_symbol *s, int argc, t_atom *argv)
{
  x->changed = 1;
  if (x->n2 != argc) {
    freebytes(x->ap2, x->n2 * sizeof(t_atom));
    x->n2 = argc;
    x->ap2 = copybytes(argv, argc * sizeof(t_atom));
  } else memcpy(x->ap2, argv, argc * sizeof(t_atom));
}

static void glue_lst(t_glue *x, t_symbol *s, int argc, t_atom *argv)
{
  if (x->n != x->n2+argc) {
    freebytes(x->ap, x->n * sizeof(t_atom));
    x->n1 = argc;
    x->n  = x->n1+x->n2;
    x->ap = (t_atom *)getbytes(sizeof(t_atom)*x->n);
    memcpy(x->ap+argc, x->ap2, x->n2*sizeof(t_atom));
  } else if ((x->n1 != argc)||x->changed)memcpy(x->ap+argc, x->ap2, x->n2*sizeof(t_atom));

  x->n1 = argc;
  memcpy(x->ap, argv, x->n1*sizeof(t_atom));

  x->changed=0;

  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->n, x->ap);
}

static void glue_bang(t_glue *x)
{
  if (x->changed) {
    if (x->n1+x->n2 != x->n){
      t_atom *ap = (t_atom*)getbytes(sizeof(t_atom)*(x->n1+x->n2));
      memcpy(ap, x->ap, x->n1*sizeof(t_atom));
      freebytes(x->ap, sizeof(t_atom)*x->n);
      x->ap=ap;
      x->n=x->n1+x->n2;
    }
    memcpy(x->ap+x->n1, x->ap2, x->n2*sizeof(t_atom));
    x->changed=0;
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->n, x->ap);
}

static void glue_free(t_glue *x)
{
  freebytes(x->ap,  sizeof(t_atom)*x->n);
  freebytes(x->ap2, sizeof(t_atom)*x->n2);
}

static void *glue_new(t_symbol *s, int argc, t_atom *argv)
{
  t_glue *x = (t_glue *)pd_new(glue_class);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym(""));
  outlet_new(&x->x_obj, 0);
  x->n =x->n2  = 0;
  x->ap=x->ap2 = 0;
  x->changed   = 0;

  if (argc)glue_lst2(x, gensym("list"), argc, argv);

  return (x);
}

static void glue_setup(void)
{
  glue_class = class_new(gensym("glue"), (t_newmethod)glue_new, 
			 (t_method)glue_free, sizeof(t_glue), 0, A_GIMME, 0);
  class_addlist(glue_class, glue_lst);
  class_addmethod  (glue_class, (t_method)glue_lst2, gensym(""), A_GIMME, 0);
  class_addbang(glue_class, glue_bang);

  class_sethelpsymbol(glue_class, gensym("zexy/glue"));
}

/*skalar multiplikation */

static t_class *scalmul_class;
static t_class *scalmul_scal_class;

typedef struct _scalmul
{
  t_object x_obj;

  t_int n1, n2;

  t_float *buf1, *buf2;

  t_float f;
} t_scalmul;


static void scalmul_lst2(t_scalmul *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float *fp;
  if (x->n2 != argc) {
    freebytes(x->buf2, x->n2 * sizeof(t_float));
    x->n2 = argc;
    x->buf2=(t_float *)getbytes(sizeof(t_float)*x->n2);
  };
  fp = x->buf2;
  while(argc--)*fp++=atom_getfloat(argv++);
}

static void scalmul_lst(t_scalmul *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float *fp;
  t_atom  *ap;
  int n;

  if (argc){
    if (x->n1 != argc) {
      freebytes(x->buf1, x->n1 * sizeof(t_float));
      x->n1 = argc;
      x->buf1=(t_float *)getbytes(sizeof(t_float)*x->n1);
    };
    fp = x->buf1;
    while(argc--)*fp++=atom_getfloat(argv++);
  }

  if (x->n1*x->n2==1){
    outlet_float(x->x_obj.ob_outlet, *x->buf1**x->buf2);
    return;
  }
  if (x->n1==1){
    t_atom *a;
    int i = x->n2;
    t_float f = *x->buf1;
    fp = x->buf2;
    n = x->n2;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
    a = ap;
    while(i--){
      SETFLOAT(a, *fp++*f);
      a++;
    }
  } else if (x->n2==1){
    t_float f = *x->buf2;
    t_atom *a;
    int i = x->n1;
    n = x->n1;
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
    a = ap;
    fp = x->buf1;
    while(i--){
      SETFLOAT(a, *fp++*f);
      a++;
    }
  } else {
    t_atom *a;
    int i;
    t_float *fp2=x->buf2;
    fp = x->buf1;
    n = x->n1;
    if (x->n1!=x->n2){
      post("scalar multiplication: truncating vectors to the same length");
      if (x->n2<x->n1)n=x->n2;
    }
    ap = (t_atom *)getbytes(sizeof(t_atom)*n);
    a = ap;
    i=n;
    while(i--){
      SETFLOAT(a, *fp++**fp2++);
      a++;
    }
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), n, ap);
  freebytes(ap, sizeof(t_atom)*n);
}
static void scalmul_free(t_scalmul *x)
{
  freebytes(x->buf1, sizeof(t_float)*x->n1);
  freebytes(x->buf2, sizeof(t_float)*x->n2);
}

static void *scalmul_new(t_symbol *s, int argc, t_atom *argv)
{
  t_scalmul *x;

  if (argc-1){
    x = (t_scalmul *)pd_new(scalmul_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym(""));
  } else x = (t_scalmul *)pd_new(scalmul_scal_class);

  outlet_new(&x->x_obj, 0);

  x->n1   =1;
  x->buf1 =(t_float*)getbytes(sizeof(t_float));
  *x->buf1=0;

  if (argc)scalmul_lst2(x, gensym("list"), argc, argv);
  else {
    x->n2   =1;
    x->buf2 =(t_float*)getbytes(sizeof(t_float));
    *x->buf2=0;
  }

  if (argc==1)floatinlet_new(&x->x_obj, x->buf2);

  return (x);
}

static void scalmul_setup(void)
{
  scalmul_class = class_new(gensym("."), (t_newmethod)scalmul_new, 
			    (t_method)scalmul_free, sizeof(t_scalmul), 0, A_GIMME, 0);
  class_addlist(scalmul_class, scalmul_lst);
  class_addmethod  (scalmul_class, (t_method)scalmul_lst2, gensym(""), A_GIMME, 0);
  scalmul_scal_class = class_new(gensym("."), 0, (t_method)scalmul_free, 
				 sizeof(t_scalmul), 0, 0);
  class_addlist(scalmul_scal_class, scalmul_lst);

  class_sethelpsymbol(scalmul_class, gensym("zexy/scalarmult"));
  class_sethelpsymbol(scalmul_scal_class, gensym("zexy/scalarmult"));
}




/* -------------- overall setup routine for this file ----------------- */

void z_connective_setup(void)
{
  segregate_setup();
  nop_setup();
  mypdlist_setup();
  glue_setup();

  list2int_setup();
  scalmul_setup();

  a2l_setup();

  /* I don't supply HELP - functionality, since this might harm overall-performance here */
}
