/* 2504:forum::für::umläute:2001 */

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

#define MY_WRITE

#define T_FLOAT long double

#include "zexy.h"
#include <math.h>

#ifdef MY_WRITE
#include <stdio.h>
#endif
#include <string.h>

#ifdef NT
#include <memory.h>
#define fabsf fabs
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


/* -------------------- matrix ------------------------------ */

static t_class *matrix_class;

typedef struct _matrix
{
  t_object x_obj;

  int      row;
  int      col;

  t_atom *atombuffer;

  int     current_row, current_col;  /* this makes things easy for the mtx_row & mtx_col...*/
  t_float f;

  t_canvas *x_canvas;
} t_matrix;

/* intern utility functions */

static void setdimen(t_matrix *x, int row, int col)
{
  x->col = col;
  x->row = row;
  SETFLOAT(x->atombuffer,   row);
  SETFLOAT(x->atombuffer+1, col);
}

static void adjustsize(t_matrix *x, int desiredRow, int desiredCol)
{
  int col=x->col, row=x->row;

  if (desiredRow<1){
    post("cannot make less than 1 rows");
    desiredRow=1;
  }
  if (desiredCol<1){
    post("cannot make less than 1 columns");
    desiredCol=1;
  }

  if (col*row!=desiredRow*desiredCol){
    if(x->atombuffer)freebytes(x->atombuffer, (col*row+2)*sizeof(t_atom));
    x->atombuffer=(t_atom *)getbytes((desiredCol*desiredRow+2)*sizeof(t_atom));
  }

  setdimen(x, desiredRow, desiredCol);
  return;
}

static void debugmtx(int argc, t_float *buf, int id)
{
  int i=argc;
  while(i--){
	  int j=argc;
	  startpost("debug%d: ", id);
	  while(j--)
		  startpost("%f  ", *buf++);
	  endpost();
  }
}
static T_FLOAT *matrix2float(t_atom *ap)
{
  int row = atom_getfloat(ap++);
  int col=atom_getfloat(ap++);
  int length   = row * col;
  T_FLOAT *buffer = (T_FLOAT *)getbytes(sizeof(T_FLOAT)*length);
  T_FLOAT *buf = buffer;
  while(length--)*buf++=atom_getfloat(ap++);
  return buffer;
}
static void float2matrix(t_atom *ap, T_FLOAT *buffer)
{
  int row=atom_getfloat(ap++);
  int col=atom_getfloat(ap++);
  int length = row * col;
  T_FLOAT*buf= buffer;
  while(length--){
    SETFLOAT(ap, *buf++);
    ap++;
  }
  freebytes(buffer, row*col*sizeof(T_FLOAT));
}


/* core functions */

static void matrix_bang(t_matrix *x)
{
  /* output the matrix */
  if (x->atombuffer)outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->col*x->row+2, x->atombuffer);
}

static void matrix_matrix2(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;

  if (argc<2){
    post("matrix : corrupt matrix passed");
    return;
  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){
    post("matrix : corrupt matrix passed");
    return;
  }
  if (row*col > argc-2){
    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }

  /* this is fast and dirty, MAYBE make it slow and clean */
  /* or, to clean matrices, use the mtx_check object */
  if (row*col != x->row*x->col) {
    freebytes(x->atombuffer, x->row*x->col*sizeof(t_atom));
    x->atombuffer = copybytes(argv, (row*col+2)*sizeof(t_atom));
  } else memcpy(x->atombuffer, argv, (row*col+2)*sizeof(t_atom));

  setdimen(x, row, col);
}

static void matrix_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;

  if (argc<2){
    post("matrix : corrupt matrix passed");
    return;
  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){
    post("matrix : corrupt matrix passed");
    return;
  }
  if (row*col > argc-2){
    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }

  matrix_matrix2(x, s, argc, argv);
  matrix_bang(x);
}


/* basic functions */

static void matrix_set(t_matrix *x, t_float f)
{
  int size = x->col * x->row;
  t_atom *buf=x->atombuffer+2;
  if(x->atombuffer)while(size--)SETFLOAT(&buf[size], f);
}

static void matrix_zeros(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 0);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 0);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);

    matrix_set(x, 0);
  }

  matrix_bang(x);
}

static void matrix_ones(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 1);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 1);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);

    matrix_set(x, 1);
  }

  matrix_bang(x);
}

static void matrix_eye(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  int n;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 0);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 0);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);
    matrix_set(x, 0);
  }

  col=x->col;
  row=x->row;
  n = (col<row)?col:row;
  while(n--)SETFLOAT(x->atombuffer+2+n*(1+col), 1);
  
  matrix_bang(x);
}
static void matrix_egg(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  int n;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 0);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 0);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);
    matrix_set(x, 0);
  }

  col=x->col;
  row=x->row;
  n = (col<row)?col:row;
  while(n--)SETFLOAT(x->atombuffer+2+(n+1)*(col-1), 1);
  
  matrix_bang(x);
}

static void matrix_diag(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col=argc;
  argv+=argc-1;
  if (argc<1) {
    post("matrix: no diagonale present");
    return;
  }
  adjustsize(x, argc, argc);
  matrix_set(x, 0);

  while(argc--)SETFLOAT(x->atombuffer+2+argc*(1+col), atom_getfloat(argv--));

  matrix_bang(x);
}
static void matrix_diegg(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col=argc;
  argv+=argc-1;
  if (argc<1) {
    post("matrix: no dieggonale present");
    return;
  }
  adjustsize(x, argc, argc);
  matrix_set(x, 0);

  while(argc--){
	  t_atom *ap=x->atombuffer+2+(argc+1)*(col-1);
	  SETFLOAT(ap, atom_getfloat(argv--));
  }

  matrix_bang(x);
}
/* the rest */

static void matrix_row(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap;
  int row=x->row, col=x->col;
  int r;
  t_float f;

  switch (argc){
  case 0:
    for (r=0;r<row;r++)outlet_list(x->x_obj.ob_outlet, gensym("row"), col, x->atombuffer+r*col+2);
    break;
  case 1:
    r=atom_getfloat(argv)-1;
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    outlet_list(x->x_obj.ob_outlet, gensym("row"), col, x->atombuffer+r*col+2);
    break;
  case 2:
    r=atom_getfloat(argv)-1;
    f=atom_getfloat(argv+1);
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    
    
  default:
    r=atom_getfloat(argv++)-1;
    if (argc--<col){
      post("matrix: sparse rows not yet supported : use \"mtx_check\"");
      return;
    }
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    if (r==row) {
    } else {
      ap=x->atombuffer+2+col*r;
      memcpy(ap, argv, col*sizeof(t_atom));
    }
  }
}

static void matrix_col(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap;
  int row=x->row, col=x->col;
  int c, r;

  switch (argc){
  case 0:
    ap=(t_atom *)getbytes(row*sizeof(t_atom));
    for (c=0;c<col;c++) {
      for (r=0;r<row;r++)SETFLOAT(&ap[r], atom_getfloat(x->atombuffer+2+c+col*r));
      outlet_list(x->x_obj.ob_outlet, gensym("col"), row, ap);
    }
    freebytes(ap, row*sizeof(t_atom));
    break;
  case 1:
    ap=(t_atom *)getbytes(row*sizeof(t_atom));
    c=atom_getfloat(argv)-1;
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    for (r=0;r<row;r++)SETFLOAT(&ap[r], atom_getfloat(x->atombuffer+2+c+col*r));
    outlet_list(x->x_obj.ob_outlet, gensym("col"), row, ap);
    freebytes(ap, row*sizeof(t_atom));
    break;
  default:
    c=atom_getfloat(argv++)-1;
    if (argc--<row){
      post("matrix: sparse cols not yet supported : use \"mtx_check\"");
      return;
    }
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    argv+=argc-1;
    if (argc>row)argc=row;
    while(argc--){
      ap=x->atombuffer+2+c+col*argc;
      SETFLOAT(ap, atom_getfloat(argv--));
    }
  }
}

