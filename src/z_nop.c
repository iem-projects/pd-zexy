#include "zexy.h"
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ nop~ ----------------------------- */
/* this will pass trough the signal unchanged except for a delay of 1 block */

static t_class *nop_class;

typedef struct _nop
{
	t_object x_obj;
	t_float *buf;
	int n;
	int toggle;
} t_nop;

static t_int *nop_perform(t_int *w)
{
	t_float *in  = (t_float *)w[1];
	t_float *out = (t_float *)w[2];
	t_nop *x = (t_nop *)w[3];
	int n = x->n;
	t_float *rp = x->buf + n * x->toggle, *wp = x->buf + n * (x->toggle ^= 1);

	while (n--)	{
		*wp++ = *in++;
		*out++ = *rp++;
	}

	return (w+4);
}

static void nop_dsp(t_nop *x, t_signal **sp)
{
	if (x->n != sp[0]->s_n)
		{
			freebytes(x->buf, x->n * 2 * sizeof(t_float));

			x->buf = (t_float *)getbytes(sizeof(t_float) * 2 * (x->n = sp[0]->s_n));
		}
	dsp_add(nop_perform, 3, sp[0]->s_vec, sp[1]->s_vec, x);
}

static void helper(t_nop *x)
{
	post("%c nop~-object ::\tdo_nothing but delay a signal for 1 block\n"
		"\t\t this might be helpful for synchronising signals", HEARTSYMBOL);
}

static void nop_free(t_nop *x)
{
	freebytes(x->buf, x->n * sizeof(t_float));
}


static void *nop_new()
{
	t_nop *x = (t_nop *)pd_new(nop_class);
	outlet_new(&x->x_obj, gensym("signal"));
	x->toggle = 0;
	x->n = 0;

	return (x);
}

void z_nop_setup(void)
{
	nop_class = class_new(gensym("nop~"), (t_newmethod)nop_new, (t_method)nop_free,
					sizeof(t_nop), 0, A_DEFFLOAT, 0);
	class_addmethod(nop_class, nullfn, gensym("signal"), 0);
	class_addmethod(nop_class, (t_method)nop_dsp, gensym("dsp"), 0);

	class_addmethod(nop_class, (t_method)helper, gensym("help"), 0);
	class_sethelpsymbol(nop_class, gensym("zexy/nop~"));
}
