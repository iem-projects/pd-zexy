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
	finally :: some of the missing binops for signals :: abs~, sgn~, >~, <~, ==~, &&~, ||~

	1302:forum::für::umläute:2000
*/

#include "zexy.h"
#include <math.h>

typedef struct _misc
{
  t_object x_obj;
} t_misc;


/* ------------------------ sigABS~ ----------------------------- */

static t_class *sigABS_class;

static t_int *sigABS_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);
  
  while (n--) *out++ = fabsf(*in++);
  
  return (w+4);
}

static void sigABS_dsp(t_misc *x, t_signal **sp)
{
  dsp_add(sigABS_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void sigABS_helper(void)
{
  post("\n%c abs~ \t\t:: absolute value of a signal", HEARTSYMBOL);
}

static void *sigABS_new(void)
{
  t_misc *x = (t_misc *)pd_new(sigABS_class);
  outlet_new(&x->x_obj, gensym("signal"));

  return (x);
}

static void sigABS_setup(void)
{
  sigABS_class = class_new(gensym("abs~"), (t_newmethod)sigABS_new, 0,
			   sizeof(t_misc), 0, A_DEFFLOAT, 0);
  class_addmethod(sigABS_class, nullfn, gensym("signal"), 0);
  class_addmethod(sigABS_class, (t_method)sigABS_dsp, gensym("dsp"), 0);
  
  class_addmethod(sigABS_class, (t_method)sigABS_helper, gensym("help"), 0);
  
  class_sethelpsymbol(sigABS_class, gensym("zexy/sigbinops+"));
}

/* ------------------------ sgn~ ----------------------------- */

static t_class *sigSGN_class;

static t_int *sigSGN_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  int n = (int)(w[3]);
  t_float x;
  
  while (n--) {
    if ((x=*in++)>0.) *out++=1.;
    else if	(x<0.) *out++=-1.;
    else *out++=0.;
  }
  
  return (w+4);
}

static void sigSGN_dsp(t_misc *x, t_signal **sp)
{
  dsp_add(sigSGN_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void sigSGN_helper(void)
{
  post("\n%c sgn~ \t\t:: sign of a signal", HEARTSYMBOL);
}

static void *sigSGN_new()
{
  t_misc *x = (t_misc *)pd_new(sigSGN_class);
  outlet_new(&x->x_obj, gensym("signal"));
  
  return (x);
}

static void sigSGN_setup(void)
{
  sigSGN_class = class_new(gensym("sgn~"), (t_newmethod)sigSGN_new, 0,
			   sizeof(t_misc), 0, A_DEFFLOAT, 0);
  class_addmethod(sigSGN_class, nullfn, gensym("signal"), 0);
  class_addmethod(sigSGN_class, (t_method)sigSGN_dsp, gensym("dsp"), 0);
  
  class_addmethod(sigSGN_class, (t_method)sigSGN_helper, gensym("help"), 0);
  class_sethelpsymbol(sigSGN_class, gensym("zexy/sigbinops+"));
}

/* ------------------------ relational~ ----------------------------- */

/* ----------------------------- sigGRT ----------------------------- */
static t_class *sigGRT_class, *scalarsigGRT_class;

typedef struct _sigGRT
{
  t_object x_obj;
  float x_f;
} t_sigGRT;

typedef struct _scalarsigGRT
{
  t_object x_obj;
  float x_f;
  t_float x_g;    	    /* inlet value */
} t_scalarsigGRT;

static void *sigGRT_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc > 1) post(">~: extra arguments ignored");
  if (argc) 
    {
      t_scalarsigGRT *x = (t_scalarsigGRT *)pd_new(scalarsigGRT_class);
      floatinlet_new(&x->x_obj, &x->x_g);
      x->x_g = atom_getfloatarg(0, argc, argv);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
  else
    {
      t_sigGRT *x = (t_sigGRT *)pd_new(sigGRT_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
}

t_int *sigGRT_perform(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = *in1++ > *in2++; 
  return (w+5);
}

t_int *sigGRT_perf8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
      float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
      float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];
      
      float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
      float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];
      
      out[0] = f0 > g0; out[1] = f1 > g1; out[2] = f2 > g2; out[3] = f3 > g3;
      out[4] = f4 > g4; out[5] = f5 > g5; out[6] = f6 > g6; out[7] = f7 > g7;
    }
  return (w+5);
}