static void matrix_element(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap=x->atombuffer+2;
  int row=x->row, col=x->col;
  int r, c, i=row*col;

  switch (argc){
  case 0:
    while(i--)outlet_float(x->x_obj.ob_outlet, atom_getfloat(ap++));
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
    outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->atombuffer+2+c+r*col));
    break;
  case 2:
    r=atom_getfloat(argv++)-1;
    c=atom_getfloat(argv++)-1;
    if ((r<0)||(r>=row)){      post("matrix: row index %d is out of range", r+1);      return;    }
    if ((c<0)||(c>=col)){      post("matrix: col index %d is out of range", c+1);      return;    }
    outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->atombuffer+2+c+r*col));
    break;
  default:
    r=atom_getfloat(argv++)-1;
    c=atom_getfloat(argv++)-1;
    if ((r<0)||(r>=row)){      post("matrix: row index %d is out of range", r+1);      return;    }
    if ((c<0)||(c>=col)){      post("matrix: col index %d is out of range", c+1);      return;    }
    SETFLOAT(x->atombuffer+2+c+r*col, atom_getfloat(argv));
  }
}

static void matrix_float(t_matrix *x, t_float f)
{
  matrix_set(x, f);
  matrix_bang(x);
}

/* ------------- file I/O ------------------ */

static void matrix_read(t_matrix *x, t_symbol *filename)
{
  t_binbuf *bbuf = binbuf_new();
  t_atom *ap;
  int n;

  if (binbuf_read_via_path(bbuf, filename->s_name, canvas_getdir(x->x_canvas)->s_name, 0))
    error("matrix: failed to read %s", filename->s_name);

  ap=binbuf_getvec(bbuf);
  n =binbuf_getnatom(bbuf)-1;
  
  if ((ap->a_type == A_SYMBOL) && 
      (!strcmp(ap->a_w.w_symbol->s_name,"matrix") || !strcmp(ap->a_w.w_symbol->s_name,"#matrix")) ){
    matrix_matrix2(x, gensym("matrix"), n, ap+1);
  }

  binbuf_free(bbuf);
}
#ifndef MY_WRITE
static void matrix_write(t_matrix *x, t_symbol *filename)
{
  t_binbuf *bbuf = binbuf_new();
  t_atom atom, *ap=x->atombuffer;
  char buf[MAXPDSTRING];
  int n = x->row;

  canvas_makefilename(x->x_canvas, filename->s_name, buf, MAXPDSTRING);

  /* we now write "#matrix" instead of "matrix",
   * so that these files can easily read by other 
   * applications such as octave
   */
  SETSYMBOL(&atom, gensym("#matrix"));
  binbuf_add(bbuf, 1, &atom);
  binbuf_add(bbuf, 2, ap);
  binbuf_addsemi(bbuf);
  ap+=2;
  while(n--){
    binbuf_add(bbuf, x->col, ap);
    binbuf_addsemi(bbuf);
    ap+=x->col;
  }

  if (binbuf_write(bbuf, buf, "", 1)){
    error("matrix: failed to write %s", filename->s_name);
  }

  binbuf_free(bbuf);
}
#else
static void matrix_write(t_matrix *x, t_symbol *filename)
{
  t_atom *ap=x->atombuffer+2;
  char filnam[MAXPDSTRING];
  int rows = x->row, cols = x->col;
  FILE *f=0;

  sys_bashfilename(filename->s_name, filnam);

  /* open file */
  if (!(f = fopen(filnam, "w"))) {
    error("matrix : failed to open %s", filnam);
  } else {
    char *text=(char *)getbytes(sizeof(char)*MAXPDSTRING);
    int textlen;

    /* header:
     * we now write "#matrix" instead of "matrix",
     * so that these files can easily read by other 
     * applications such as octave
     */
    sprintf(text, "#matrix %d %d\n", rows, cols);
    textlen = strlen(text);
    if (fwrite(text, textlen*sizeof(char), 1, f) < 1) {
      error("matrix : failed to write %s", filnam); goto end;
    }

    while(rows--) {
      int c = cols;
      while (c--) {
	t_float val = atom_getfloat(ap++);
	sprintf(text, "%.15f ", val);
	textlen=strlen(text);
	if (fwrite(text, textlen*sizeof(char), 1, f) < 1) {
	  error("matrix : failed to write %s", filnam); goto end;
	}
      }
      if (fwrite("\n", sizeof(char), 1, f) < 1) {
	error("matrix : failed to write %s", filnam); goto end;
      }
    }
    freebytes(text, sizeof(char)*MAXPDSTRING);
  }

 end:
  /* close file */
  if (f) fclose(f);
}
#endif

static void matrix_free(t_matrix *x)
{
  freebytes(x->atombuffer, (x->col*x->row+2)*sizeof(t_atom));
  x->atombuffer=0;
  x->col=x->row=0;
}
static void matrix_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  /* like matrix, but without col/row information, so the previous size is kept */
  int row=x->row, col=x->col;

  if(!row*col){
    post("matrix : unknown matrix dimensions");
    return;
  }
  if (argc<row*col){
    post("matrix: sparse matrices not yet supported : use \"mtx_check\" !");
    return;
  }
  
  memcpy(x->atombuffer+2, argv, row*col*sizeof(t_atom));
  matrix_bang(x);
}

static void *matrix_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(matrix_class);
  int row, col;


  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
  outlet_new(&x->x_obj, 0);

  x->atombuffer   = 0;
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
    adjustsize(x, row, col);
    matrix_set(x, 0);
  }

  return (x);
}

