#include "zexy.h"
#include <stdlib.h>

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#define sqrtf sqrt
#endif

/*
 * atoi : ascii to integer
 * strcmp    : compare 2 lists as if they were strings
*/

/* atoi ::  ascii to integer */

static t_class *atoi_class;

typedef struct _atoi
{
  t_object x_obj;
  int i;
} t_atoi;
static void atoi_bang(t_atoi *x)
{
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_float(t_atoi *x, t_floatarg f)
{
  x->i = f;
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_symbol(t_atoi *x, t_symbol *s)
{
  int base=10;
  const char* c = s->s_name;
  if(c[0]=='0'){
    base=8;
    if (c[1]=='x')base=16;
  }
  x->i=strtol(c, 0, base);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_list(t_atoi *x, t_symbol *s, int argc, t_atom *argv)
{
  int base=10;
  const char* c;

  if (argv->a_type==A_FLOAT){
    x->i=atom_getfloat(argv);
    outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
    return;
  }

  if (argc>1){
    base=atom_getfloat(argv+1);
    if (base<2) {
      error("atoi: setting base to 10");
      base=10;
    }
  }
  c=atom_getsymbol(argv)->s_name;
  x->i=strtol(c, 0, base);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}

static void *atoi_new(void)
{
  t_atoi *x = (t_atoi *)pd_new(atoi_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

static void atoi_setup(void)
{
  atoi_class = class_new(gensym("atoi"), (t_newmethod)atoi_new, 0,
			 sizeof(t_atoi), 0, A_DEFFLOAT, 0);

  class_addbang(atoi_class, (t_method)atoi_bang);
  class_addfloat(atoi_class, (t_method)atoi_float);
  class_addlist(atoi_class, (t_method)atoi_list);
  class_addsymbol(atoi_class, (t_method)atoi_symbol);
  class_addanything(atoi_class, (t_method)atoi_symbol);

  class_sethelpsymbol(atoi_class, gensym("zexy/atoi"));
}

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


static void strcmp_setup(void)
{
  strcmp_class = class_new(gensym("strcmp"), (t_newmethod)strcmp_new, 
			 (t_method)strcmp_free, sizeof(t_strcmp), 0, A_GIMME, 0);

  class_addbang    (strcmp_class, strcmp_bang);
  class_addlist    (strcmp_class, strcmp_list);
  class_addmethod  (strcmp_class, (t_method)strcmp_secondlist, gensym("lst2"), A_GIMME, 0);

  class_sethelpsymbol(strcmp_class, gensym("zexy/strcmp"));
}

/* ------------------------- list2symbol ------------------------------- */

/* compare 2 lists ( == for lists) */

static t_class *list2symbol_class;

typedef struct _list2symbol
{
  t_object x_obj;

  t_binbuf *bbuf;
} t_list2symbol;

static void list2symbol_bang(t_list2symbol *x)
{
  char *str=0, *s2;
  int n=0;

  binbuf_gettext(x->bbuf, &str, &n);
  /* memory bug ! detected and fixed by Juvu */
  s2 = copybytes(str, n+1);
  s2[n]=0;

  outlet_symbol(x->x_obj.ob_outlet, gensym(s2));
  freebytes(str, n);
  freebytes(s2,n+1);
}

static void list2symbol_list(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  binbuf_clear(x->bbuf);
  binbuf_add(x->bbuf, argc, argv);
  
  list2symbol_bang(x);
}
static void list2symbol_anything(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom ap;
  binbuf_clear(x->bbuf);
  SETSYMBOL(&ap, s);
  binbuf_add(x->bbuf, 1, &ap);
  binbuf_add(x->bbuf, argc, argv);
  
  list2symbol_bang(x);
}

static void *list2symbol_new(t_symbol *s, int argc, t_atom *argv)
{
  t_list2symbol *x = (t_list2symbol *)pd_new(list2symbol_class);

  outlet_new(&x->x_obj, 0);
  x->bbuf = binbuf_new();
  binbuf_add(x->bbuf, argc, argv);

  return (x);
}

static void list2symbol_free(t_list2symbol *x)
{
  binbuf_free(x->bbuf);
}


static void list2symbol_setup(void)
{
  list2symbol_class = class_new(gensym("list2symbol"), (t_newmethod)list2symbol_new, 
			 (t_method)list2symbol_free, sizeof(t_list2symbol), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)list2symbol_new, gensym("l2s"), A_GIMME, 0);
  class_addbang    (list2symbol_class, list2symbol_bang);
  class_addlist    (list2symbol_class, list2symbol_list);
  class_addanything(list2symbol_class, list2symbol_anything);

  class_sethelpsymbol(list2symbol_class, gensym("zexy/list2symbol"));
}


/* global setup routine */

void z_strings_setup(void)
{
  atoi_setup();
  strcmp_setup();
  list2symbol_setup();
}