t_int *scalarsigGRT_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float f = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = *in++ > f; 
  return (w+5);
}

t_int *scalarsigGRT_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float g = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in += 8, out += 8)
    {
      float f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
      float f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];
      
      out[0] = f0 > g; out[1] = f1 > g; out[2] = f2 > g; out[3] = f3 > g;
      out[4] = f4 > g; out[5] = f5 > g; out[6] = f6 > g; out[7] = f7 > g;
    }
  return (w+5);
}

void dsp_add_sigGRT(t_sample *in1, t_sample *in2, t_sample *out, int n)
{
  if (n&7)
    dsp_add(sigGRT_perform, 4, in1, in2, out, n);
  else	
    dsp_add(sigGRT_perf8, 4, in1, in2, out, n);
}

static void sigGRT_dsp(t_sigGRT *x, t_signal **sp)
{
  dsp_add_sigGRT(sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void scalarsigGRT_dsp(t_scalarsigGRT *x, t_signal **sp)
{
  if (sp[0]->s_n&7)
    dsp_add(scalarsigGRT_perform, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
  else	
    dsp_add(scalarsigGRT_perf8, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
}

static void sigGRT_setup(void)
{
  sigGRT_class = class_new(gensym(">~"), (t_newmethod)sigGRT_new, 0,
			   sizeof(t_sigGRT), 0, A_GIMME, 0);
  class_addmethod(sigGRT_class, (t_method)sigGRT_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(sigGRT_class, t_sigGRT, x_f);
  class_sethelpsymbol(sigGRT_class, gensym("zexy/sigbinops+"));
  scalarsigGRT_class = class_new(gensym(">~"), 0, 0,
				 sizeof(t_scalarsigGRT), 0, 0);
  CLASS_MAINSIGNALIN(scalarsigGRT_class, t_scalarsigGRT, x_f);
  class_addmethod(scalarsigGRT_class, (t_method)scalarsigGRT_dsp, gensym("dsp"),
		  0);
  class_sethelpsymbol(scalarsigGRT_class, gensym("zexy/sigbinops+"));
}


/* ----------------------------- sigLESS ----------------------------- */
static t_class *sigLESS_class, *scalarsigLESS_class;

typedef struct _sigLESS
{
  t_object x_obj;
  float x_f;
} t_sigLESS;

typedef struct _scalarsigLESS
{
  t_object x_obj;
  float x_f;
  t_float x_g;    	    /* inlet value */
} t_scalarsigLESS;

static void *sigLESS_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc > 1) post("<~: extra arguments ignored");
  if (argc) 
    {
      t_scalarsigLESS *x = (t_scalarsigLESS *)pd_new(scalarsigLESS_class);
      floatinlet_new(&x->x_obj, &x->x_g);
      x->x_g = atom_getfloatarg(0, argc, argv);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
  else
    {
      t_sigLESS *x = (t_sigLESS *)pd_new(sigLESS_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
}

t_int *sigLESS_perform(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = *in1++ < *in2++; 
  return (w+5);
}

t_int *sigLESS_perf8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
      float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
      float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];
      
      float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
      float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];
      
      out[0] = f0 < g0; out[1] = f1 < g1; out[2] = f2 < g2; out[3] = f3 < g3;
      out[4] = f4 < g4; out[5] = f5 < g5; out[6] = f6 < g6; out[7] = f7 < g7;
    }
  return (w+5);
}

t_int *scalarsigLESS_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float f = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = *in++ < f; 
  return (w+5);
}

t_int *scalarsigLESS_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float g = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in += 8, out += 8)
    {
      float f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
      float f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];
      
      out[0] = f0 < g; out[1] = f1 < g; out[2] = f2 < g; out[3] = f3 < g;
      out[4] = f4 < g; out[5] = f5 < g; out[6] = f6 < g; out[7] = f7 < g;
    }
  return (w+5);
}

void dsp_add_sigLESS(t_sample *in1, t_sample *in2, t_sample *out, int n)
{
  if (n&7)
    dsp_add(sigLESS_perform, 4, in1, in2, out, n);
  else	
    dsp_add(sigLESS_perf8, 4, in1, in2, out, n);
}