static void matrix_setup(void)
{
  matrix_class = class_new(gensym("matrix"), (t_newmethod)matrix_new, 
			   (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)matrix_new, gensym("mtx"), A_GIMME, 0);

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
  class_addlist    (matrix_class, matrix_list);
  class_addmethod  (matrix_class, (t_method)matrix_row, gensym("row"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_col, gensym("column"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_col, gensym("col"), A_GIMME, 0);
  class_addmethod  (matrix_class, (t_method)matrix_element, gensym("element"), A_GIMME, 0);

  /* the file functions */
  class_addmethod  (matrix_class, (t_method)matrix_write, gensym("write"), A_SYMBOL, 0);
  class_addmethod  (matrix_class, (t_method)matrix_read , gensym("read") , A_SYMBOL, 0);

 
  class_sethelpsymbol(matrix_class, gensym("zexy/matrix"));
}


/* ------------------------------------------------------------------------------------- */

/* mtx_resize */

static t_class *mtx_resize_class;
static void mtx_resize_list2(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int r, c;
  if (argc<1)return;
  if (argc>2)error("mtx_resize : only rows & cols are needed, skipping the rest");
  if (argc==1)r=c=atom_getfloat(argv++);
  else{
    r=atom_getfloat(argv++);
    c=atom_getfloat(argv++);
  }

  if (r<0)r=0;
  if (c<0)c=0;

  x->current_row = r;
  x->current_col = c;
}

static void mtx_resize_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  int r = x->current_row, c = x->current_col;
  int R=0, ROW, COL;

  if (argc<2){    post("mtx_add: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_add: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  if (!r)r=row;
  if (!c)c=col;

  if (r==row && c==col) { // no need to change
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, argv);
    return;
  }

  x->atombuffer=(t_atom *)getbytes((c*r+2)*sizeof(t_atom));
  setdimen(x, r, c);
  matrix_set(x, 0);

  ROW=(r<row)?r:row;
  COL=(c<col)?c:col;
  R=ROW;
  while(R--)memcpy(x->atombuffer+2+(ROW-R-1)*c, argv+2+(ROW-R-1)*col, COL*sizeof(t_atom));
      
  matrix_bang(x);

  freebytes(x->atombuffer, (c*r+2)*sizeof(t_atom));
}

static void *mtx_resize_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_resize_class);
  int c=0, r=0;

  if(argc){
    if(argc-1){
      r=atom_getfloat(argv);
      c=atom_getfloat(argv+1);
    } else r=c=atom_getfloat(argv);
    if(c<0)c=0;
    if(r<0)r=0;
  }
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  outlet_new(&x->x_obj, 0);
  x->current_row = r;
  x->current_col = c;
  x->row = x->col= 0;
  x->atombuffer  = 0;

  return (x);
}
static void mtx_resize_setup(void)
{
  mtx_resize_class = class_new(gensym("mtx_resize"), (t_newmethod)mtx_resize_new, 
			       0, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addmethod  (mtx_resize_class, (t_method)mtx_resize_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod  (mtx_resize_class, (t_method)mtx_resize_list2,  gensym(""), A_GIMME, 0);
  class_sethelpsymbol(mtx_resize_class, gensym("zexy/mtx_size"));
}

/* mtx_row */
static t_class *mtx_row_class;

static void mtx_row_float(t_matrix *x, t_floatarg f)
{
  int i = f;
  if(i<0)i=0;
  x->current_row = i;
}
static void mtx_row_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;
  if (argc<2){    post("matrix : corrupt matrix passed");    return;  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){    post("matrix : corrupt matrix passed");    return;  }
  if (row*col > argc-2){    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");    return;  }
  matrix_matrix2(x, s, argc, argv);
  matrix_bang(x);
}
static void mtx_row_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  if (argc==1){
    t_float f=atom_getfloat(argv);
    t_atom *ap=x->atombuffer+2+(x->current_row-1)*x->col;
    if (x->current_row>x->row){
      post("mtx_row : too high a row is to be set");
      return;
    }
    if (x->current_row){
      int n=x->col;
      while(n--){
	SETFLOAT(ap, f);
	ap++;
      }
    }
    matrix_bang(x);
    return;
  }

  if (argc<x->col){
    post("mtx_row : row length is too small for %dx%d-matrix", x->row, x->col);
    return;
  }
  if (x->current_row>x->row){
    post("mtx_row : too high a row is to be set");
    return;
  }
  if(x->current_row) {memcpy(x->atombuffer+2+(x->current_row-1)*x->col, argv, x->col*sizeof(t_atom));
  }  else {
    int r=x->row;
    while(r--)memcpy(x->atombuffer+2+r*x->col, argv, x->col*sizeof(t_atom));      
  }
  matrix_bang(x);
}
static void *mtx_row_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_row_class);
  int i, j, q;

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  x->current_row=0;
  x->col=x->row=0;
  x->atombuffer=0;
  switch (argc) {
  case 0:break;
  case 1:
    i = atom_getfloat(argv);
    if (i<0)i=0;
    if(i)adjustsize(x, i, i);
    matrix_set(x, 0);
    break;
  case 2:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    break;
  default:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    q = atom_getfloat(argv++);if(q<0)q=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    x->current_row=q;
  }
  return (x);
}
static void mtx_row_setup(void)
{
  mtx_row_class = class_new(gensym("mtx_row"), (t_newmethod)mtx_row_new, 
			    (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_row_class, matrix_bang);
  class_addlist  (mtx_row_class, mtx_row_list);
  class_addmethod(mtx_row_class, (t_method)mtx_row_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_row_class, (t_method)mtx_row_float, gensym(""), A_FLOAT, 0);
  class_sethelpsymbol(mtx_row_class, gensym("zexy/mtx_element"));
}


/* mtx_col */
static t_class *mtx_col_class;

static void mtx_col_float(t_matrix *x, t_floatarg f)
{
  int i = f;
  if(i<0)i=0;
  x->current_col = i;
}
static void mtx_col_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;
  if (argc<2){    post("matrix : corrupt matrix passed");    return;  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){    post("matrix : corrupt matrix passed");    return;  }
  if (row*col > argc-2){    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");    return;  }
  matrix_matrix2(x, s, argc, argv);
  matrix_bang(x);
}
static void mtx_col_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  if (argc==1){
    t_float f=atom_getfloat(argv);
    t_atom *ap=x->atombuffer+1+x->current_col;
    if (x->current_col>x->col){
      post("mtx_col : too high a column is to be set");
      return;
    }
    if (x->current_col){
      int n=x->row;
      while(n--){
	SETFLOAT(ap, f);
	ap+=x->row+1;
      }
    }
    matrix_bang(x);
    return;
  }

  if (argc<x->row){
    post("mtx_col : column length is too small for %dx%d-matrix", x->row, x->col);
    return;
  }
  if (x->current_col>x->col){
    post("mtx_col : too high a column is to be set");
    return;
  }
  if(x->current_col) {
    int r=x->row;
    t_atom *ap=x->atombuffer+1+x->current_col;
    while(r--)SETFLOAT(&ap[(x->row-r-1)*x->col], atom_getfloat(argv++));
  }  else {
    int r=x->row;
    t_atom *ap=x->atombuffer+2;
    while (r--) {
      t_float f=atom_getfloat(argv++);
      int c=x->col;
      while(c--){
	SETFLOAT(ap, f);
	ap++;
      }
    }
  }
  matrix_bang(x);
}
static void *mtx_col_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_col_class);
  int i, j, q;
  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  x->current_col=0;
  x->col=x->row=0;
  x->atombuffer=0;
  switch (argc) {
  case 0:break;
  case 1:
    i = atom_getfloat(argv);
    if (i<0)i=0;
    if(i)adjustsize(x, i, i);
    matrix_set(x, 0);
    break;
  case 2:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    break;
  default:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    q = atom_getfloat(argv++);if(q<0)q=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    x->current_col=q;
  }
  return (x);
}
static void mtx_col_setup(void)
{
  mtx_col_class = class_new(gensym("mtx_col"), (t_newmethod)mtx_col_new, 
			    (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_col_class, matrix_bang);
  class_addlist  (mtx_col_class, mtx_col_list);
  class_addmethod(mtx_col_class, (t_method)mtx_col_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_col_class, (t_method)mtx_col_float, gensym(""), A_FLOAT, 0);
  class_sethelpsymbol(mtx_col_class, gensym("zexy/mtx_element"));
}

/* mtx_element */
static t_class *mtx_element_class;

static void mtx_element_list2(t_matrix *x, t_floatarg f1, t_floatarg f2)
{
  int r = f1, c= f2;
  if(r<0)r=0;
  if(c<0)c=0;
  x->current_row = r;
  x->current_col = c;
}
static void mtx_element_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;
  if (argc<2){    post("matrix : corrupt matrix passed");    return;  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){    post("matrix : corrupt matrix passed");    return;  }
  if (row*col > argc-2){    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");    return;  }
  matrix_matrix2(x, s, argc, argv);
  matrix_bang(x);
}
static void mtx_element_float(t_matrix *x, t_floatarg f)
{
  if(x->current_col>x->col || x->current_row>x->row){
    error("mtx_element: element position exceeds matrix dimensions");
    return;
  }
  if(x->current_row == 0 && x->current_col == 0){
    matrix_set(x, f);
    matrix_bang(x);
    return;
  }
  if(x->current_row*x->current_col)SETFLOAT(x->atombuffer+1+(x->current_row-1)*x->col+x->current_col, f);
  else {
    t_atom *ap=x->atombuffer+2;
    int count;
    if (!x->current_col){
      ap+=x->col*(x->current_row-1);
      count=x->col;
      while(count--)SETFLOAT(&ap[count], f);
    } else { // x->current_row==0
      ap+=x->current_col-1;
      count=x->row;
      while(count--)SETFLOAT(&ap[count*x->col], f);
    }
  }
  matrix_bang(x);
}

static void *mtx_element_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_element_class);
  int i, j, q;
  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  x->current_row=x->current_col=0;
  x->col=x->row=0;
  x->atombuffer=0;
  switch (argc) {
  case 1:
    i = atom_getfloat(argv);
    if (i<0)i=0;
    if(i)adjustsize(x, i, i);
    matrix_set(x, 0);
    break;
  case 2:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    break;
  case 4:
    i = atom_getfloat(argv++);if(i<0)i=0;
    j = atom_getfloat(argv++);if(j<0)j=0;
    if(i*j)adjustsize(x, i, j);
    matrix_set(x, 0);
    q = atom_getfloat(argv++);if(q<0)q=0;
    x->current_row=q;
    q = atom_getfloat(argv++);if(q<0)q=0;
    x->current_col=q;
    break;
  default:;
  }
  return (x);
}
static void mtx_element_setup(void)
{
  mtx_element_class = class_new(gensym("mtx_element"), (t_newmethod)mtx_element_new, 
				(t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_element_class, matrix_bang);
  class_addfloat (mtx_element_class, mtx_element_float);
  class_addmethod(mtx_element_class, (t_method)mtx_element_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_element_class, (t_method)mtx_element_list2, gensym(""), A_FLOAT, A_FLOAT, 0);
  class_sethelpsymbol(mtx_element_class, gensym("zexy/mtx_element"));
}

/* mtx_eye */
static t_class *mtx_eye_class;
static void *mtx_eye_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_eye_class);
  int col=0, row=0;
  outlet_new(&x->x_obj, 0);
  x->row = x->col = 0;
  x->atombuffer   = 0;
  switch(argc) {
  case 0:
    break;
  case 1:
    col=row=atom_getfloat(argv);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
  }
  if(col<0)col=0;
  if(row<0)row=0;
  if (col*row){
    int n = (col<row)?col:row;
    x->atombuffer = (t_atom *)getbytes((col*row+2)*sizeof(t_atom));
    setdimen(x, row, col);
    matrix_set(x, 0);
    while(n--)SETFLOAT(x->atombuffer+2+n*(1+col), 1);
  }
  return (x);
}
static void mtx_eye_setup(void)
{
  mtx_eye_class = class_new(gensym("mtx_eye"), (t_newmethod)mtx_eye_new, 
			    (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addlist(mtx_eye_class, matrix_eye);
  class_addbang(mtx_eye_class, matrix_bang);
  class_addmethod(mtx_eye_class, (t_method)matrix_eye, gensym("matrix"), A_GIMME, 0);

  class_sethelpsymbol(mtx_eye_class, gensym("zexy/mtx_special"));
}
/* mtx_egg */
static t_class *mtx_egg_class;
static void *mtx_egg_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_egg_class);
  int col=0, row=0;
  outlet_new(&x->x_obj, 0);
  x->row = x->col = 0;
  x->atombuffer   = 0;
  switch(argc) {
  case 0:
    break;
  case 1:
    col=row=atom_getfloat(argv);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
  }
  if(col<0)col=0;
  if(row<0)row=0;
  if (col*row){
    int n = (col<row)?col:row;
    x->atombuffer = (t_atom *)getbytes((col*row+2)*sizeof(t_atom));
    setdimen(x, row, col);
    matrix_set(x, 0);
    while(n--)SETFLOAT(x->atombuffer+2+(n+1)*(col-1), 1);
  }
  return (x);
}
static void mtx_egg_setup(void)
{
  mtx_egg_class = class_new(gensym("mtx_egg"), (t_newmethod)mtx_egg_new, 
			    (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addlist(mtx_egg_class, matrix_egg);
  class_addbang(mtx_egg_class, matrix_bang);
  class_addmethod(mtx_egg_class, (t_method)matrix_egg, gensym("matrix"), A_GIMME, 0);

  class_sethelpsymbol(mtx_egg_class, gensym("zexy/mtx_special"));
}
/* mtx_ones */
static t_class *mtx_ones_class;
static void *mtx_ones_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_ones_class);
  int col=0, row=0;
  outlet_new(&x->x_obj, 0);
  x->row = x->col = 0;
  x->atombuffer   = 0;
  switch(argc) {
  case 0:
    break;
  case 1:
    col=row=atom_getfloat(argv);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
  }
  if(col<0)col=0;
  if(row<0)row=0;
  if (col*row){
    x->atombuffer = (t_atom *)getbytes((col*row+2)*sizeof(t_atom));
    setdimen(x, row, col);
    matrix_set(x, 1);
  }
  return (x);
}
static void mtx_ones_setup(void)
{
  mtx_ones_class = class_new(gensym("mtx_ones"), (t_newmethod)mtx_ones_new, 
			     (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addlist(mtx_ones_class, matrix_ones);
  class_addbang(mtx_ones_class, matrix_bang);
  class_addmethod(mtx_ones_class, (t_method)matrix_ones, gensym("matrix"), A_GIMME, 0);

  class_sethelpsymbol(mtx_ones_class, gensym("zexy/mtx_special"));
}

/* mtx_zeros */
static t_class *mtx_zeros_class;
static void *mtx_zeros_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_zeros_class);
  int col=0, row=0;
  outlet_new(&x->x_obj, 0);
  x->row = x->col = 0;
  x->atombuffer   = 0;
  switch(argc) {
  case 0:
    break;
  case 1:
    col=row=atom_getfloat(argv);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
  }
  if(col<0)col=0;
  if(row<0)row=0;
  if (col*row){
    x->atombuffer = (t_atom *)getbytes((col*row+2)*sizeof(t_atom));
    setdimen(x, row, col);
    matrix_set(x, 0);
  }
  return (x);
}
static void mtx_zeros_setup(void)
{
  mtx_zeros_class = class_new(gensym("mtx_zeros"), (t_newmethod)mtx_zeros_new, 
			      (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addlist(mtx_zeros_class, matrix_zeros);
  class_addbang(mtx_zeros_class, matrix_bang);
  class_addmethod(mtx_zeros_class, (t_method)matrix_zeros, gensym("matrix"), A_GIMME, 0);

  class_sethelpsymbol(mtx_zeros_class, gensym("zexy/mtx_special"));
}

/* mtx_diag */
static t_class *mtx_diag_class;
static void mtx_diag_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  int length=(col<row)?col:row, n=length;
  t_atom *ap = (t_atom *)getbytes(length * sizeof(t_atom)), *dummy=ap;
  if(row*col>argc-2)post("mtx_diag: sparse matrices not yet supported : use \"mtx_check\"");
  else {
    for(n=0;n<length;n++, dummy++)SETFLOAT(dummy, atom_getfloat(argv+n*(col+1)));
    outlet_list(x->x_obj.ob_outlet, gensym("diag"), length, ap);
  }
  freebytes(ap, (length * sizeof(t_atom)));
}

static void *mtx_diag_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_diag_class);
  outlet_new(&x->x_obj, 0);
  x->row = x->col = 0;
  x->atombuffer   = 0;

  if(!argc)return(x);
  x->atombuffer = (t_atom *)getbytes((argc*argc+2)*sizeof(t_atom));
  setdimen(x, argc, argc);
  matrix_set(x, 0);
  argv+=argc-1;
  while(argc--)SETFLOAT(x->atombuffer+2+argc*(1+x->col), atom_getfloat(argv--));

  return (x);
}
static void mtx_diag_setup(void)
{
  mtx_diag_class = class_new(gensym("mtx_diag"), (t_newmethod)mtx_diag_new, 
			     (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addlist  (mtx_diag_class, matrix_diag);
  class_addbang  (mtx_diag_class, matrix_bang);
  class_addmethod(mtx_diag_class, (t_method)mtx_diag_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_diag_class, gensym("zexy/mtx_trace"));
}

/* mtx_diegg */
static t_class *mtx_diegg_class;
static void mtx_diegg_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  int length=(col<row)?col:row, n=length;
  t_atom *ap = (t_atom *)getbytes(length * sizeof(t_atom)), *dummy=ap;
  if(row*col>argc-2)post("mtx_diegg: sparse matrices not yet supported : use \"mtx_check\"");
  else {
    for(n=0;n<length;n++, dummy++)SETFLOAT(dummy, atom_getfloat(argv+(n-1)*(col-1)));
    outlet_list(x->x_obj.ob_outlet, gensym("diegg"), length, ap);
  }
  freebytes(ap, (length * sizeof(t_atom)));
}

static void *mtx_diegg_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_diegg_class);
  outlet_new(&x->x_obj, 0);
  x->row = x->col = 0;
  x->atombuffer   = 0;

  if(!argc)return(x);
  x->atombuffer = (t_atom *)getbytes((argc*argc+2)*sizeof(t_atom));
  setdimen(x, argc, argc);
  matrix_set(x, 0);
  argv+=argc-1;
  while(argc--)SETFLOAT(x->atombuffer+2+argc*(1+x->col), atom_getfloat(argv--));

  return (x);
}
static void mtx_diegg_setup(void)
{
  mtx_diegg_class = class_new(gensym("mtx_diegg"), (t_newmethod)mtx_diegg_new, 
			     (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addlist  (mtx_diegg_class, matrix_diegg);
  class_addbang  (mtx_diegg_class, matrix_bang);
  class_addmethod(mtx_diegg_class, (t_method)mtx_diegg_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_diegg_class, gensym("zexy/mtx_special"));
}
/* mtx_trace */
static t_class *mtx_trace_class;
typedef struct _mtx_trace
{
  t_object x_obj;
  t_float trace;
} t_mtx_trace;
static void mtx_trace_bang(t_mtx_trace *x)
{
  outlet_float(x->x_obj.ob_outlet, x->trace);
}
static void mtx_trace_matrix(t_mtx_trace *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  int length=(col<row)?col:row;
  t_float trace = 0;
  if(row*col>argc-2)post("mtx_trace: sparse matrices not yet supported : use \"mtx_check\"");
  else while(length--)trace+=atom_getfloat(argv+length*(col+1));
  x->trace=trace;
  mtx_trace_bang(x);
}
static void *mtx_trace_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mtx_trace *x = (t_mtx_trace *)pd_new(mtx_trace_class);
  outlet_new(&x->x_obj, 0);
  x->trace=0;
  return (x);
}
static void mtx_trace_setup(void)
{
  mtx_trace_class = class_new(gensym("mtx_trace"), (t_newmethod)mtx_trace_new, 
			      0, sizeof(t_mtx_trace), 0, A_GIMME, 0);
  class_addbang  (mtx_trace_class, mtx_trace_bang);
  class_addmethod(mtx_trace_class, (t_method)mtx_trace_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_trace_class, gensym("zexy/mtx_trace"));
}

/* mtx_mean */
static t_class *mtx_mean_class;

static void mtx_mean_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *ip, *op;
  int c=col, r;
  t_float sum;
  t_float factor=1./row;
  adjustsize(x, 1, col);
  op=x->atombuffer;

  while(c--){
    sum=0;
    ip=argv+col-c-1;
    r=row;
    while(r--)sum+=atom_getfloat(ip+col*r);
    SETFLOAT(op, sum*factor);
    op++;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("row"), col, x->atombuffer);
}

