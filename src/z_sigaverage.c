#include "zexy.h"
#include <math.h>

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#define sqrtf sqrt
#endif

#ifdef MACOSX
#define sqrtf sqrt
#endif

/* ---------------- envrms~ - simple envelope follower. ----------------- */
/* this is exactly the same as msp's env~-object, but does not output dB but RMS !! */
/* i found env~+dbtorms most inconvenient (and expensive...) */

#define MAXOVERLAP 10
#define MAXVSTAKEN 64

t_class *sigenvrms_class;

typedef struct sigenvrms
{
    t_object x_obj; 	    	    /* header */
    void *x_outlet;		    /* a "float" outlet */
    void *x_clock;		    /* a "clock" object */
    float *x_buf;		    /* a Hanning window */
    int x_phase;		    /* number of points since last output */
    int x_period;		    /* requested period of output */
    int x_realperiod;		    /* period rounded up to vecsize multiple */
    int x_npoints;		    /* analysis window size in samples */
    float x_result;		    /* result to output */
    float x_sumbuf[MAXOVERLAP];	    /* summing buffer */
} t_sigenvrms;

static void sigenvrms_tick(t_sigenvrms *x);

static void *sigenvrms_new(t_floatarg fnpoints, t_floatarg fperiod)
{
    int npoints = fnpoints;
    int period = fperiod;
    t_sigenvrms *x;
    float *buf;
    int i;

    if (npoints < 1) npoints = 1024;
    if (period < 1) period = npoints/2;
    if (period < npoints / MAXOVERLAP + 1)
	period = npoints / MAXOVERLAP + 1;
    if (!(buf = getbytes(sizeof(float) * (npoints + MAXVSTAKEN))))
    {
	error("env: couldn't allocate buffer");
	return (0);
    }
    x = (t_sigenvrms *)pd_new(sigenvrms_class);
    x->x_buf = buf;
    x->x_npoints = npoints;
    x->x_phase = 0;
    x->x_period = period;
    for (i = 0; i < MAXOVERLAP; i++) x->x_sumbuf[i] = 0;
    for (i = 0; i < npoints; i++)
	buf[i] = (1. - cos((2 * 3.141592654 * i) / npoints))/npoints;
    for (; i < npoints+MAXVSTAKEN; i++) buf[i] = 0;
    x->x_clock = clock_new(x, (t_method)sigenvrms_tick);
    x->x_outlet = outlet_new(&x->x_obj, gensym("float"));
    return (x);
}

