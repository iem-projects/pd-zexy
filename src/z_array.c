/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* sampling */

#include "m_pd.h"

/* ------------------------- table16 -------------------------- */
/* a 16bit table */
static t_class *table16_class;
typedef struct _table16 {
  t_object x_obj;

  t_symbol *x_tablename;
  long      x_size;
  short    *x_table; // hold the data

  int x_usedindsp;
} t_table16;

static void *table16_new(t_symbol *s, t_float f){
  t_table16 *x = (t_table16*)pd_new(table16_class);
  int i=f;
  if(i<1)i=1;
  x->x_tablename=s;
  x->x_size=i;
  x->x_table=getbytes(x->x_size*sizeof(short));
  x->x_usedindsp=0;
  pd_bind(&x->x_obj.ob_pd, x->x_tablename);
  return(x);
}

static void table16_free(t_table16 *x){
  if(x->x_table)freebytes(x->x_table, x->x_size*sizeof(short));
  pd_unbind(&x->x_obj.ob_pd, x->x_tablename);
}

static int table16_getarray16(t_table16*x, int*size,short**vec){
  *size=x->x_size;
  *vec =x->x_table;
  return 1;
}
static void table16_usedindsp(t_table16*x){
  x->x_usedindsp=1;
}
 
static void table16_setup(void){
  table16_class = class_new(gensym("table16"),
			    (t_newmethod)table16_new, (t_method)table16_free,
			    sizeof(t_table16), 0, A_DEFSYM, A_DEFFLOAT, 0);
}


/* ------------------------- tab16write~ -------------------------- */

static t_class *tab16write_tilde_class;

typedef struct _tab16write_tilde
{
    t_object x_obj;
    int x_phase;
    int x_nsampsintab;
    short *x_vec;
    t_symbol *x_arrayname;
    float x_f;
} t_tab16write_tilde;

static void *tab16write_tilde_new(t_symbol *s)
{
    t_tab16write_tilde *x = (t_tab16write_tilde *)pd_new(tab16write_tilde_class);
    x->x_phase = 0x7fffffff;
    x->x_arrayname = s;
    x->x_f = 0;
    return (x);
}

static t_int *tab16write_tilde_perform(t_int *w)
{
    t_tab16write_tilde *x = (t_tab16write_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]), phase = x->x_phase, endphase = x->x_nsampsintab;
    if (!x->x_vec) goto bad;
    
    if (endphase > phase)
    {
    	int nxfer = endphase - phase;
    	short *fp = x->x_vec + phase;
    	if (nxfer > n) nxfer = n;
    	phase += nxfer;
    	while (nxfer--)
	{
	    float f = *in++;
    	    if (PD_BADFLOAT(f))
	    	f = 0;
	    *fp++ = f;
    	}
    	x->x_phase = phase;
    }
bad:
    return (w+4);
}

