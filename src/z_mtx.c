/* 1605:forum::für::umläute:2001 */

/* objects for manipulating matrices */
/* mostly i refer to matlab/octave matrix functions */

/*
  matrix : basic object : create and store matrices
  mtx    : alias for matrix

  mtx_resize
  mtx_row
  mtx_col
  mtx_element

  mtx_ones
  mtx_zeros
  mtx_eye
  mtx_egg

  mtx_diag
  mtx_diegg
  mtx_trace

  mtx_mean
  mtx_rand

  mtx_transpose
  mtx_scroll
  mtx_roll

  mtx_add
  mtx_+
  mtx_mul
  mtx_*
  mtx_.*
  mtx_./

  mtx_inverse
  mtx_pivot

  mtx_size

  mtx_check
  mtx_print
*/

#include "zexy.h"
#include <math.h>

#ifdef NT
#include <memory.h>
#include <string.h>
#define fabsf fabs
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#define VALID_MTX 12450

typedef struct _matrix
{
  int row;
  int col;

  int valid;

  t_float *buffer;

} t_matrix;

/* intern utility functions */
static void MTX_setdimen(t_matrix *x, int row, int col)
{
  x->col = col;
  x->row = row;
}

static void MTX_adjustsize(t_matrix *m, int desiredRow, int desiredCol)
{
  int col=m->col, row=m->row;

  if (desiredRow<1){
    post("cannot make less than 1 rows");
    desiredRow=1;
  }
  if (desiredCol<1){
    post("cannot make less than 1 columns");
    desiredCol=1;
  }

  if (col*row!=desiredRow*desiredCol){
    if(m->buffer)freebytes(m->buffer, (col*row)*sizeof(t_float));
    m->buffer=(t_float *)getbytes((desiredCol*desiredRow)*sizeof(t_float));
  }

  MTX_setdimen(m, desiredRow, desiredCol);
  return;
}

static void MTX_debugmtx(t_matrix *m, int id)
{
  int i=m->col;
  t_float *buf = m->buffer;
  while(i--){
	  int j=m->row;
	  startpost("debug%d: ", id);
	  while(j--)
		  startpost("%f  ", *buf++);
	  endpost();
  }
}

static void MTX_freematrix(t_matrix *x)
{
  freebytes(x->buffer, x->row*x->col*sizeof(t_float));
  x->row = x->col = x->valid = 0;
}

static void outlet_matrix(t_outlet *x, t_matrix *m)
{
  t_gpointer *gp=(t_gpointer *)m;
  outlet_pointer(x, gp);
}
static t_matrix *get_inmatrix(t_gpointer *gp)
{
  t_matrix *m = (t_matrix *)gp;
  if (m->valid != VALID_MTX){
    error("matrix: no valid matrix passed !");
    return 0;
  }
  if ((m->col <= 0) || (m->row <= 0)){
    error("matrix: strange dimensions");
    return 0;
  }
  return m;
}
static void MTX_set(t_matrix *m, t_float f)
{
  t_float *buf=m->buffer;
  int size = m->col*m->row;
  if(buf)while(size--)*buf++=f;
}

/* -------------------- matrix ------------------------------ */

static t_class *matrix_class;

typedef struct _mtx
{
  t_object x_obj;

  t_matrix *m;

  int     current_row, current_col;  /* this makes things easy for the mtx_row & mtx_col...*/

  t_canvas *x_canvas; /* and for reading / writing */
} t_mtx;

/* core functions */

