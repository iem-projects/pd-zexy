#include "zexy.h"
#include <math.h>

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#define atan2f atan2
#define sqrtf sqrt
#define sinf sin
#define cosf cos
#endif

#ifdef MACOSX
#define atan2f atan2
#define sqrtf sqrt
#define sinf sin
#define cosf cos
#endif

/* ----------------------- deg/rad utils ----------------- */
t_class *deg2rad_class, *rad2deg_class;
typedef struct _deg2rad
{
  t_object x_obj;
  t_float factor;
} t_deg2rad;

/* deg2rad :: degree 2 radian */

static void deg2rad_float(t_deg2rad *x, t_float f)
{  
  outlet_float(x->x_obj.ob_outlet, x->factor*f);
}

static void *deg2rad_new(t_floatarg f)
{
  t_deg2rad *x = (t_deg2rad *)pd_new(deg2rad_class);
  outlet_new(&x->x_obj, gensym("float"));
  x->factor=atan2f(1,1)/45.0;

  return (x);
}

static void deg2rad_help(void)
{
  post("deg2rad\t:: convert degree 2 radians");
}

static void deg2rad_setup(void)
{
  deg2rad_class = class_new(gensym("deg2rad"), (t_newmethod)deg2rad_new, 0,
			 sizeof(t_deg2rad), 0, A_DEFFLOAT, 0);

  class_addmethod(deg2rad_class, (t_method)deg2rad_help, gensym("help"), 0);
  class_addfloat(deg2rad_class, deg2rad_float);
  class_sethelpsymbol(deg2rad_class, gensym("zexy/deg2rad"));
}

/* rad2deg :: radian 2 degree */
t_class *rad2deg_class;

static void rad2deg_float(t_deg2rad *x, t_float f)
{  
  outlet_float(x->x_obj.ob_outlet, x->factor*f);
}

static void *rad2deg_new(t_floatarg f)
{
  t_deg2rad *x = (t_deg2rad *)pd_new(rad2deg_class);
  outlet_new(&x->x_obj, gensym("float"));
  x->factor=45.0/atan2f(1,1);

  return (x);
}

static void rad2deg_help(void)
{
  post("rad2deg\t:: convert radian 2 degree");
}

static void rad2deg_setup(void)
{
  rad2deg_class = class_new(gensym("rad2deg"), (t_newmethod)rad2deg_new, 0,
			 sizeof(t_deg2rad), 0, A_DEFFLOAT, 0);

  class_addmethod(rad2deg_class, (t_method)rad2deg_help, gensym("help"), 0);
  class_addfloat(rad2deg_class, rad2deg_float);
  class_sethelpsymbol(rad2deg_class, gensym("zexy/deg2rad"));
}

/* ------------------------ coordinate transformations ----------------------------- */

typedef struct _coordinates
{
  t_object x_obj;

  t_outlet *out[3];
  t_float old_coord[3], new_coord[3];
} t_coordinates;

void coordinates_free(t_coordinates *x)
{
}

void coord_bang(t_coordinates *x)
{
  int i=3;
  while(i--)outlet_float(x->out[i], x->new_coord[i]);
}


/* cart2pol :: cartesian to polar coordinates */
t_class *cart2pol_class;

static void cart2pol_bang(t_coordinates *x)
{
  t_float X=x->old_coord[0], Y=x->old_coord[1];
  x->new_coord[0]=sqrtf(X*X+Y*Y);   /* R   */
  x->new_coord[1]=atan2f(Y, X);     /* PHI */
  x->new_coord[2]=x->old_coord[2];  /* Z   */
  coord_bang(x);
}

static void cart2pol_float(t_coordinates *x, t_float f)
{  
  x->old_coord[0]=f;
  cart2pol_bang(x);
}

static void *cart2pol_new(t_floatarg X, t_floatarg Y, t_floatarg Z)
{
  t_coordinates *x = (t_coordinates *)pd_new(cart2pol_class);
  int i=3;
  floatinlet_new(&x->x_obj, &x->old_coord[1]);
  floatinlet_new(&x->x_obj, &x->old_coord[2]);
  while(i--){
    x->out[2-i]=outlet_new(&x->x_obj, gensym("float"));
    x->new_coord[i]=0;
  }
  x->old_coord[0]=X;
  x->old_coord[1]=Y;
  x->old_coord[2]=Z;

  return (x);
}

static void cart2pol_help(void)
{
  post("cart2pol\t:: convert cartesian to polar coordinates");
  post("\t\"<x> <y> <z>\": returns <r> <phi> <z>");
}