static t_int *sigenvrms_perform(t_int *w)
{
    t_sigenvrms *x = (t_sigenvrms *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    int count;
    float *sump; 
    in += n;
    for (count = x->x_phase, sump = x->x_sumbuf;
	count < x->x_npoints; count += x->x_realperiod, sump++)
    {
	float *hp = x->x_buf + count;
	float *fp = in;
	float sum = *sump;
	int i;
	
	for (i = 0; i < n; i++)
	{
	    fp--;
	    sum += *hp++ * (*fp * *fp);
	}
	*sump = sum;
    }
    sump[0] = 0;
    x->x_phase -= n;
    if (x->x_phase < 0)
    {
	x->x_result = x->x_sumbuf[0];
	for (count = x->x_realperiod, sump = x->x_sumbuf;
	    count < x->x_npoints; count += x->x_realperiod, sump++)
		sump[0] = sump[1];
	sump[0] = 0;
	x->x_phase = x->x_realperiod - n;
	clock_delay(x->x_clock, 0L);
    }
    return (w+4);
}

static void sigenvrms_dsp(t_sigenvrms *x, t_signal **sp)
{
    if (x->x_period % sp[0]->s_n) x->x_realperiod =
	x->x_period + sp[0]->s_n - (x->x_period % sp[0]->s_n);
    else x->x_realperiod = x->x_period;
    dsp_add(sigenvrms_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
    if (sp[0]->s_n > MAXVSTAKEN) bug("sigenvrms_dsp");
}

static void sigenvrms_tick(t_sigenvrms *x)	/* callback function for the clock */
{
    outlet_float(x->x_outlet, sqrtf(x->x_result));
}

static void sigenvrms_ff(t_sigenvrms *x)		/* cleanup on free */
{
    clock_free(x->x_clock);
    freebytes(x->x_buf, (x->x_npoints + MAXVSTAKEN) * sizeof(float));
}

static void sigenvrms_help(void)
{
	post("envrms~\t:: envelope follower that does output rms instead of dB");
}


void sigenvrms_setup(void )
{
    sigenvrms_class = class_new(gensym("envrms~"), (t_newmethod)sigenvrms_new,
    	(t_method)sigenvrms_ff, sizeof(t_sigenvrms), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(sigenvrms_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigenvrms_class, (t_method)sigenvrms_dsp, gensym("dsp"), 0);

	class_addmethod(sigenvrms_class, (t_method)sigenvrms_help, gensym("help"), 0);
	class_sethelpsymbol(sigenvrms_class, gensym("zexy/envrms~"));
}

/* ------------------------ average~ ----------------------------- */

/* tilde object to take absolute value. */

static t_class *avg_class;

typedef struct _avg
{
    t_object x_obj;

	t_float n_inv;
	t_float buf;
	int blocks;
} t_avg;


/* average :: arithmetic mean of one signal-vector */

static t_int *avg_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);

	t_avg *x = (t_avg *)w[2];
    int n = (int)(w[3]);

	t_float buf = 0.;

    while (n--)
    {
		buf += *in++;
    }
    outlet_float(x->x_obj.ob_outlet, buf*x->n_inv);

    return (w+4);
}

static void avg_dsp(t_avg *x, t_signal **sp)
{
	x->n_inv=1./sp[0]->s_n;
    dsp_add(avg_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void *avg_new(void)
{
    t_avg *x = (t_avg *)pd_new(avg_class);
    outlet_new(&x->x_obj, gensym("float"));
    return (x);
}

static void avg_help(void)
{
	post("avg~\t:: outputs the arithmetic mean of each signal-vector");
}


void avg_setup(void)
{
    avg_class = class_new(gensym("avg~"), (t_newmethod)avg_new, 0,
    	sizeof(t_avg), 0, A_DEFFLOAT, 0);
    class_addmethod(avg_class, nullfn, gensym("signal"), 0);
    class_addmethod(avg_class, (t_method)avg_dsp, gensym("dsp"), 0);

	class_addmethod(avg_class, (t_method)avg_help, gensym("help"), 0);
	class_sethelpsymbol(avg_class, gensym("zexy/avg~"));
}


/* triggered average :: arithmetic mean between last and current BANG */

static t_class *tavg_class;

typedef struct _tavg
{
    t_object x_obj;

	t_float n_inv;
	t_float buf;
	int blocks;
} t_tavg;



static void tavg_bang(t_avg *x)
{
	if (x->blocks) {
		outlet_float(x->x_obj.ob_outlet, x->buf*x->n_inv/x->blocks);
		x->blocks = 0;
		x->buf = 0.;
	}
}

static t_int *tavg_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
	t_tavg *x = (t_tavg *)w[2];
    int n = (int)(w[3]);

	t_float buf = x->buf;

    while (n--) buf += *in++;

	x->buf = buf;
	x->blocks++;

    return (w+4);
}

static void tavg_dsp(t_tavg *x, t_signal **sp)
{
	x->n_inv=1./sp[0]->s_n;
    dsp_add(tavg_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void *tavg_new(void)
{
    t_tavg *x = (t_tavg *)pd_new(tavg_class);
    outlet_new(&x->x_obj, gensym("float"));
    return (x);
}

static void tavg_help(void)
{
	post("tavg~\t\t:: outputs the arithmetic mean of a signal when triggered");
	post("<bang>\t\t:  triggers the output");
}

void tavg_setup(void)
{
    tavg_class = class_new(gensym("tavg~"), (t_newmethod)tavg_new, 0,
    	sizeof(t_tavg), 0, A_DEFFLOAT, 0);
    class_addmethod(tavg_class, nullfn, gensym("signal"), 0);
    class_addmethod(tavg_class, (t_method)tavg_dsp, gensym("dsp"), 0);

	class_addbang(tavg_class, tavg_bang);

	class_addmethod(tavg_class, (t_method)tavg_help, gensym("help"), 0);
	class_sethelpsymbol(tavg_class, gensym("zexy/tavg~"));
}

/* global setup routine */

void z_sigaverage_setup(void)
{
	avg_setup();
	tavg_setup();
	sigenvrms_setup();
}
