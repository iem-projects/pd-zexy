
/* 1008:forum::für::umläute:2001 */

/*
  urn :  "generate random numbers without duplicates"
  very max-like
*/

#include "zexy.h"

/* ------------------------- urn ------------------------------- */

static t_class *urn_class;

typedef struct _urn
{
  t_object x_obj;
  unsigned int x_seed;       /* the seed of the generator */

  unsigned int x_range;      /* max. random-number + 1 */
  unsigned int x_count;      /* how many random numbers have we generated ? */
  char  *x_state;            /* has this number been generated already ? */
} t_urn;

static int makeseed(void)
{
  static unsigned int random_nextseed = 1489853723;
  random_nextseed = random_nextseed * 435898247 + 938284287;
  return (random_nextseed & 0x7fffffff);
}

static void urn_clear(t_urn *x)
{
  unsigned int i=x->x_range;
  t_int *dummy=(t_int*)x->x_state;
  while(i--)*dummy++=0;
  x->x_count=0;
}

static void makestate(t_urn *x, unsigned int newrange)
{
  if (x->x_range == newrange)return;

  if (x->x_range && x->x_state) {
    freebytes(x->x_state, sizeof(char)*x->x_range);
    x->x_state=0;
  }

  x->x_range=newrange;
  x->x_state=getbytes(sizeof(char)*x->x_range);

  urn_clear(x);
}

static void urn_bang(t_urn *x)
{
  unsigned int range = (x->x_range<1?1:x->x_range);
  unsigned int randval = (unsigned int)x->x_state;

  int nval, used=1;

  if (x->x_count>=range)urn_clear(x);

  while (used) {
    randval = randval * 472940017 + 832416023;
    nval = ((double)range) * ((double)randval)
    	* (1./4294967296.);
    if (nval >= range) nval = range-1;
    used=x->x_state[nval];
  }

  x->x_count++;
  x->x_state[nval]=1;
  outlet_float(x->x_obj.ob_outlet, nval);
}

static void urn_flt2(t_urn *x, t_float f)
{
  unsigned int range = (f<1)?1:f;
  makestate(x, range);
}


static void urn_seed(t_urn *x, t_float f)
{
  x->x_seed = f;
}

static void *urn_new(t_floatarg f)
{
  t_urn *x = (t_urn *)pd_new(urn_class);
  
  if (f<1.0)f=1.0;
  x->x_range = f;
  x->x_seed = makeseed();
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  outlet_new(&x->x_obj, &s_float);
  return (x);

  return (x);
}

static void urn_setup(void)
{
  urn_class = class_new(gensym("urn"), (t_newmethod)urn_new, 
			      0, sizeof(t_urn), 0, A_DEFFLOAT,  0);
  
  class_addbang (urn_class, urn_bang);
  class_addmethod(urn_class, (t_method)urn_clear, gensym("clear"), 0);
  class_addmethod(urn_class, (t_method)urn_flt2, gensym(""), A_DEFFLOAT, 0);
  class_addmethod(urn_class, (t_method)urn_seed, gensym("seed"), A_DEFFLOAT, 0);
  

  class_sethelpsymbol(urn_class, gensym("zexy/urn"));
}

void z_random_setup(void)
{
  urn_setup();
}