static void *mtx_mean_new(void)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_mean_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;
  return (x);
}
static void mtx_mean_setup(void)
{
  mtx_mean_class = class_new(gensym("mtx_mean"), (t_newmethod)mtx_mean_new, 
			     (t_method)matrix_free, sizeof(t_matrix), 0, 0, 0);
  class_addmethod(mtx_mean_class, (t_method)mtx_mean_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_mean_class, gensym("zexy/mtx_mean"));
}

/* mtx_rand */
static t_class *mtx_rand_class;

static void mtx_rand_seed(t_matrix *x, t_float f)
{
  x->current_row=f;
}
static int makeseed(void)
{
  static unsigned int random_nextseed = 1489853723;
  random_nextseed = random_nextseed * 435898247 + 938284287;
  return (random_nextseed & 0x7fffffff);
}
static void mtx_rand_random(t_matrix *x)
{
  long size = x->row * x->col;
  t_atom *ap=x->atombuffer+2;
  int val = x->current_row;
  while(size--)SETFLOAT(ap+size, ((float)(((val=val*435898247+382842987)&0x7fffffff)-0x40000000))*(float)(0.5/0x40000000)+0.5);
  x->current_row=val;
}

static void mtx_rand_list(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row = atom_getfloat(argv++);
  int col = atom_getfloat(argv++);

  if(!argv)return;
  if(argc==1)col=row;

  adjustsize(x, row, col);
  mtx_rand_random(x);
  matrix_bang(x);
}
static void mtx_rand_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  matrix_matrix2(x, s, argc, argv);
  mtx_rand_random(x);
  matrix_bang(x);
}
static void mtx_rand_bang(t_matrix *x)
{
  mtx_rand_random(x);
  matrix_bang(x);
}
static void *mtx_rand_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_rand_class);
  int row, col;
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;
  x->current_row=makeseed();

  if (argc) {
	row=atom_getfloat(argv);
	col=(argc>1)?atom_getfloat(argv+1):row;
	adjustsize(x, row, col);
	mtx_rand_random(x);
  }
  return (x);
}
static void mtx_rand_setup(void)
{
  mtx_rand_class = class_new(gensym("mtx_rand"), (t_newmethod)mtx_rand_new, 
			     (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addmethod(mtx_rand_class, (t_method)mtx_rand_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_rand_class, mtx_rand_list);
  class_addbang  (mtx_rand_class, mtx_rand_bang);

  class_addmethod(mtx_rand_class, (t_method)mtx_rand_seed, gensym("seed"), A_FLOAT, 0);
  class_sethelpsymbol(mtx_rand_class, gensym("zexy/mtx_rand"));
}


/* mtx_scroll */
/* scroll the rows */
static t_class *mtx_scroll_class;

static void mtx_scroll_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  int rowscroll = ((int)x->f%row+row)%row;

  if(row*col>argc-2) {
    post("mtx_scroll: sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  adjustsize(x, row, col);

  memcpy(x->atombuffer+2, argv+(row-rowscroll)*col, rowscroll*col*sizeof(t_atom));
  memcpy(x->atombuffer+2+rowscroll*col, argv, (row-rowscroll)*col*sizeof(t_atom));

  matrix_bang(x);
}

static void *mtx_scroll_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_scroll_class);
  floatinlet_new(&x->x_obj, &(x->f));
  outlet_new(&x->x_obj, 0);

  x->f=argc?atom_getfloat(argv):0;
  x->col=x->row=0;
  x->atombuffer=0;
  return (x);
}
static void mtx_scroll_setup(void)
{
  mtx_scroll_class = class_new(gensym("mtx_scroll"), (t_newmethod)mtx_scroll_new, 
				  (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_scroll_class, matrix_bang);
  class_addmethod(mtx_scroll_class, (t_method)mtx_scroll_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_scroll_class, gensym("zexy/mtx_transpose"));
}

/* mtx_roll */
/* roll the rows */
static t_class *mtx_roll_class;

static void mtx_roll_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *ap;
  int colroll = ((int)x->f%col+col)%col;
  int c;

  if(row*col>argc-2) {
    post("mtx_roll: sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }

  adjustsize(x, row, col);
  ap = x->atombuffer+2;

  c=col;
  while(c--){
    t_atom *in  = argv+col-c-1;
    t_atom *out = ap  +(col-c-1+colroll)%col;
    int r = row;
    while (r--){
      SETFLOAT(out, atom_getfloat(in));
      out+=col;
      in+=col;
    }

  }

  matrix_bang(x);
}

static void *mtx_roll_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_roll_class);
  floatinlet_new(&x->x_obj, &(x->f));
  outlet_new(&x->x_obj, 0);

  x->f=argc?atom_getfloat(argv):0;
  x->col=x->row=0;
  x->atombuffer=0;
  return (x);
}
static void mtx_roll_setup(void)
{
  mtx_roll_class = class_new(gensym("mtx_roll"), (t_newmethod)mtx_roll_new, 
				  (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_roll_class, matrix_bang);
  class_addmethod(mtx_roll_class, (t_method)mtx_roll_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_roll_class, gensym("zexy/mtx_transpose"));
}

/* mtx_transpose */
static t_class *mtx_transpose_class;

static void mtx_transpose_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *ap;
  int r, c;

  if(row*col>argc-2) {
    post("mtx_transpose: sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  if (col*row!=x->col*x->row) {
    freebytes(x->atombuffer, (x->col*x->row+2)*sizeof(t_atom));
    x->atombuffer = (t_atom *)getbytes((row*col+2)*sizeof(t_atom));
  }
  ap = x->atombuffer+2;
  setdimen(x, col, row);
  r = row;
  while(r--){
    c=col;
    while(c--) {
      t_float f = atom_getfloat(argv+r*col+c);
      SETFLOAT(ap+c*row+r, f);
    }
  }
    
  matrix_bang(x);
}

static void *mtx_transpose_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_transpose_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;
  return (x);
}
static void mtx_transpose_setup(void)
{
  mtx_transpose_class = class_new(gensym("mtx_transpose"), (t_newmethod)mtx_transpose_new, 
				  (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_transpose_class, matrix_bang);
  class_addmethod(mtx_transpose_class, (t_method)mtx_transpose_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_transpose_class, gensym("zexy/mtx_transpose"));
}

/* -­------------------------------------------------------------- */
/* matrix math */

typedef struct _mtx_binscalar
{
  t_object x_obj;

  t_matrix m; // the output matrix
  t_float f;  // the second input
} t_mtx_binscalar;

typedef struct _mtx_binmtx
{
  t_object x_obj;

  t_matrix m;  // the output matrix
  t_matrix m2; // the second input
} t_mtx_binmtx;

static void mtx_bin_matrix2(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row = atom_getfloat(argv);
  int col = atom_getfloat(argv+1);
  if (argc<2){post("mtx_bin2: crippled matrix"); return;}
  if ((col<1)||(row<1)) {post("mtx_bin2: invalid dimensions %dx%d", row,col); return;}
  if (col*row+2>argc){ post("mtx_bin2: sparse matrix not yet supported : use \"mtx_check\""); return;}

  if (row*col!=x->m2.row*x->m2.col) {
    freebytes(x->m2.atombuffer, (x->m2.row*x->m2.col+2)*sizeof(t_atom));
    x->m2.atombuffer=copybytes(argv,(row*col+2)*sizeof(t_atom));
  }else memcpy(x->m2.atombuffer, argv, (row*col+2)*sizeof(t_atom));
  setdimen(&x->m2, row, col);
}

static void mtx_binmtx_bang(t_mtx_binmtx *x)
{
  if((&x->m)&&(x->m.atombuffer))
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->m.col*x->m.row+2, x->m.atombuffer);
}


static void mtx_binmtx_free(t_mtx_binmtx *x)
{
  matrix_free(&x->m);
  matrix_free(&x->m2);
}
static void mtx_binscalar_bang(t_mtx_binscalar *x)
{
  if((&x->m)&&(x->m.atombuffer))
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->m.col*x->m.row+2, x->m.atombuffer);
}
static void mtx_binscalar_free(t_mtx_binscalar *x)
{
  matrix_free(&x->m);
}



