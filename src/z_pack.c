/* 3108:forum::für::umläute:2000 */

/* objects for manipulating packages*/

/*
  repack    : (re)pack floats/symbols/pointers to fixed-length packages
  niagara   : divides a package into 2 sub-packages
  packel    : get a specifique element of a package by its index
*/

#include "zexy.h"
#include <string.h>
#ifdef NT
#include <memory.h>
//#error do we need memory.h
#endif


/* -------------------- repack ------------------------------ */

/*
(re)pack a sequence of (packages of) atoms into a package of given length

"bang" gives out the current package immediately
the second inlet lets you change the default package size
*/

static t_class *repack_class;

typedef struct _repack
{
  t_object x_obj;
  t_atom  *buffer;
  int      bufsize;

  int      outputsize;
  int      current;
} t_repack;


static void repack_set(t_repack *x, t_float f)
{
  /* set the new default size */
  int n = f;

  if (n > 0) {

    /* flush all the newsized packages that are in the buffer */
    t_atom *dumbuf = x->buffer;
    int     dumcur = x->current;

    while (n <= dumcur) {
      outlet_list(x->x_obj.ob_outlet, gensym("list"), n, dumbuf);
      dumcur -= n;
      dumbuf += n;
    }

    if (dumcur < 0) error("this should never happen :: dumcur = %d < 0", dumcur);
    else {
      memcpy(x->buffer, dumbuf, dumcur * sizeof(t_atom));
      x->current = dumcur;
    }
	 
    if (n > x->bufsize) {
      dumbuf = (t_atom *)getbytes(n * sizeof(t_atom));
      memcpy(dumbuf, x->buffer, x->current * sizeof(t_atom));
      freebytes(x->buffer, x->bufsize * sizeof(t_atom));
      x->buffer =  dumbuf;
      x->bufsize = n;
    }
    
    x->outputsize = n;
  }
}

static void repack_bang(t_repack *x)
{
  /* output the list as far as we are now */
  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->current, x->buffer);
  x->current = 0;
}

static void repack_float(t_repack *x, t_float f)
{
  /* add a float-atom to the list */
  SETFLOAT(&x->buffer[x->current], f);
  x->current++;
  if (x->current >= x->outputsize) repack_bang(x);
}

static void repack_symbol(t_repack *x, t_symbol *s)
{
  /* add a sym-atom to the list */
  SETSYMBOL(&x->buffer[x->current], s);
  x->current++;
  if (x->current >= x->outputsize) repack_bang(x);
}
static void repack_pointer(t_repack *x, t_gpointer *p)
{
  /* add a pointer-atom to the list */
  SETPOINTER(&x->buffer[x->current], p);
  x->current++;
  if (x->current >= x->outputsize) repack_bang(x);
}
static void repack_list(t_repack *x, t_symbol *s, int argc, t_atom *argv)
{
  int remain = x->outputsize - x->current;
  t_atom *ap = argv;

  if (argc >= remain) {
    memcpy(x->buffer+x->current, ap, remain * sizeof(t_atom));
    ap   += remain;
    argc -= remain;
    outlet_list(x->x_obj.ob_outlet, gensym("list"), x->outputsize, x->buffer);
    x->current = 0;
  }

  while (argc >= x->outputsize) {
    outlet_list(x->x_obj.ob_outlet, gensym("list"), x->outputsize, ap);
    ap += x->outputsize;
    argc -= x->outputsize;
  }

  memcpy(x->buffer + x->current, ap, argc * sizeof(t_atom));
  x->current += argc;
}
static void repack_anything(t_repack *x, t_symbol *s, int argc, t_atom *argv)
{
  SETSYMBOL(&x->buffer[x->current], s);
  x->current++;

  if (x->current >= x->outputsize) {
    repack_bang(x);
  }
  repack_list(x, gensym("list"), argc, argv);
}

static void *repack_new(t_floatarg f)
{
  t_repack *x = (t_repack *)pd_new(repack_class);



  x->outputsize = x->bufsize = (f>0.f)?f:2 ;
  x->current = 0;


  x->buffer = (t_atom *)getbytes(x->bufsize * sizeof(t_atom));

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  outlet_new(&x->x_obj, 0);

  return (x);
}

static void repack_setup(void)
{
  repack_class = class_new(gensym("repack"), (t_newmethod)repack_new, 
			   0, sizeof(t_repack), 0, A_DEFFLOAT, 0);
  
  class_addbang    (repack_class, repack_bang);
  class_addfloat   (repack_class, repack_float);
  class_addsymbol  (repack_class, repack_symbol);
  class_addpointer (repack_class, repack_pointer);
  class_addlist    (repack_class, repack_list);
  class_addanything(repack_class, repack_anything);
  class_addmethod  (repack_class, (t_method)repack_set, gensym(""), A_DEFFLOAT, 0);

  class_sethelpsymbol(repack_class, gensym("zexy/repack"));
}

/* ------------------------- niagara ------------------------------- */