static void sigLESS_dsp(t_sigLESS *x, t_signal **sp)
{
  dsp_add_sigLESS(sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void scalarsigLESS_dsp(t_scalarsigLESS *x, t_signal **sp)
{
  if (sp[0]->s_n&7)
    dsp_add(scalarsigLESS_perform, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
  else	
    dsp_add(scalarsigLESS_perf8, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
}

static void sigLESS_setup(void)
{
  sigLESS_class = class_new(gensym("<~"), (t_newmethod)sigLESS_new, 0,
			    sizeof(t_sigLESS), 0, A_GIMME, 0);
  class_addmethod(sigLESS_class, (t_method)sigLESS_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(sigLESS_class, t_sigLESS, x_f);
  class_sethelpsymbol(sigLESS_class, gensym("zexy/sigbinops+"));
  scalarsigLESS_class = class_new(gensym("<~"), 0, 0,
				  sizeof(t_scalarsigLESS), 0, 0);
  CLASS_MAINSIGNALIN(scalarsigLESS_class, t_scalarsigLESS, x_f);
  class_addmethod(scalarsigLESS_class, (t_method)scalarsigLESS_dsp, gensym("dsp"),
		  0);
  class_sethelpsymbol(scalarsigLESS_class, gensym("zexy/sigbinops+"));
}

/* ----------------------------- sigEQUAL ----------------------------- */
static t_class *sigEQUAL_class, *scalarsigEQUAL_class;

typedef struct _sigEQUAL
{
  t_object x_obj;
  float x_f;
} t_sigEQUAL;

typedef struct _scalarsigEQUAL
{
  t_object x_obj;
  float x_f;
  t_float x_g;    	    /* inlet value */
} t_scalarsigEQUAL;

static void *sigEQUAL_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc > 1) post("==~: extra arguments ignored");
  if (argc) 
    {
      t_scalarsigEQUAL *x = (t_scalarsigEQUAL *)pd_new(scalarsigEQUAL_class);
      floatinlet_new(&x->x_obj, &x->x_g);
      x->x_g = atom_getfloatarg(0, argc, argv);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
  else
    {
      t_sigEQUAL *x = (t_sigEQUAL *)pd_new(sigEQUAL_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
}

t_int *sigEQUAL_perform(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = (*in1++ == *in2++); 
  return (w+5);
}

t_int *sigEQUAL_perf8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
      float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
      float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];
      
      float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
      float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];
      
      out[0] = f0 == g0; out[1] = f1 == g1; out[2] = f2 == g2; out[3] = f3 == g3;
      out[4] = f4 == g4; out[5] = f5 == g5; out[6] = f6 == g6; out[7] = f7 == g7;
    }
  return (w+5);
}

t_int *scalarsigEQUAL_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float f = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = (*in++ == f); 
  return (w+5);
}

t_int *scalarsigEQUAL_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float g = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in += 8, out += 8)
    {
      float f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
      float f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

      out[0] = (f0 == g); out[1] = (f1 == g); out[2] = (f2 == g); out[3] = (f3 == g);
      out[4] = (f4 == g); out[5] = (f5 == g); out[6] = (f6 == g); out[7] = (f7 == g);
    }
  return (w+5);
}

void dsp_add_sigEQUAL(t_sample *in1, t_sample *in2, t_sample *out, int n)
{
  if (n&7)
    dsp_add(sigEQUAL_perform, 4, in1, in2, out, n);
  else	
    dsp_add(sigEQUAL_perf8, 4, in1, in2, out, n);
}