/* mtx_add */
static t_class *mtx_add_class, *mtx_addscalar_class;

static void mtx_addscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  
  t_float offset=x->f;
  t_atom *buf;
  t_atom *ap=argv+2;

  if(argc<2){post("mtx_add: crippled matrix");return; }
  adjustsize(&x->m, row, col);

  buf=x->m.atombuffer+2;

  while(n--){
    buf->a_type = A_FLOAT;
    buf++->a_w.w_float = atom_getfloat(ap++) + offset;
  }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_addscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float offset = x->f;
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++) + offset;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void mtx_add_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *m;
  t_atom *m1 = argv+2;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_add: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_add: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  if (!(x->m2.col*x->m2.row)) {
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, argv);
    return;
  }

  if ((col!=x->m2.col)||(row!=x->m2.row)){ 
    post("mtx_add: matrix dimensions do not match");
    /* LATER SOLVE THIS */    
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(m1++)+atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_add_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ post("mulitply with what ?");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f+atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void *mtx_add_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_add : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_addscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_add_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->m.col = x->m.row =  x->m2.col = x->m2.row = 0;
    x->m.atombuffer = x->m2.atombuffer = 0;
    return(x);
  }
}

static void mtx_add_setup(void)
{
  mtx_add_class = class_new(gensym("mtx_add"), (t_newmethod)mtx_add_new, (t_method)mtx_binmtx_free,
			    sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)mtx_add_new, gensym("mtx_+"), A_GIMME,0);
  class_addmethod(mtx_add_class, (t_method)mtx_add_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_add_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_add_class, mtx_add_float);
  class_addbang  (mtx_add_class, mtx_binmtx_bang);

  mtx_addscalar_class = class_new(gensym("mtx_add"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addcreator(0, gensym("mtx_+"), 0, 0);
  class_addmethod(mtx_addscalar_class, (t_method)mtx_addscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_addscalar_class, mtx_addscalar_list);
  class_addbang  (mtx_addscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_add_class, gensym("zexy/mtx_binops"));
  class_sethelpsymbol(mtx_addscalar_class, gensym("zexy/mtx_binops"));
}

/* mtx_sub */
static t_class *mtx_sub_class, *mtx_subscalar_class;

static void mtx_subscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  
  t_float offset=x->f;
  t_atom *buf;
  t_atom *ap=argv+2;

  if(argc<2){post("mtx_sub: crippled matrix");return; }
  adjustsize(&x->m, row, col);

  buf=x->m.atombuffer+2;

  while(n--){
    buf->a_type = A_FLOAT;
    buf++->a_w.w_float = atom_getfloat(ap++) - offset;
  }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_subscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float offset = x->f;
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++) - offset;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void mtx_sub_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *m;
  t_atom *m1 = argv+2;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_sub: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_sub: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  if (!(x->m2.col*x->m2.row)) {
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, argv);
    return;
  }

  if ((col!=x->m2.col)||(row!=x->m2.row)){ 
    post("mtx_sub: matrix dimensions do not match");
    /* LATER SOLVE THIS */    
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(m1++)-atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_sub_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ post("mulitply with what ?");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f-atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void *mtx_sub_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_sub : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_subscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_sub_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->m.col = x->m.row =  x->m2.col = x->m2.row = 0;
    x->m.atombuffer = x->m2.atombuffer = 0;
    return(x);
  }
}

