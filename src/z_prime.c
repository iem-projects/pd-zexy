#include "zexy.h"
#include <math.h>


static t_class *prime_class;

typedef struct _prime {
  t_object  x_obj;
} t_prime;


void prime_float(t_prime *x, t_float f)
{

  unsigned int i=f;
  unsigned int max_divisor;
  unsigned int divisor=1;

  if (f<2){
    outlet_float(x->x_obj.ob_outlet, 0.0);
    return;
  }

  if (!(i%2)){
    outlet_float(x->x_obj.ob_outlet, (t_float)(i==2));
    return;
  }

  max_divisor = sqrt(f)+1;

  while ((divisor+=2)<max_divisor)
    if (!(i%divisor)) {
      outlet_float(x->x_obj.ob_outlet, 0.0);
      return;
    }

  outlet_float(x->x_obj.ob_outlet, 1.0);
}

void *prime_new(void)
{
  t_prime *x = (t_prime *)pd_new(prime_class);

  outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void z_prime_setup(void) {
  prime_class = class_new(gensym("prime"),
			  (t_newmethod)prime_new,
			  0, sizeof(t_prime),
			  CLASS_DEFAULT, 0);

  class_addfloat(prime_class, prime_float);
}