static void matrix_bang(t_mtx *x)
{ /* output the matrix */
  post("matrix %dx%d @ %x", x->m->row, x->m->col, x->m->buffer);
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_matrix2(t_mtx *x, t_gpointer *gp)
{
  t_matrix *m = get_inmatrix(gp);
  if(!m)return;

  if (x->m->row*x->m->col != m->row*m->col) {
    freebytes(x->m->buffer, x->m->row*x->m->col*sizeof(t_float));
    x->m->buffer = copybytes(m->buffer, m->row*m->col*sizeof(t_float));
  } else memcpy(x->m->buffer, m->buffer, m->row*m->col*sizeof(t_float));

  MTX_setdimen(x->m, m->row, m->col);
}
static void matrix_matrix(t_mtx *x, t_gpointer *gp)
{
  t_matrix *m = get_inmatrix(gp);
  if(!m)return;

  matrix_matrix2(x, gp);
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}


/* basic functions */

static void matrix_zeros(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  switch(argc) {
  case 0: /* zero out the actual matrix */
    break;
  case 1:
    row=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, row);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, col);
  }
  MTX_set(x->m, 0);
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_ones(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  switch(argc) {
  case 0: /* zero out the actual matrix */
    break;
  case 1:
    row=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, row);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, col);
  }
  MTX_set(x->m, 1);
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_eye(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row, n;
  switch(argc) {
  case 0: /* zero out the actual matrix */
    break;
  case 1:
    row=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, row);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, col);
  }
  MTX_set(x->m, 0);

  col=x->m->col;
  row=x->m->row;
  n = (col<row)?col:row;
  while(n--)x->m->buffer[n*(1+col)]=1;
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_egg(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row, n;
  switch(argc) {
  case 0: /* zero out the actual matrix */
    break;
  case 1:
    row=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, row);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    MTX_adjustsize(x->m, row, col);
  }
  MTX_set(x->m, 0);

  col=x->m->col;
  row=x->m->row;
  n = (col<row)?col:row;
  while(n--)x->m->buffer[(n+1)*(col-1)]=1;
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_diag(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int col=argc;
  argv+=argc-1;
  if (argc<1) {
    post("matrix: no diagonale present");
    return;
  }
  MTX_adjustsize(x->m, argc, argc);
  MTX_set(x->m, 0);

  while(argc--)x->m->buffer[argc*(1+col)]=atom_getfloat(argv--);
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_diegg(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int col=argc;
  argv+=argc-1;
  if (argc<1) {
    post("matrix: no dieggonale present");
    return;
  }
  MTX_adjustsize(x->m, argc, argc);
  MTX_set(x->m, 0);
  while(argc--)x->m->buffer[(argc+1)*(col-1)]=atom_getfloat(argv--);
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_float(t_mtx *x, t_float f)
{
  MTX_set(x->m, f);
  outlet_matrix(x->x_obj.ob_outlet, x->m);
}
static void matrix_row(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap=(t_atom *)getbytes(sizeof(t_atom)*x->m->col);
  t_float *mtx_flt;
  int row=x->m->row, col=x->m->col;
  int r, c;
  t_float f;

  switch (argc){
  case 0: /* show all rows as a list of floats */
   for (r=0;r<row;r++){
     mtx_flt = x->m->buffer+r*col;
     argv = ap;
     c=col;
     while(c--){
       SETFLOAT(argv, *mtx_flt);
       argv++;
     }
     outlet_list(x->x_obj.ob_outlet, gensym("row"), col, ap);
    }
    break;
  case 1: /* show this row as a list of floats */
    r=atom_getfloat(argv)-1;
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    argv=ap;
    mtx_flt=x->m->buffer+r*col;
    c=col;
    while(c--){
      SETFLOAT(argv, *mtx_flt);
      argv++;
     }
    outlet_list(x->x_obj.ob_outlet, gensym("row"), col, ap);
    break;
  case 2: /* set this row to a constant value */
    r=atom_getfloat(argv)-1;
    f=atom_getfloat(argv+1);
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    mtx_flt = x->m->buffer+r*col;
    c=col;
    while(c--)*mtx_flt++=f;
    outlet_matrix(x->x_obj.ob_outlet, x->m);
  default: /* set this row to new values */
    r=atom_getfloat(argv++)-1;
    if (argc--<col){
      post("matrix: sparse rows not yet supported : use \"mtx_check\"");
      return;
    }
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    mtx_flt=x->m->buffer+col*r;
    c=col;
    while(c--)*mtx_flt++=atom_getfloat(argv++);
    outlet_matrix(x->x_obj.ob_outlet, x->m);
  }
  freebytes(ap, x->m->col*sizeof(t_atom));
}
static void matrix_col(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float *mtx_flt;
  int row=x->m->row, col=x->m->col;
  int c=0, r=0;
  t_float f;
  t_atom *ap=(t_atom *)getbytes(row*sizeof(t_atom));

  switch (argc){
  case 0: /* show all columns as a list of floats */
    for (c=0;c<col;c++) {
      mtx_flt = x->m->buffer+c;

      for (r=0;r<row;r++)SETFLOAT(&ap[r], mtx_flt[col*r]);
      outlet_list(x->x_obj.ob_outlet, gensym("col"), row, ap);
    }
    break;
  case 1:/* show this column as a list of floats */
    c=atom_getfloat(argv)-1;
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    mtx_flt = x->m->buffer+c;
 
    for (r=0;r<row;r++)SETFLOAT(&ap[r],x->m->buffer[col*r]);
    outlet_list(x->x_obj.ob_outlet, gensym("col"), row, ap);
    break;
  case 2: /* set this column to a constant value */
    c=atom_getfloat(argv)-1;
    f=atom_getfloat(argv+1);
    if ((c<0)||(c>=row)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    mtx_flt = x->m->buffer+r*col;
    c=col;
    while(c--){
      *mtx_flt=f;
      mtx_flt+=col;
    }
    outlet_matrix(x->x_obj.ob_outlet, x->m);
  default:/* set this column to new values */
    c=atom_getfloat(argv++)-1;
    if (argc--<row){
      post("matrix: sparse cols not yet supported : use \"mtx_check\"");
      return;
    }
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    mtx_flt=x->m->buffer+c;
    r=row;
    while(r--){
      *mtx_flt=atom_getfloat(argv++);
      mtx_flt+=col;
    }
    outlet_matrix(x->x_obj.ob_outlet, x->m);
  }
  freebytes(ap, row*sizeof(t_atom));
}
static void matrix_element(t_mtx *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float *mtx_flt=x->m->buffer;
  int row=x->m->row, col=x->m->col;
  int r, c, i=row*col;

  switch (argc){
  case 0:
    while(i--)outlet_float(x->x_obj.ob_outlet, *mtx_flt++);
    break;
  case 1:
    r=c=atom_getfloat(argv)-1;
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    outlet_float(x->x_obj.ob_outlet, mtx_flt[c+r*col]);
    break;
  case 2:
    r=atom_getfloat(argv++)-1;
    c=atom_getfloat(argv++)-1;
    if ((r<0)||(r>=row)){      post("matrix: row index %d is out of range", r+1);      return;    }
    if ((c<0)||(c>=col)){      post("matrix: col index %d is out of range", c+1);      return;    }
    outlet_float(x->x_obj.ob_outlet, mtx_flt[c+r*col]);
    break;
  default:
    r=atom_getfloat(argv++)-1;
    c=atom_getfloat(argv++)-1;
    if ((r<0)||(r>=row)){      post("matrix: row index %d is out of range", r+1);      return;    }
    if ((c<0)||(c>=col)){      post("matrix: col index %d is out of range", c+1);      return;    }
    mtx_flt[c+r*col]=atom_getfloat(argv);
    outlet_matrix(x->x_obj.ob_outlet, x->m);
  }
}

/* ------------- file I/O ------------------ */
static void matrix_read(t_mtx *x, t_symbol *filename)
{
  t_binbuf *bbuf = binbuf_new();
  t_atom *ap;
  int n;

  if (binbuf_read_via_path(bbuf, filename->s_name, canvas_getdir(x->x_canvas)->s_name, 0))
    error("matrix: failed to read %s", filename->s_name);

  ap=binbuf_getvec(bbuf);
  n =binbuf_getnatom(bbuf)-1;
  
  if ((ap->a_type == A_SYMBOL) && !strcmp(ap->a_w.w_symbol->s_name,"matrix") ){
    /* ok, this looks like a matrix */
    int row = atom_getfloat(ap+1);
    int col = atom_getfloat(ap+2);
    t_float *mtx_flt;
    if (n-2<row*col){
      error("matrix: sparse matrices not supported");
      binbuf_free(bbuf);
      return;
    }
    if (row<1 || col<1){
      error("matrix: illegal matrix dimensions");
      binbuf_free(bbuf);
      return;
    }

    MTX_adjustsize(x->m, row, col);
    ap+=2;
    mtx_flt=x->m->buffer;
    n=row*col;
    while(n--)*mtx_flt++=atom_getfloat(ap++);
  }

  binbuf_free(bbuf);
}
static void matrix_write(t_mtx *x, t_symbol *filename)
{
  t_binbuf *bbuf = binbuf_new();
  t_atom atom;
  t_float *mtx_flt=x->m->buffer;
  char buf[MAXPDSTRING];
  int n;
  int r = x->m->row;
  int c = x->m->col;
  canvas_makefilename(x->x_canvas, filename->s_name, buf, MAXPDSTRING);

  SETSYMBOL(&atom, gensym("matrix"));
  binbuf_add(bbuf, 1, &atom);
  SETFLOAT (&atom, x->m->row);
  binbuf_add(bbuf, 1, &atom);
  SETFLOAT (&atom, x->m->col);
  binbuf_add(bbuf, 1, &atom);
  binbuf_addsemi(bbuf);
  n=x->m->row*x->m->col;
  while(r--){
    c=x->m->col;
    while(c--){
      SETFLOAT(&atom, *mtx_flt++);
      binbuf_add(bbuf, 1, &atom);
    }
    binbuf_addsemi(bbuf);
  }

  if (binbuf_write(bbuf, buf, "", 1)){
    error("matrix: failed to write %s", filename->s_name);
  }

  binbuf_free(bbuf);
}

/* ----------------- setup ------------------- */

static void matrix_free(t_mtx *x)
{
  MTX_freematrix(x->m);
  freebytes(x->m, sizeof(t_matrix));
}
static void *matrix_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mtx *x = (t_mtx *)pd_new(matrix_class);
  int row, col;

  x->m = (t_matrix *)getbytes(sizeof(t_matrix));

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("pointer"), gensym(""));
  outlet_new(&x->x_obj, 0);

  x->m->buffer= 0;
  x->m->row = x->m->col = 0;
  x->m->valid = VALID_MTX;
  x->x_canvas = canvas_getcurrent();

  switch (argc) {
  case 0:
    row = col = 0;
    break;
  case 1:
    if (argv->a_type == A_SYMBOL) {
      matrix_read(x, argv->a_w.w_symbol);
      return(x);
    }
    row = col = atom_getfloat(argv);
    break;
  default:
    row = atom_getfloat(argv++);
    col = atom_getfloat(argv++);
  }

  if(row*col){
    MTX_adjustsize(x->m, row, col);
    MTX_set(x->m, 0);
  }

  return (x);
}

static void matrix_setup(void)
{
  matrix_class = class_new(gensym("mtx"), (t_newmethod)matrix_new, 
			   (t_method)matrix_free, sizeof(t_mtx), 0, A_GIMME, 0);

  /* the core : functions for matrices */
  class_addmethod  (matrix_class, (t_method)matrix_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_matrix2, gensym(""), A_GIMME, 0);
 
  /* the basics : functions for creation */
  class_addmethod  (matrix_class, (t_method)matrix_eye, gensym("eye"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_diag, gensym("diag"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_ones, gensym("ones"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_zeros, gensym("zeros"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_egg, gensym("egg"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_diegg, gensym("diegg"), A_GIMME, 0);
 
  /* the rest : functions for anything */
  class_addbang    (matrix_class, matrix_bang);
  class_addfloat   (matrix_class, matrix_float);
  //  class_addlist    (matrix_class, matrix_list);
  class_addmethod  (matrix_class, (t_method)matrix_row, gensym("row"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_col, gensym("column"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_col, gensym("col"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_element, gensym("element"), A_GIMME, 0);

  /* the file functions */
  class_addmethod  (matrix_class, (t_method)matrix_write, gensym("write"), A_SYMBOL, 0);
  class_addmethod  (matrix_class, (t_method)matrix_read , gensym("read") , A_SYMBOL, 0);

 
  class_sethelpsymbol(matrix_class, gensym("zexy/mtx"));
}


/* mtx_print */

static t_class *mtx_print_class;

typedef struct _m_print
{
  t_object x_obj;

} t_m_print;
static void mtx_print(t_m_print *x, t_gpointer *gp)
{
  t_matrix *m = get_inmatrix(gp);
  int c, r;
  t_float *f;
  if (!m) return;

  c=m->col;
  r=m->row;
  f=m->buffer;

  post("matrix %dx%d @ %x", r,c, f);

  while(r--){
    c=m->col;
    while(c--)startpost("%f\t", *f++);
    endpost();
  }
  
}
static void *mtx_print_new(void)
{
  t_m_print *x = (t_m_print *)pd_new(mtx_print_class);

  return (x);
}
static void mtx_print_setup(void)
{
  mtx_print_class = class_new(gensym("m_print"), (t_newmethod)mtx_print_new, 
			      0, sizeof(t_m_print), 0, 0, 0);
  class_addpointer(mtx_print_class, mtx_print);
}


void z_mtx_setup(void)
{
  matrix_setup();
  mtx_print_setup();
}