static void mtx_sub_setup(void)
{
  mtx_sub_class = class_new(gensym("mtx_sub"), (t_newmethod)mtx_sub_new, (t_method)mtx_binmtx_free,
			    sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)mtx_sub_new, gensym("mtx_-"), A_GIMME,0);
  class_addmethod(mtx_sub_class, (t_method)mtx_sub_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_sub_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_sub_class, mtx_sub_float);
  class_addbang  (mtx_sub_class, mtx_binmtx_bang);

  mtx_subscalar_class = class_new(gensym("mtx_sub"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addcreator(0, gensym("mtx_-"), 0, 0);
  class_addmethod(mtx_subscalar_class, (t_method)mtx_subscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_subscalar_class, mtx_subscalar_list);
  class_addbang  (mtx_subscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_sub_class, gensym("zexy/mtx_binops"));
  class_sethelpsymbol(mtx_subscalar_class, gensym("zexy/mtx_binops"));
}


/* mtx_mul */
static t_class *mtx_mul_class, *mtx_mulelement_class, *mtx_mulscalar_class;

static void mtx_mul_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap1=argv+2, *ap2=m2->atombuffer+2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  int row2, col2, n, r, c;

  if (!m2->atombuffer){ post("mulitply with what ?");            return; }
  if (argc<2){          post("mtx_mul: crippled matrix");        return; }
  if ((col<1)||(row<1)){post("mtx_mul: invalid dimensions");     return; }
  if (col*row>argc-2){  post("sparse matrix not yet supported : use \"mtx_check\""); return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
 
  if (col!=row2) {      post("mtx_mul: matrix dimensions do not match !"); return;  }

  adjustsize(m, row, col2); 
  ap=m->atombuffer+2;

  for(r=0;r<row;r++)
    for(c=0;c<col2;c++) {
      T_FLOAT sum = 0.f;
      for(n=0;n<col;n++)sum+=(T_FLOAT)atom_getfloat(ap1+col*r+n)*atom_getfloat(ap2+col2*n+c);
      SETFLOAT(ap+col2*r+c,sum);
    }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}

static void mtx_mul_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ post("mulitply with what ?");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f*atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}