static void cart2pol_setup(void)
{
  cart2pol_class = class_new(gensym("cart2pol"), (t_newmethod)cart2pol_new, (t_method)coordinates_free,
			 sizeof(t_coordinates), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(cart2pol_class, (t_method)cart2pol_help, gensym("help"), 0);
  class_addfloat(cart2pol_class, cart2pol_float);
  class_addbang(cart2pol_class, cart2pol_bang);

  class_sethelpsymbol(cart2pol_class, gensym("zexy/cart2pol"));
}


/* pol2cart :: polar to cartesian coordinates */
t_class *pol2cart_class;

static void pol2cart_bang(t_coordinates *x)
{
  x->new_coord[0]=x->old_coord[0]*cosf(x->old_coord[1]);  /* X */
  x->new_coord[1]=x->old_coord[0]*sinf(x->old_coord[1]);  /* Y */
  x->new_coord[2]=x->old_coord[2];                        /* Z */
  coord_bang(x);
}

static void pol2cart_float(t_coordinates *x, t_float f)
{  
  x->old_coord[0]=f;
  pol2cart_bang(x);
}

static void *pol2cart_new(t_floatarg X, t_floatarg Y, t_floatarg Z)
{
  t_coordinates *x = (t_coordinates *)pd_new(pol2cart_class);
  int i=3;
  floatinlet_new(&x->x_obj, &x->old_coord[1]);
  floatinlet_new(&x->x_obj, &x->old_coord[2]);
  while(i--){
    x->out[2-i]=outlet_new(&x->x_obj, gensym("float"));
    x->new_coord[i]=0;
  }
  x->old_coord[0]=X;
  x->old_coord[1]=Y;
  x->old_coord[2]=Z;
  return (x);
}

static void pol2cart_help(void)
{
  post("pol2cart\t:: convert polar to cartesian coordinates");
  post("\t\"<r> <phi> <z>\": returns <x> <x> <z>");
}

static void pol2cart_setup(void)
{
  pol2cart_class = class_new(gensym("pol2cart"), (t_newmethod)pol2cart_new, (t_method)coordinates_free,
			 sizeof(t_coordinates), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(pol2cart_class, (t_method)pol2cart_help, gensym("help"), 0);
  class_addfloat(pol2cart_class, pol2cart_float);
  class_addbang(pol2cart_class, pol2cart_bang);

  class_sethelpsymbol(pol2cart_class, gensym("zexy/pol2cart"));
}

/* cart2sph :: cartesian to sphar coordinates */
t_class *cart2sph_class;

static void cart2sph_bang(t_coordinates *x)
{
  t_float X=x->old_coord[0], Y=x->old_coord[1], Z=x->old_coord[2];
  x->new_coord[0]=sqrtf(X*X+Y*Y+Z*Z);       /* R     */
  x->new_coord[1]=atan2f(Y, X);             /* PHI   */
  x->new_coord[2]=atan2f(Z, sqrt(X*X+Y*Y)); /* THETA */
  coord_bang(x);
}

static void cart2sph_float(t_coordinates *x, t_float f)
{  
  x->old_coord[0]=f;
  cart2sph_bang(x);
}

static void *cart2sph_new(t_floatarg X, t_floatarg Y, t_floatarg Z)
{
  t_coordinates *x = (t_coordinates *)pd_new(cart2sph_class);
  int i=3;
  floatinlet_new(&x->x_obj, &x->old_coord[1]);
  floatinlet_new(&x->x_obj, &x->old_coord[2]);
  while(i--){
    x->out[2-i]=outlet_new(&x->x_obj, gensym("float"));
    x->new_coord[i]=0;
  }
  x->old_coord[0]=X;
  x->old_coord[1]=Y;
  x->old_coord[2]=Z;
  return (x);
}

static void cart2sph_help(void)
{
  post("cart2sph\t:: convert cartesian to sphar coordinates");
  post("\t\"<x> <y> <z>\": returns <r> <phi> <theta>");
}

static void cart2sph_setup(void)
{
  cart2sph_class = class_new(gensym("cart2sph"), (t_newmethod)cart2sph_new, (t_method)coordinates_free,
			 sizeof(t_coordinates), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(cart2sph_class, (t_method)cart2sph_help, gensym("help"), 0);
  class_addfloat(cart2sph_class, cart2sph_float);
  class_addbang(cart2sph_class, cart2sph_bang);

  class_sethelpsymbol(cart2sph_class, gensym("zexy/cart2sph"));
}


/* sph2cart :: sphar to cartesian coordinates */
t_class *sph2cart_class;

static void sph2cart_bang(t_coordinates *x)
{
  x->new_coord[0]=x->old_coord[0]*cosf(x->old_coord[1])*cosf(x->old_coord[2]);  /* X */
  x->new_coord[1]=x->old_coord[0]*sinf(x->old_coord[1])*cosf(x->old_coord[2]);  /* Y */
  x->new_coord[2]=x->old_coord[0]*sinf(x->old_coord[2]);                        /* Z */
  coord_bang(x);
}

static void sph2cart_float(t_coordinates *x, t_float f)
{  
  x->old_coord[0]=f;
  sph2cart_bang(x);
}

static void *sph2cart_new(t_floatarg X, t_floatarg Y, t_floatarg Z)
{
  t_coordinates *x = (t_coordinates *)pd_new(sph2cart_class);
  int i=3;
  floatinlet_new(&x->x_obj, &x->old_coord[1]);
  floatinlet_new(&x->x_obj, &x->old_coord[2]);
  while(i--){
    x->out[2-i]=outlet_new(&x->x_obj, gensym("float"));
    x->new_coord[i]=0;
  }
  x->old_coord[0]=X;
  x->old_coord[1]=Y;
  x->old_coord[2]=Z;
  return (x);
}

static void sph2cart_help(void)
{
  post("sph2cart\t:: convert sphar to cartesian coordinates");
  post("\t\"<r> <phi> <theta>\": returns <x> <y> <z>");
}

static void sph2cart_setup(void)
{
  sph2cart_class = class_new(gensym("sph2cart"), (t_newmethod)sph2cart_new, (t_method)coordinates_free,
			 sizeof(t_coordinates), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(sph2cart_class, (t_method)sph2cart_help, gensym("help"), 0);
  class_addfloat(sph2cart_class, sph2cart_float);
  class_addbang(sph2cart_class, sph2cart_bang);

  class_sethelpsymbol(sph2cart_class, gensym("zexy/sph2cart"));
}


/* pol2sph :: polesian to sphar coordinates */
t_class *pol2sph_class;

static void pol2sph_bang(t_coordinates *x)
{
  t_float r=x->old_coord[0], z=x->old_coord[2];
  x->new_coord[0]=sqrtf(r*r+z*z);   /* R     */
  x->new_coord[1]=x->old_coord[1];  /* PHI   */
  x->new_coord[2]=atan2f(z,r);      /* THETA */
  coord_bang(x);
}

static void pol2sph_float(t_coordinates *x, t_float f)
{  
  x->old_coord[0]=f;
  pol2sph_bang(x);
}

static void *pol2sph_new(t_floatarg X, t_floatarg Y, t_floatarg Z)
{
  t_coordinates *x = (t_coordinates *)pd_new(pol2sph_class);
  int i=3;
  floatinlet_new(&x->x_obj, &x->old_coord[1]);
  floatinlet_new(&x->x_obj, &x->old_coord[2]);
  while(i--){
    x->out[2-i]=outlet_new(&x->x_obj, gensym("float"));
    x->new_coord[i]=0;
  }
  x->old_coord[0]=X;
  x->old_coord[1]=Y;
  x->old_coord[2]=Z;
  return (x);
}

static void pol2sph_help(void)
{
  post("pol2sph\t:: convert polar to spheric coordinates");
  post("\t\"<r> <phi> <z>\": returns <r> <phi> <theta>");
}

static void pol2sph_setup(void)
{
  pol2sph_class = class_new(gensym("pol2sph"), (t_newmethod)pol2sph_new, (t_method)coordinates_free,
			 sizeof(t_coordinates), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(pol2sph_class, (t_method)pol2sph_help, gensym("help"), 0);
  class_addfloat(pol2sph_class, pol2sph_float);
  class_addbang(pol2sph_class, pol2sph_bang);

  class_sethelpsymbol(pol2sph_class, gensym("zexy/pol2sph"));
}


/* sph2pol :: sphar to polesian coordinates */
t_class *sph2pol_class;

static void sph2pol_bang(t_coordinates *x)
{
  x->new_coord[0]=x->old_coord[0]*cosf(x->old_coord[2]); /*  R  */
  x->new_coord[1]=x->old_coord[1];                       /* PHI */
  x->new_coord[2]=x->old_coord[0]*sinf(x->old_coord[2]); /*  Z  */

  coord_bang(x);
}

static void sph2pol_float(t_coordinates *x, t_float f)
{  
  x->old_coord[0]=f;
  sph2pol_bang(x);
}

static void *sph2pol_new(t_floatarg X, t_floatarg Y, t_floatarg Z)
{
  t_coordinates *x = (t_coordinates *)pd_new(sph2pol_class);
  int i=3;
  floatinlet_new(&x->x_obj, &x->old_coord[1]);
  floatinlet_new(&x->x_obj, &x->old_coord[2]);
  while(i--){
    x->out[2-i]=outlet_new(&x->x_obj, gensym("float"));
    x->new_coord[i]=0;
  }
  x->old_coord[0]=X;
  x->old_coord[1]=Y;
  x->old_coord[2]=Z;
  return (x);
}

static void sph2pol_help(void)
{
  post("sph2pol\t:: convert spherical to polar coordinates");
  post("\t\"<r> <phi> <theta>\": returns <r> <phi> <z>");
}

static void sph2pol_setup(void)
{
  sph2pol_class = class_new(gensym("sph2pol"), (t_newmethod)sph2pol_new, (t_method)coordinates_free,
			 sizeof(t_coordinates), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(sph2pol_class, (t_method)sph2pol_help, gensym("help"), 0);
  class_addfloat(sph2pol_class, sph2pol_float);
  class_addbang(sph2pol_class, sph2pol_bang);

  class_sethelpsymbol(sph2pol_class, gensym("zexy/sph2pol"));
}

/* global setup routine */

void z_coordinates_setup(void)
{
  cart2pol_setup();
  pol2cart_setup();
  cart2sph_setup();
  sph2cart_setup();
   pol2sph_setup();
   sph2pol_setup();

   deg2rad_setup();
   rad2deg_setup();
}