/*
divides a package into 2 sub-packages at a specified point
like the niagara-falls, some water goes down to the left side, the rest to the right side, devided by the rock
*/

static t_class *niagara_class;

typedef struct _niagara
{
  t_object x_obj;
  t_float rock;
  t_outlet *left, *right;
} t_niagara;

static void niagara_list(t_niagara *x, t_symbol *s, int argc, t_atom *argv)
{
  int n_l, n_r;
  t_atom *ap_l, *ap_r;
  int dumrock = x->rock;
  int rock = ((dumrock < 0.f)?(argc+dumrock):dumrock);

  n_l  = (rock < argc)?rock:argc;
  ap_l = argv;

  n_r  = argc - n_l;
  ap_r = &argv[n_l];

  if (n_r) outlet_list(x->right, s, n_r, ap_r);
  if (n_l) outlet_list(x->left, s, n_l, ap_l);
}

static void niagara_any(t_niagara *x, t_symbol *s, int argc, t_atom *argv)
{
  int n_l, n_r;
  t_atom *ap_l, *ap_r;
  t_symbol *s_r, *s_l;
  int dumrock = x->rock;
  int rock = ((dumrock < 0.f)?(argc+dumrock):dumrock-1);

  n_l  = (rock < argc)?rock:argc;
  ap_l = argv;
  s_l  = s;

  n_r  = argc - n_l;
  ap_r = &argv[n_l];

  if (n_r) {
    s_r = 0;
    if (ap_r->a_type == A_FLOAT) s_r = gensym("list");
    else {
      s_r = atom_getsymbol(ap_r);
      ap_r++;
      n_r--;
    }
    outlet_anything(x->right, s_r, n_r, ap_r);
  }

  if (n_l+1 ) outlet_anything(x->left, s_l, n_l, ap_l);
}

static void *niagara_new(t_floatarg f)
{
  t_niagara *x = (t_niagara *)pd_new(niagara_class);

  x->rock = f;

  x->left =  outlet_new(&x->x_obj, &s_list);
  x->right = outlet_new(&x->x_obj, &s_list);

  floatinlet_new(&x->x_obj, &x->rock);

  return (x);
}

static void niagara_setup(void)
{
  niagara_class = class_new(gensym("niagara"), (t_newmethod)niagara_new, 
			    0, sizeof(t_niagara), 0, A_DEFFLOAT,  0);
  
  class_addlist    (niagara_class, niagara_list);
  class_addanything(niagara_class, niagara_any);

  class_sethelpsymbol(niagara_class, gensym("zexy/niagara"));
}

/* ------------------------- packel ------------------------------- */

/*
get the nth element of a package
*/

static t_class *packel_class;

typedef struct _packel
{
  t_object x_obj;
  t_float position;
} t_packel;

static void packel_list(t_packel *x, t_symbol *s, int argc, t_atom *argv)
{
  int mypos = x->position;

  if (mypos) {
    t_atom *current;
    int pos = (mypos < 0)?(argc+mypos):(mypos-1);

    if(argc==0){
      if (pos==0||pos==-1)outlet_bang(x->x_obj.ob_outlet);
      return;
    }
    
    if (pos < 0 || pos >= argc)return;

    current = &(argv[pos]);

    switch (current->a_type) {
    case A_FLOAT:
      outlet_float(x->x_obj.ob_outlet, atom_getfloat(current));
      break;
    case A_SYMBOL:
      outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(current));
      break;
    case A_POINTER:
      outlet_pointer(x->x_obj.ob_outlet, current->a_w.w_gpointer);
      break;
    case A_NULL:
      outlet_bang(x->x_obj.ob_outlet);
    default:
	  ;
    }
  } else outlet_list(x->x_obj.ob_outlet, s, argc, argv);
}

static void packel_anything(t_packel *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *av2 = (t_atom *)getbytes((argc + 1) * sizeof(t_atom));
  int i;

  for (i = 0; i < argc; i++)
    av2[i + 1] = argv[i];
  SETSYMBOL(av2, s);
  packel_list(x, gensym("list"), argc+1, av2);
  freebytes(av2, (argc + 1) * sizeof(t_atom));
}

static void *packel_new(t_floatarg f)
{
  t_packel *x = (t_packel *)pd_new(packel_class);
  outlet_new(&x->x_obj, 0);
  floatinlet_new(&x->x_obj, &x->position);
  x->position = (int) f;

  return (x);
}

static void packel_setup(void)
{
  packel_class = class_new(gensym("packel"), (t_newmethod)packel_new, 
			   0, sizeof(t_packel), 0, A_DEFFLOAT, 0);

  class_addlist  (packel_class, packel_list);
  class_addanything(packel_class, packel_anything);

  class_sethelpsymbol(packel_class, gensym("zexy/packel"));
}

/* -------------- overall setup routine for this file ----------------- */

void z_pack_setup(void)
{
  repack_setup();
  niagara_setup();
  packel_setup();

  /* I don't supply HELP - functionality, since this might harm overall-performance ere */
}
