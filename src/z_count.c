#include "m_pd.h"

static t_class *prime_class;

typedef struct _prime {
  t_object  x_obj;
  t_int i_count;
} t_prime;


void prime_bug(t_prime *x)
{
  bug("bug!");
}

void *prime_new(t_floatarg f)
{
  t_prime *x = (t_prime *)pd_new(prime_class);

  x->i_count=f;
  outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void z_prime_setup(void) {
  prime_class = class_new(gensym("prime"),
			  (t_newmethod)prime_new,
			  0, sizeof(t_prime),
			  CLASS_DEFAULT, 0);

  class_addbang(prime_class, prime_bang);
  class_addmethod(prime_class, (t_method)prime_bug, gensym("bug"), 0);
  class_addmethod(prime_class, (t_method)prime_error, gensym("error"), 0);
}