static void sigEQUAL_dsp(t_sigEQUAL *x, t_signal **sp)
{
  dsp_add_sigEQUAL(sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void scalarsigEQUAL_dsp(t_scalarsigEQUAL *x, t_signal **sp)
{
  if (sp[0]->s_n&7)
    dsp_add(scalarsigEQUAL_perform, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
  else	
    dsp_add(scalarsigEQUAL_perf8, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
}

static void sigEQUAL_setup(void)
{
  sigEQUAL_class = class_new(gensym("==~"), (t_newmethod)sigEQUAL_new, 0,
			     sizeof(t_sigEQUAL), 0, A_GIMME, 0);
  class_addmethod(sigEQUAL_class, (t_method)sigEQUAL_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(sigEQUAL_class, t_sigEQUAL, x_f);
  class_sethelpsymbol(sigEQUAL_class, gensym("zexy/sigbinops+"));
  scalarsigEQUAL_class = class_new(gensym("==~"), 0, 0,
				   sizeof(t_scalarsigEQUAL), 0, 0);
  CLASS_MAINSIGNALIN(scalarsigEQUAL_class, t_scalarsigEQUAL, x_f);
  class_addmethod(scalarsigEQUAL_class, (t_method)scalarsigEQUAL_dsp, gensym("dsp"),
		  0);
  class_sethelpsymbol(scalarsigEQUAL_class, gensym("zexy/sigbinops+"));
}

/* ------------------------ logical~ ----------------------------- */

/* ----------------------------- sigAND ----------------------------- */
static t_class *sigAND_class, *scalarsigAND_class;

typedef struct _sigAND
{
  t_object x_obj;
  float x_f;
} t_sigAND;

typedef struct _scalarsigAND
{
  t_object x_obj;
  float x_f;
  t_float x_g;    	    /* inlet value */
} t_scalarsigAND;

static void *sigAND_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc > 1) post("&&~: extra arguments ignored");
  if (argc) 
    {
      t_scalarsigAND *x = (t_scalarsigAND *)pd_new(scalarsigAND_class);
      floatinlet_new(&x->x_obj, &x->x_g);
      x->x_g = atom_getfloatarg(0, argc, argv);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
  else
    {
      t_sigAND *x = (t_sigAND *)pd_new(sigAND_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
}

t_int *sigAND_perform(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = (int)*in1++ && (int)*in2++; 
  return (w+5);
}

t_int *sigAND_perf8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
      int f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
      int f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

      int g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
      int g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];

      out[0] = f0 && g0; out[1] = f1 && g1; out[2] = f2 && g2; out[3] = f3 && g3;
      out[4] = f4 && g4; out[5] = f5 && g5; out[6] = f6 && g6; out[7] = f7 && g7;
    }
  return (w+5);
}

t_int *scalarsigAND_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float f = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = (int)*in++ && (int)f; 
  return (w+5);
}

t_int *scalarsigAND_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  int g = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in += 8, out += 8)
    {
      int f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
      int f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

      out[0] = f0 && g; out[1] = f1 && g; out[2] = f2 && g; out[3] = f3 && g;
      out[4] = f4 && g; out[5] = f5 && g; out[6] = f6 && g; out[7] = f7 && g;
    }
  return (w+5);
}

void dsp_add_sigAND(t_sample *in1, t_sample *in2, t_sample *out, int n)
{
  if (n&7)
    dsp_add(sigAND_perform, 4, in1, in2, out, n);
  else	
    dsp_add(sigAND_perf8, 4, in1, in2, out, n);
}