static void mtx_mulelement_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_mul: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_mul: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }
  if (!(x->m2.col*x->m2.row)) {
    adjustsize(&x->m, row, col);
    matrix_set(&x->m, 0);
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
    return;
  }
  if ((col!=x->m2.col)||(row!=x->m2.row)){    post("matrix dimension do not match");    /* LATER SOLVE THIS */    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(argv++)*atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}

static void mtx_mulscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  t_atom *m;
  t_float factor = x->f;
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);

  if (argc<2){
    post("mtx_mul: crippled matrix");
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_mulscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float factor = x->f;
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_mul_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_mul : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_mulscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    if (s->s_name[4]=='.') {
      /* element mul */

      t_matrix *x = (t_matrix *)pd_new(mtx_mulelement_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
      outlet_new(&x->x_obj, 0);
      x->col = x->row = 0;
      x->atombuffer = 0;
      return(x);
    } else {
      t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_mul_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
      outlet_new(&x->x_obj, 0);
      x->m.col = x->m.row = x->m2.col = x->m2.row = 0;
      x->m.atombuffer = x->m2.atombuffer = 0;
      return (x);
    }
  }
}

static void mtx_mul_setup(void)
{
  mtx_mul_class = class_new(gensym("mtx_mul"), (t_newmethod)mtx_mul_new, (t_method)mtx_binmtx_free,
			    sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)mtx_mul_new, gensym("mtx_*"), A_GIMME,0);
  class_addmethod(mtx_mul_class, (t_method)mtx_mul_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_mul_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_mul_class, mtx_mul_float);
  class_addbang  (mtx_mul_class, mtx_binmtx_bang);

  mtx_mulelement_class = class_new(gensym("mtx_.*"), (t_newmethod)mtx_mul_new, (t_method)mtx_binmtx_free,
				   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_mulelement_class, (t_method)mtx_mulelement_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_mulelement_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_mulelement_class, mtx_mul_float);
  class_addbang  (mtx_mulelement_class, mtx_binmtx_bang);

  mtx_mulscalar_class = class_new(gensym("mtx_mul"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addcreator(0, gensym("mtx_*"), 0, 0);
  class_addcreator(0, gensym("mtx_.*"), 0, 0);
  class_addmethod(mtx_mulscalar_class, (t_method)mtx_mulscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_mulscalar_class, mtx_mulscalar_list);
  class_addbang  (mtx_mulscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_mul_class, gensym("zexy/mtx_binops"));
  class_sethelpsymbol(mtx_mulelement_class, gensym("zexy/mtx_binops"));
  class_sethelpsymbol(mtx_mulscalar_class, gensym("zexy/mtx_binops"));
}


/* mtx_div */
static t_class *mtx_divelement_class, *mtx_divscalar_class;

static void mtx_divelement_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_div: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_div: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }
  if (!(x->m2.col*x->m2.row)) {
    adjustsize(&x->m, row, col);
    matrix_set(&x->m, 0);
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
    return;
  }
  if ((col!=x->m2.col)||(row!=x->m2.row)){    post("matrix dimension do not match");    /* LATER SOLVE THIS */    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(argv++)/atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_divelement_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ post("divide by what ?");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f/atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void mtx_divscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  t_atom *m;
  t_float factor = 1.0/x->f;
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);

  if (argc<2){
    post("mtx_div: crippled matrix");
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_divscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float factor = 1.0/x->f;

  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_div_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_div : extra arguments ignored");
  if (argc) {
    /* scalar division */
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_divscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    /* element division */
    t_matrix *x = (t_matrix *)pd_new(mtx_divelement_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->col = x->row = 0;
    x->atombuffer = 0;
    return(x);
  }
}

static void mtx_div_setup(void)
{
  mtx_divelement_class = class_new(gensym("mtx_./"), (t_newmethod)mtx_div_new, (t_method)mtx_binmtx_free,
				   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_divelement_class, (t_method)mtx_divelement_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_divelement_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_divelement_class, mtx_divelement_float);
  class_addbang  (mtx_divelement_class, mtx_binmtx_bang);

  mtx_divscalar_class = class_new(gensym("mtx_./"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addmethod(mtx_divscalar_class, (t_method)mtx_divscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_divscalar_class, mtx_divscalar_list);
  class_addbang  (mtx_divscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_divelement_class, gensym("zexy/mtx_binops"));
  class_sethelpsymbol(mtx_divscalar_class, gensym("zexy/mtx_binops"));
}

/* mtx_inverse */
static t_class *mtx_inverse_class;

static void mtx_inverse_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  /* maybe we should do this in double or long double ? */
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  int i, k, row2=row*row;

  T_FLOAT *original, *inverted;
  T_FLOAT *a1, *a2, *b1, *b2;  // dummy pointers

  int ok = 0;

  if(row*col+2>argc){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  if (row!=col){
    post("mtx_inverse: only square matrices can be inverted");
    return;
  }

  // reserve memory for outputting afterwards
  adjustsize(x, row, row);
  // 1. get the 2 matrices : orig; invert (create as eye, but will be orig^(-1))
  inverted = (T_FLOAT *)getbytes(sizeof(T_FLOAT)*row2);
  // 1a extract values of A to float-buf
  original=matrix2float(argv);
  // 1b make an eye-shaped float-buf for B
  i=row2;
  b1=inverted;
  while(i--)*b1++=0;
  i=row;
  b1=inverted;
  while(i--)b1[i*(row+1)]=1;

  // 2. do the Gauss-Jordan
  for (k=0;k<row;k++) {
    // 2. adjust current row
    T_FLOAT diagel = original[k*(col+1)];
#if 1
    T_FLOAT i_diagel = diagel?1./diagel:0;
    if (!diagel)ok++;
#else
    T_FLOAT i_diagel = 1./diagel;
#endif

    /* normalize current row (set the diagonal-element to 1 */
    a2=original+k*col;
    b2=inverted+k*col;
    i=row;
    while(i--){
      *a2++*=i_diagel;
      *b2++*=i_diagel;
    }
    /* eliminate the k-th element in each row by adding the weighted normalized row */

    a2=original+k*row;
    b2=inverted+k*row;
    for(i=0;i<row;i++)
      if (i-k) {
	T_FLOAT f=-*(original+i*row+k);
	int j = row;
	a1=original+i*row;
	b1=inverted+i*row;
	while (j--) {
	  *(a1+j)+=f**(a2+j);
	  *(b1+j)+=f**(b2+j);
	}
      }
  }
  // 3. output the matrix
  // 3a convert the floatbuf to an atombuf;
  float2matrix(x->atombuffer, inverted);
  // 3b destroy the buffers
  freebytes(original, sizeof(T_FLOAT)*row2);

  if (ok)post("mtx_inverse: couldn't really invert the matrix !!! %d error%c", ok, (ok-1)?'s':0);

  // 3c output the atombuf;
  matrix_bang(x);
}

static void *mtx_inverse_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_inverse_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;

  return (x);
}
static void mtx_inverse_setup(void)
{
  mtx_inverse_class = class_new(gensym("mtx_inverse"), (t_newmethod)mtx_inverse_new, 
				(t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_inverse_class, matrix_bang);
  class_addmethod(mtx_inverse_class, (t_method)mtx_inverse_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_inverse_class, gensym("zexy/mtx_inverse"));
}


/* mtx_pivot */
static t_class *mtx_pivot_class;

typedef struct _mtx_pivot
{
  t_object x_obj;

  t_matrix m;  // the output matrix
  t_matrix m_pre;  // the pre -multiply matrix
  t_matrix m_post; // the post-multiply matrix

  t_outlet *pivo, *pre, *post;
  
} t_mtx_pivot;

static void mtx_pivot_matrix(t_mtx_pivot *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *m_pre, *m_post;
  int i, j, k;
  int min_rowcol = (row<col)?row:col;
  T_FLOAT *buffer, *buf;
  int *i_pre, *i_post, *i_buf;

  int pivot_row, pivot_col;

  if (argc<2){    post("mtx_pivot: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_pivot: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  adjustsize(&x->m, row, col);
  adjustsize(&x->m_pre, row, row);
  adjustsize(&x->m_post,col, col);
  matrix_set(&x->m_pre, 0);
  matrix_set(&x->m_post, 0);

  buffer = matrix2float(argv);
  i_pre    = (int *)getbytes(sizeof(int)*row);
  i_post   = (int *)getbytes(sizeof(int)*col);

  /* clear pre&post matrices */
  i=row;
  i_buf=i_pre;
  while(i--)*i_buf++=row-i-1;
  i=col;
  i_buf=i_post;
  while(i--)*i_buf++=col-i-1;

  /* do the pivot thing */

  for (k=0; k<min_rowcol-1; k++){
    // 1. find max_element
    T_FLOAT max = 0;
    pivot_row = pivot_col = k;

    for(i=k; i<row; i++){
      buf=buffer+col*i+k;

      j=col-k;
      while(j--){
	T_FLOAT f = fabsf(*buf++);
	if (f>max) {
	  max=f;
	  pivot_row = i;
	  pivot_col = col-j-1;
	}
      }
    }
    // 2. move max el to [k,k]
    // 2a swap rows
    if (k-pivot_row) {
      T_FLOAT *oldrow=buffer+col*k;
      T_FLOAT *newrow=buffer+col*pivot_row;

      i=col;
      while(i--){
	T_FLOAT f=*oldrow;
	*oldrow++=*newrow;
	*newrow++=f;
      }
      i=i_pre[k];
      i_pre[k]=i_pre[pivot_row];
      i_pre[pivot_row]=i;
    }
    // 2b swap columns
    if (k-pivot_col) {
      T_FLOAT *oldcol=buffer+k;
      T_FLOAT *newcol=buffer+pivot_col;

      i=row;
      while(i--){
	T_FLOAT f=*oldcol;
	*oldcol=*newcol;
	*newcol=f;
	oldcol+=col;
	newcol+=col;
      }
      i=i_post[k];
      i_post[k]=i_post[pivot_col];
      i_post[pivot_col]=i;
    }
  }

  float2matrix(x->m.atombuffer, buffer);

  i=col;
  m_post = x->m_post.atombuffer+2;
  while(i--){
    SETFLOAT(m_post+i_post[i]*col+i, 1);
  }
  i=row;
  m_pre = x->m_pre.atombuffer+2;
  while(i--)SETFLOAT(m_pre+i_pre[i]+i*col, 1);

  
  outlet_anything(x->post, gensym("matrix"), 2+col*col, x->m_post.atombuffer);
  outlet_anything(x->pre, gensym("matrix"), 2+row*row, x->m_pre.atombuffer);
  outlet_anything(x->pivo , gensym("matrix"), 2+row*col, x->m.atombuffer );
}

static void mtx_pivot_free(t_mtx_pivot *x)
{
  matrix_free(&x->m);
  matrix_free(&x->m_pre);
  matrix_free(&x->m_post);
}

static void *mtx_pivot_new(void)
{
  t_mtx_pivot *x = (t_mtx_pivot *)pd_new(mtx_pivot_class);

  x->pivo = outlet_new(&x->x_obj, 0);
  x->pre  = outlet_new(&x->x_obj, 0);
  x->post = outlet_new(&x->x_obj, 0);

  x->m.atombuffer = x->m_pre.atombuffer = x->m_post.atombuffer = 0;
  x->m.row = x->m.col = x->m_pre.row = x->m_pre.col = x->m_post.row = x->m_post.col = 0;

  return(x);
}

static void mtx_pivot_setup(void)
{
  mtx_pivot_class = class_new(gensym("mtx_pivot"), (t_newmethod)mtx_pivot_new, (t_method)mtx_pivot_free,
			      sizeof(t_mtx_pivot), 0, 0, 0);
  class_addmethod(mtx_pivot_class, (t_method)mtx_pivot_matrix, gensym("matrix"), A_GIMME, 0);

  class_sethelpsymbol(mtx_pivot_class, gensym("zexy/mtx_transpose"));
}


/* -­------------------------------------------------------------- */
/* utilities */
/* mtx_check */
static t_class *mtx_check_class;

static void mtx_check_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *ap;
  int length=row*col, n;
  argc-=2;

  if(length>argc) {
    /* sparse matrix */
    adjustsize(x, row, col);
    matrix_set(x, 0);
    argv+=2;
    ap=x->atombuffer+2;
    n=argc;
    while(n--){
      t_float f = atom_getfloat(argv++);
      SETFLOAT(ap, f);
      ap++;
    }    
    matrix_bang(x);
  } else {
    SETFLOAT(argv, row);
    SETFLOAT(argv+1, col);
    ap=argv+2;
    n=length;
    while(n--){
      t_float f = atom_getfloat(ap);
      SETFLOAT(ap, f);
      ap++;
    }
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), length+2, argv);
  }
}

static void *mtx_check_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_check_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;
  return (x);
}
static void mtx_check_setup(void)
{
  mtx_check_class = class_new(gensym("mtx_check"), (t_newmethod)mtx_check_new, 
			      (t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_check_class, matrix_bang);
  class_addmethod(mtx_check_class, (t_method)mtx_check_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_check_class, gensym("zexy/matrix"));
}

/* mtx_size */
static t_class *mtx_size_class;
typedef struct _mtx_size
{
  t_object x_obj;

  int      row;
  int      col;

  t_outlet *left, *right;
} t_mtx_size;

static void mtx_size_matrix(t_mtx_size *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc<2)return;
  outlet_float(x->right, atom_getfloat(argv+1));
  outlet_float(x->left,  atom_getfloat(argv));

}

static void *mtx_size_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mtx_size *x = (t_mtx_size *)pd_new(mtx_size_class);
  x->left  = outlet_new(&x->x_obj, 0);
  x->right = outlet_new(&x->x_obj, 0);

  return (x);
}
static void mtx_size_setup(void)
{
  mtx_size_class = class_new(gensym("mtx_size"), (t_newmethod)mtx_size_new, 
			     0, sizeof(t_mtx_size), 0, A_GIMME, 0);
  class_addmethod(mtx_size_class, (t_method)mtx_size_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_size_class, gensym("zexy/mtx_size"));
}

