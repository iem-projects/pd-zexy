/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   1999:forum::für::umläute:2004
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/*
This external makes the two main test-functions available :
dirac~ : will make a single peak (eg: a 1 in all the 0s) at a desired position in the signal-vector
			the position can be passed as an argument when creating the object
step~  : will make a unity step at a desired point in the signal-vector; the second input specifies a 
			length:	after the so-specified time has elapsed, the step will toggle back to the previous
			value;
			the length can be passed as an argument when creating the object
			with length==1 you might do the dirac~ thing a little bit more complicated
			with length==0 the output just toggles between 0 and 1 every time you bang the object

NOTE : the inlets do NOT specify any times but sample-NUMBERS; there are 64 samples in a signal-vector,
		each "lasting" for 1/44100 secs.
*/

#include "zexy.h"

/* ------------------------ dirac~ ----------------------------- */ 


static t_class *dirac_class;

typedef struct _dirac
{
    t_object x_obj;
	t_float position;
	t_float do_it;
} t_dirac;

static void dirac_bang(t_dirac *x)
{
	x->do_it = x->position;
}

static void dirac_float(t_dirac *x, t_float where)
{
	x->do_it = x->position = where;
}

static t_int *dirac_perform(t_int *w)
{
	t_dirac *x = (t_dirac *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	int n = (int)(w[3]);

	int do_it = x->do_it;

	while (n--)
		{
		*out++ = (!do_it--);
		}
	x->do_it = do_it;

	return (w+4);
}

static void dirac_dsp(t_dirac *x, t_signal **sp)
{
	dsp_add(dirac_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void dirac_helper(void)
{
	post("%c dirac~-object :: generates a dirac (unity-pulse)", HEARTSYMBOL);
	post("creation : \"dirac~ [<position>]\" : create a dirac at specified position (in samples)\n"
		 "inlet\t: <position>\t: create a dirac at new position\n"
		 "\t  'bang'\t: create a dirac at specified position\n"
		 "\t  'help'\t: view this\n"
		 "outlet\t: signal~");
}



static void *dirac_new(t_floatarg where)
{
	t_dirac *x = (t_dirac *)pd_new(dirac_class);

	outlet_new(&x->x_obj, gensym("signal"));

	x->do_it = 0;
	x->position = where;
	return (x);
}
 
void dirac_tilde_setup(void)
{
	dirac_class = class_new(gensym("dirac~"), (t_newmethod)dirac_new, 0,
		sizeof(t_dirac), 0, A_DEFFLOAT, 0);
	class_addfloat(dirac_class, dirac_float); 
	class_addbang(dirac_class, dirac_bang); 
	class_addmethod(dirac_class, (t_method)dirac_dsp, gensym("dsp"), 0);

	class_addmethod(dirac_class, (t_method)dirac_helper, gensym("help"), 0);
    class_sethelpsymbol(dirac_class, gensym("zexy/dirac~"));
  zexy_register("dirac~");
}

void z_dirac__setup(void)
{
  dirac_tilde_setup();
}