void tab16write_tilde_set(t_tab16write_tilde *x, t_symbol *s)
{
    t_table16 *a;

    x->x_arrayname = s;
    if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    {
    	if (*s->s_name) pd_error(x, "tab16write~: %s: no such array",
    	    x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!table16_getarray16(a, &x->x_nsampsintab, &x->x_vec))
    {
    	error("%s: bad template for tab16write~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else table16_usedindsp(a);
}

static void tab16write_tilde_dsp(t_tab16write_tilde *x, t_signal **sp)
{
    tab16write_tilde_set(x, x->x_arrayname);
    dsp_add(tab16write_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void tab16write_tilde_bang(t_tab16write_tilde *x)
{
    x->x_phase = 0;
}

static void tab16write_tilde_stop(t_tab16write_tilde *x)
{}

static void tab16write_tilde_free(t_tab16write_tilde *x)
{}

static void tab16write_tilde_setup(void)
{
    tab16write_tilde_class = class_new(gensym("tab16write~"),
    	(t_newmethod)tab16write_tilde_new, (t_method)tab16write_tilde_free,
    	sizeof(t_tab16write_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tab16write_tilde_class, t_tab16write_tilde, x_f);
    class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_dsp,
    	gensym("dsp"), 0);
    class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_set,
    	gensym("set"), A_SYMBOL, 0);
    class_addmethod(tab16write_tilde_class, (t_method)tab16write_tilde_stop,
    	gensym("stop"), 0);
    class_addbang(tab16write_tilde_class, tab16write_tilde_bang);
}

/* ------------ tab16play~ - non-transposing sample playback --------------- */

static t_class *tab16play_tilde_class;

typedef struct _tab16play_tilde
{
    t_object x_obj;
    t_outlet *x_bangout;
    int x_phase;
    int x_nsampsintab;
    int x_limit;
    short *x_vec;
    t_symbol *x_arrayname;
} t_tab16play_tilde;

static void *tab16play_tilde_new(t_symbol *s)
{
    t_tab16play_tilde *x = (t_tab16play_tilde *)pd_new(tab16play_tilde_class);
    x->x_phase = 0x7fffffff;
    x->x_limit = 0;
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_signal);
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    return (x);
}

static t_int *tab16play_tilde_perform(t_int *w)
{
    t_tab16play_tilde *x = (t_tab16play_tilde *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    short *fp;
    int n = (int)(w[3]), phase = x->x_phase,
    	endphase = (x->x_nsampsintab < x->x_limit ?
	    x->x_nsampsintab : x->x_limit), nxfer, n3;
    if (!x->x_vec || phase >= endphase)
    	goto zero;
    
    nxfer = endphase - phase;
    fp = x->x_vec + phase;
    if (nxfer > n)
    	nxfer = n;
    n3 = n - nxfer;
    phase += nxfer;
    while (nxfer--)
    	*out++ = *fp++;
    if (phase >= endphase)
    {
    	x->x_phase = 0x7fffffff;
	while (n3--)
	    *out++ = 0;
    }
    else x->x_phase = phase;
    
    return (w+4);
zero:
    while (n--) *out++ = 0;
    return (w+4);
}

void tab16play_tilde_set(t_tab16play_tilde *x, t_symbol *s)
{
    t_table16 *a;

    x->x_arrayname = s;
    if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    {
    	if (*s->s_name) pd_error(x, "tab16play~: %s: no such array",
    	    x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!table16_getarray16(a, &x->x_nsampsintab, &x->x_vec))
    {
    	error("%s: bad template for tab16play~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else table16_usedindsp(a);
}

static void tab16play_tilde_dsp(t_tab16play_tilde *x, t_signal **sp)
{
    tab16play_tilde_set(x, x->x_arrayname);
    dsp_add(tab16play_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void tab16play_tilde_list(t_tab16play_tilde *x, t_symbol *s,
    int argc, t_atom *argv)
{
    long start = atom_getfloatarg(0, argc, argv);
    long length = atom_getfloatarg(1, argc, argv);
    if (start < 0) start = 0;
    if (length <= 0)
    	x->x_limit = 0x7fffffff;
    else
    	x->x_limit = start + length;
    x->x_phase = start;
}

static void tab16play_tilde_stop(t_tab16play_tilde *x)
{
    x->x_phase = 0x7fffffff;
}

static void tab16play_tilde_free(t_tab16play_tilde *x)
{}

static void tab16play_tilde_setup(void)
{
    tab16play_tilde_class = class_new(gensym("tab16play~"),
    	(t_newmethod)tab16play_tilde_new, (t_method)tab16play_tilde_free,
    	sizeof(t_tab16play_tilde), 0, A_DEFSYM, 0);
    class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_dsp,
    	gensym("dsp"), 0);
    class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_stop,
    	gensym("stop"), 0);
    class_addmethod(tab16play_tilde_class, (t_method)tab16play_tilde_set,
    	gensym("set"), A_DEFSYM, 0);
    class_addlist(tab16play_tilde_class, tab16play_tilde_list);
}

/******************** tab16read~ ***********************/

static t_class *tab16read_tilde_class;

typedef struct _tab16read_tilde
{
    t_object x_obj;
    int x_npoints;
    short *x_vec;
    t_symbol *x_arrayname;
    float x_f;
} t_tab16read_tilde;

static void *tab16read_tilde_new(t_symbol *s)
{
    t_tab16read_tilde *x = (t_tab16read_tilde *)pd_new(tab16read_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *tab16read_tilde_perform(t_int *w)
{
    t_tab16read_tilde *x = (t_tab16read_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);    
    int maxindex;
    short *buf = x->x_vec;
    int i;
    
    maxindex = x->x_npoints - 1;
    if (!buf) goto zero;

    for (i = 0; i < n; i++)
    {
	int index = *in++;
	if (index < 0)
	    index = 0;
	else if (index > maxindex)
	    index = maxindex;
	*out++ = buf[index];
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

void tab16read_tilde_set(t_tab16read_tilde *x, t_symbol *s)
{
    t_table16 *a;
    
    x->x_arrayname = s;
    if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    {
    	if (*s->s_name)
    	    error("tab16read~: %s: no such array", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!table16_getarray16(a, &x->x_npoints, &x->x_vec))
    {
    	error("%s: bad template for tab16read~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else table16_usedindsp(a);
}

static void tab16read_tilde_dsp(t_tab16read_tilde *x, t_signal **sp)
{
    tab16read_tilde_set(x, x->x_arrayname);

    dsp_add(tab16read_tilde_perform, 4, x,
    	sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tab16read_tilde_free(t_tab16read_tilde *x)
{
}

static void tab16read_tilde_setup(void)
{
    tab16read_tilde_class = class_new(gensym("tab16read~"),
    	(t_newmethod)tab16read_tilde_new, (t_method)tab16read_tilde_free,
    	sizeof(t_tab16read_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tab16read_tilde_class, t_tab16read_tilde, x_f);
    class_addmethod(tab16read_tilde_class, (t_method)tab16read_tilde_dsp,
    	gensym("dsp"), 0);
    class_addmethod(tab16read_tilde_class, (t_method)tab16read_tilde_set,
    	gensym("set"), A_SYMBOL, 0);
}

/******************** tab16read4~ ***********************/

static t_class *tab16read4_tilde_class;

typedef struct _tab16read4_tilde
{
    t_object x_obj;
    int x_npoints;
    short *x_vec;
    t_symbol *x_arrayname;
    float x_f;
} t_tab16read4_tilde;

static void *tab16read4_tilde_new(t_symbol *s)
{
    t_tab16read4_tilde *x = (t_tab16read4_tilde *)pd_new(tab16read4_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *tab16read4_tilde_perform(t_int *w)
{
    t_tab16read4_tilde *x = (t_tab16read4_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);    
    int maxindex;
    short *buf = x->x_vec;
    short *fp;
    int i;
    
    maxindex = x->x_npoints - 3;

    if (!buf) goto zero;

#if 0	    /* test for spam -- I'm not ready to deal with this */
    for (i = 0,  xmax = 0, xmin = maxindex,  fp = in1; i < n; i++,  fp++)
    {
	float f = *in1;
	if (f < xmin) xmin = f;
	else if (f > xmax) xmax = f;
    }
    if (xmax < xmin + x->c_maxextent) xmax = xmin + x->c_maxextent;
    for (i = 0, splitlo = xmin+ x->c_maxextent, splithi = xmax - x->c_maxextent,
	fp = in1; i < n; i++,  fp++)
    {
	float f = *in1;
	if (f > splitlo && f < splithi) goto zero;
    }
#endif

    for (i = 0; i < n; i++)
    {
	float findex = *in++;
	int index = findex;
	float frac,  a,  b,  c,  d, cminusb;
	if (index < 1)
	    index = 1, frac = 0;
	else if (index > maxindex)
	    index = maxindex, frac = 1;
	else frac = findex - index;
	fp = buf + index;
	a = fp[-1];
	b = fp[0];
	c = fp[1];
	d = fp[2];
	/* if (!i && !(count++ & 1023))
	    post("fp = %lx,  shit = %lx,  b = %f",  fp, buf->b_shit,  b); */
	cminusb = c-b;
	*out++ = b + frac * (
	    cminusb - 0.5f * (frac-1.) * (
		(a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
	    )
	);
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

void tab16read4_tilde_set(t_tab16read4_tilde *x, t_symbol *s)
{
    t_table16 *a;
    
    x->x_arrayname = s;
    if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    {
    	if (*s->s_name)
    	    error("tab16read4~: %s: no such array", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else if (!table16_getarray16(a, &x->x_npoints, &x->x_vec))
    {
    	error("%s: bad template for tab16read4~", x->x_arrayname->s_name);
    	x->x_vec = 0;
    }
    else table16_usedindsp(a);
}

static void tab16read4_tilde_dsp(t_tab16read4_tilde *x, t_signal **sp)
{
    tab16read4_tilde_set(x, x->x_arrayname);

    dsp_add(tab16read4_tilde_perform, 4, x,
    	sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tab16read4_tilde_free(t_tab16read4_tilde *x)
{
}

static void tab16read4_tilde_setup(void)
{
    tab16read4_tilde_class = class_new(gensym("tab16read4~"),
    	(t_newmethod)tab16read4_tilde_new, (t_method)tab16read4_tilde_free,
    	sizeof(t_tab16read4_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tab16read4_tilde_class, t_tab16read4_tilde, x_f);
    class_addmethod(tab16read4_tilde_class, (t_method)tab16read4_tilde_dsp,
    	gensym("dsp"), 0);
    class_addmethod(tab16read4_tilde_class, (t_method)tab16read4_tilde_set,
    	gensym("set"), A_SYMBOL, 0);
}
/* ---------- tab16read: control, non-interpolating ------------------------ */

static t_class *tab16read_class;

typedef struct _tab16read
{
    t_object x_obj;
    t_symbol *x_arrayname;
} t_tab16read;

static void tab16read_float(t_tab16read *x, t_float f)
{
    t_table16 *a;
    int npoints;
    short *vec;

    if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    	error("%s: no such array", x->x_arrayname->s_name);
    else if (!table16_getarray16(a, &npoints, &vec))
    	error("%s: bad template for tab16read", x->x_arrayname->s_name);
    else
    {
    	int n = f;
    	if (n < 0) n = 0;
    	else if (n >= npoints) n = npoints - 1;
    	outlet_float(x->x_obj.ob_outlet, (npoints ? vec[n] : 0));
    }
}

static void tab16read_set(t_tab16read *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *tab16read_new(t_symbol *s)
{
    t_tab16read *x = (t_tab16read *)pd_new(tab16read_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void tab16read_setup(void)
{
    tab16read_class = class_new(gensym("tab16read"), (t_newmethod)tab16read_new,
    	0, sizeof(t_tab16read), 0, A_DEFSYM, 0);
    class_addfloat(tab16read_class, (t_method)tab16read_float);
    class_addmethod(tab16read_class, (t_method)tab16read_set, gensym("set"),
    	A_SYMBOL, 0);
}

/* ---------- tab16read4: control, non-interpolating ------------------------ */

static t_class *tab16read4_class;

typedef struct _tab16read4
{
    t_object x_obj;
    t_symbol *x_arrayname;
} t_tab16read4;

static void tab16read4_float(t_tab16read4 *x, t_float f)
{
    t_table16 *a;
    int npoints;
    short *vec;

    if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    	error("%s: no such array", x->x_arrayname->s_name);
    else if (!table16_getarray16(a, &npoints, &vec))
    	error("%s: bad template for tab16read4", x->x_arrayname->s_name);
    else if (npoints < 4)
    	outlet_float(x->x_obj.ob_outlet, 0);
    else if (f <= 1)
    	outlet_float(x->x_obj.ob_outlet, vec[1]);
    else if (f >= npoints - 2)
    	outlet_float(x->x_obj.ob_outlet, vec[npoints - 2]);
    else
    {
    	int n = f;
	float a, b, c, d, cminusb, frac;
	short *fp;
	if (n >= npoints - 2)
	    n = npoints - 3;
	fp = vec + n;
	frac = f - n;
	a = fp[-1];
	b = fp[0];
	c = fp[1];
	d = fp[2];
	cminusb = c-b;
	outlet_float(x->x_obj.ob_outlet, b + frac * (
	    cminusb - 0.5f * (frac-1.) * (
		(a - d + 3.0f * cminusb) * frac + (b - a - cminusb))));
    }
}

static void tab16read4_set(t_tab16read4 *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void *tab16read4_new(t_symbol *s)
{
    t_tab16read4 *x = (t_tab16read4 *)pd_new(tab16read4_class);
    x->x_arrayname = s;
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void tab16read4_setup(void)
{
    tab16read4_class = class_new(gensym("tab16read4"), (t_newmethod)tab16read4_new,
    	0, sizeof(t_tab16read4), 0, A_DEFSYM, 0);
    class_addfloat(tab16read4_class, (t_method)tab16read4_float);
    class_addmethod(tab16read4_class, (t_method)tab16read4_set, gensym("set"),
    	A_SYMBOL, 0);
}

/* ------------------ tab16write: control ------------------------ */

static t_class *tab16write_class;

typedef struct _tab16write
{
    t_object x_obj;
    t_symbol *x_arrayname;
    float x_ft1;
    int x_set;
} t_tab16write;

static void tab16write_float(t_tab16write *x, t_float f)
{
    int vecsize;
    t_table16 *a;
    short *vec;

    if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))
    	error("%s: no such array", x->x_arrayname->s_name);
    else if (!table16_getarray16(a, &vecsize, &vec))
    	error("%s: bad template for tab16write", x->x_arrayname->s_name);
    else
    {
    	int n = x->x_ft1;
    	if (n < 0) n = 0;
    	else if (n >= vecsize) n = vecsize-1;
    	vec[n] = f;
    }
}

static void tab16write_set(t_tab16write *x, t_symbol *s)
{
    x->x_arrayname = s;
}

static void tab16write_free(t_tab16write *x)
{}

static void *tab16write_new(t_symbol *s)
{
    t_tab16write *x = (t_tab16write *)pd_new(tab16write_class);
    x->x_ft1 = 0;
    x->x_arrayname = s;
    floatinlet_new(&x->x_obj, &x->x_ft1);
    return (x);
}

void tab16write_setup(void)
{
    tab16write_class = class_new(gensym("tab16write"), (t_newmethod)tab16write_new,
    	(t_method)tab16write_free, sizeof(t_tab16write), 0, A_DEFSYM, 0);
    class_addfloat(tab16write_class, (t_method)tab16write_float);
    class_addmethod(tab16write_class, (t_method)tab16write_set, gensym("set"), A_SYMBOL, 0);
}

/* ------------------------ global setup routine ------------------------- */

void z_array_setup(void)
{
    tab16write_tilde_setup();
    tab16play_tilde_setup();
    tab16read_tilde_setup();
    tab16read4_tilde_setup();
    tab16read_setup();
    tab16read4_setup();
    tab16write_setup();
    table16_setup();
}