static void sigAND_dsp(t_sigAND *x, t_signal **sp)
{
  dsp_add_sigAND(sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void scalarsigAND_dsp(t_scalarsigAND *x, t_signal **sp)
{
  if (sp[0]->s_n&7)
    dsp_add(scalarsigAND_perform, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
  else	
    dsp_add(scalarsigAND_perf8, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
}

static void sigAND_setup(void)
{
  sigAND_class = class_new(gensym("&&~"), (t_newmethod)sigAND_new, 0,
			   sizeof(t_sigAND), 0, A_GIMME, 0);
  class_addmethod(sigAND_class, (t_method)sigAND_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(sigAND_class, t_sigAND, x_f);
  class_sethelpsymbol(sigAND_class, gensym("zexy/sigbinops+"));
  scalarsigAND_class = class_new(gensym("&&~"), 0, 0,
				 sizeof(t_scalarsigAND), 0, 0);
  CLASS_MAINSIGNALIN(scalarsigAND_class, t_scalarsigAND, x_f);
  class_addmethod(scalarsigAND_class, (t_method)scalarsigAND_dsp, gensym("dsp"),
		  0);
  class_sethelpsymbol(scalarsigAND_class, gensym("zexy/sigbinops+"));
}


/* ----------------------------- sigOR ----------------------------- */
static t_class *sigOR_class, *scalarsigOR_class;

typedef struct _sigOR
{
  t_object x_obj;
  float x_f;
} t_sigOR;

typedef struct _scalarsigOR
{
  t_object x_obj;
  float x_f;
  t_float x_g;    	    /* inlet value */
} t_scalarsigOR;

static void *sigOR_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc > 1) post("||~: extra arguments ignored");
  if (argc) 
    {
      t_scalarsigOR *x = (t_scalarsigOR *)pd_new(scalarsigOR_class);
      floatinlet_new(&x->x_obj, &x->x_g);
      x->x_g = atom_getfloatarg(0, argc, argv);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
  else
    {
      t_sigOR *x = (t_sigOR *)pd_new(sigOR_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
      outlet_new(&x->x_obj, &s_signal);
      x->x_f = 0;
      return (x);
    }
}

t_int *sigOR_perform(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = (int)*in1++ || (int)*in2++; 
  return (w+5);
}

t_int *sigOR_perf8(t_int *w)
{
  t_float *in1 = (t_float *)(w[1]);
  t_float *in2 = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
    {
      int f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
      int f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

      int g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
      int g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];

      out[0] = f0 || g0; out[1] = f1 || g1; out[2] = f2 || g2; out[3] = f3 || g3;
      out[4] = f4 || g4; out[5] = f5 || g5; out[6] = f6 || g6; out[7] = f7 || g7;
    }
  return (w+5);
}

t_int *scalarsigOR_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  int f = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  while (n--) *out++ = (int)*in++ || f; 
  return (w+5);
}

t_int *scalarsigOR_perf8(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  int g = *(t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  for (; n; n -= 8, in += 8, out += 8)
    {
      int f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
      int f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];

      out[0] = f0 || g; out[1] = f1 || g; out[2] = f2 || g; out[3] = f3 || g;
      out[4] = f4 || g; out[5] = f5 || g; out[6] = f6 || g; out[7] = f7 || g;
    }
  return (w+5);
}

void dsp_add_sigOR(t_sample *in1, t_sample *in2, t_sample *out, int n)
{
  if (n&7)
    dsp_add(sigOR_perform, 4, in1, in2, out, n);
  else	
    dsp_add(sigOR_perf8, 4, in1, in2, out, n);
}

static void sigOR_dsp(t_sigOR *x, t_signal **sp)
{
  dsp_add_sigOR(sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void scalarsigOR_dsp(t_scalarsigOR *x, t_signal **sp)
{
  if (sp[0]->s_n&7)
    dsp_add(scalarsigOR_perform, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
  else	
    dsp_add(scalarsigOR_perf8, 4, sp[0]->s_vec, &x->x_g,
	    sp[1]->s_vec, sp[0]->s_n);
}

static void sigOR_setup(void)
{
  sigOR_class = class_new(gensym("||~"), (t_newmethod)sigOR_new, 0,
			  sizeof(t_sigOR), 0, A_GIMME, 0);
  class_addmethod(sigOR_class, (t_method)sigOR_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(sigOR_class, t_sigOR, x_f);
  class_sethelpsymbol(sigOR_class, gensym("zexy/sigbinops+"));
  scalarsigOR_class = class_new(gensym("||~"), 0, 0,
				sizeof(t_scalarsigOR), 0, 0);
  CLASS_MAINSIGNALIN(scalarsigOR_class, t_scalarsigOR, x_f);
  class_addmethod(scalarsigOR_class, (t_method)scalarsigOR_dsp, gensym("dsp"),
		  0);
  class_sethelpsymbol(scalarsigOR_class, gensym("zexy/sigbinops+"));
}



/* ---------------------- global setup ------------------------- */


void z_sigbin_setup(void)
{
  sigABS_setup();
  sigSGN_setup();
  sigGRT_setup();
  sigLESS_setup();
  sigEQUAL_setup();
  sigOR_setup();
  sigAND_setup();
  zexy_register("sigbin");
}
void z_z_sigbin_setup(void)
{
  /* ok, looks a bit like nonsense... */
  z_sigbin_setup();
}