/* mtx_print */
static t_class *mtx_print_class;
static void mtx_print(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  if (argc<2){
    post("mtx_print : crippled matrix");
    return;
  }
  row = atom_getfloat(argv++);
  col = atom_getfloat(argv++);
  if(row*col>argc-2){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  post("matrix:");
  while(row--){
    postatom(col, argv);
    argv+=col;
    endpost();
  }
  endpost();
}
static void *mtx_print_new(void)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_print_class);
  x->row = x->col = 0;
  x->atombuffer   = 0;
  return (x);
}
static void mtx_print_setup(void)
{
  mtx_print_class = class_new(gensym("mtx_print"), (t_newmethod)mtx_print_new, 
			      0, sizeof(t_matrix), 0, 0, 0);
  class_addmethod  (mtx_print_class, (t_method)mtx_print, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_print_class, gensym("zexy/matrix"));
}


/* -------------- overall setup routine for this file ----------------- */

void z_matrix_setup(void)
{
  matrix_setup();

  mtx_resize_setup();
  mtx_row_setup();
  mtx_col_setup();
  mtx_element_setup();

  mtx_eye_setup();
  mtx_egg_setup();
  mtx_zeros_setup();
  mtx_ones_setup();
  mtx_diag_setup();
  mtx_diegg_setup();
  mtx_trace_setup();

  mtx_transpose_setup();
  mtx_scroll_setup();
  mtx_roll_setup();

  mtx_mean_setup();
  mtx_rand_setup();

  mtx_add_setup();
  mtx_sub_setup();
  mtx_mul_setup();
  mtx_div_setup();
  mtx_inverse_setup();
  mtx_pivot_setup();

  mtx_size_setup();

  mtx_check_setup();
  mtx_print_setup();

  if (0) debugmtx(0,0,0); /* this is to avoid this compiler warning... */
}
