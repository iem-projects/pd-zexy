#include "zexy.h"
#include <stdlib.h>
#include <string.h>

#ifdef NT
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
# define sqrtf sqrt
# define STATIC_INLINE
#else
# define STATIC_INLINE static
#endif

/*
 * atoi : ascii to integer
 * strcmp    : compare 2 lists as if they were strings
 * list2symbol: convert a list into a single symbol
 * symbol2list: vice versa
*/

/* atoi ::  ascii to integer */

static t_class *atoi_class;

typedef struct _atoi
{
  t_object x_obj;
  int i;
} t_atoi;
static void atoi_bang(t_atoi *x)
{
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_float(t_atoi *x, t_floatarg f)
{
  x->i = f;
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_symbol(t_atoi *x, t_symbol *s)
{
  int base=10;
  const char* c = s->s_name;
  if(c[0]=='0'){
    base=8;
    if (c[1]=='x')base=16;
  }
  x->i=strtol(c, 0, base);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_list(t_atoi *x, t_symbol *s, int argc, t_atom *argv)
{
  int base=10;
  const char* c;

  if (argv->a_type==A_FLOAT){
    x->i=atom_getfloat(argv);
    outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
    return;
  }

  if (argc>1){
    base=atom_getfloat(argv+1);
    if (base<2) {
      error("atoi: setting base to 10");
      base=10;
    }
  }
  c=atom_getsymbol(argv)->s_name;
  x->i=strtol(c, 0, base);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}

static void *atoi_new(void)
{
  t_atoi *x = (t_atoi *)pd_new(atoi_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

static void atoi_setup(void)
{
  atoi_class = class_new(gensym("atoi"), (t_newmethod)atoi_new, 0,
			 sizeof(t_atoi), 0, A_DEFFLOAT, 0);

  class_addbang(atoi_class, (t_method)atoi_bang);
  class_addfloat(atoi_class, (t_method)atoi_float);
  class_addlist(atoi_class, (t_method)atoi_list);
  class_addsymbol(atoi_class, (t_method)atoi_symbol);
  class_addanything(atoi_class, (t_method)atoi_symbol);

  class_sethelpsymbol(atoi_class, gensym("zexy/atoi"));
}

/* ------------------------- strcmp ------------------------------- */

/* compare 2 lists ( == for lists) */

static t_class *strcmp_class;

typedef struct _strcmp
{
  t_object x_obj;

  t_binbuf *bbuf1, *bbuf2;
} t_strcmp;

static void strcmp_bang(t_strcmp *x)
{
  char *str1=0, *str2=0;
  int n1=0, n2=0;
  int result = 0;

  binbuf_gettext(x->bbuf1, &str1, &n1);
  binbuf_gettext(x->bbuf2, &str2, &n2);

  result = strcmp(str1, str2);

  freebytes(str1, n1);
  freebytes(str2, n2);

  outlet_float(x->x_obj.ob_outlet, result);
}

static void strcmp_secondlist(t_strcmp *x, t_symbol *s, int argc, t_atom *argv)
{
  binbuf_clear(x->bbuf2);
  binbuf_add(x->bbuf2, argc, argv);
}

static void strcmp_list(t_strcmp *x, t_symbol *s, int argc, t_atom *argv)
{
  binbuf_clear(x->bbuf1);
  binbuf_add(x->bbuf1, argc, argv);
  
  strcmp_bang(x);
}

static void *strcmp_new(t_symbol *s, int argc, t_atom *argv)
{
  t_strcmp *x = (t_strcmp *)pd_new(strcmp_class);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("lst2"));

  x->bbuf1 = binbuf_new();
  x->bbuf2 = binbuf_new();


  strcmp_secondlist(x, gensym("list"), argc, argv);

  return (x);
}

static void strcmp_free(t_strcmp *x)
{
  binbuf_free(x->bbuf1);
  binbuf_free(x->bbuf2);
}


static void strcmp_setup(void)
{
  strcmp_class = class_new(gensym("strcmp"), (t_newmethod)strcmp_new, 
			 (t_method)strcmp_free, sizeof(t_strcmp), 0, A_GIMME, 0);

  class_addbang    (strcmp_class, strcmp_bang);
  class_addlist    (strcmp_class, strcmp_list);
  class_addmethod  (strcmp_class, (t_method)strcmp_secondlist, gensym("lst2"), A_GIMME, 0);

  class_sethelpsymbol(strcmp_class, gensym("zexy/strcmp"));
}

/* ------------------------- list2symbol ------------------------------- */

static t_class *list2symbol_class;

typedef struct _list2symbol
{
  t_object x_obj;
  int       ac;
  t_atom   *ap;
  t_symbol *s,*connector;
} t_list2symbol;

static void list2symbol_connector(t_list2symbol *x, t_symbol *s){
  x->connector = s;
}

static void list2symbol_bang(t_list2symbol *x)
{
  t_atom *argv=x->ap;
  int     argc=x->ac;
  char *result = 0;
  int length = 0, len=0;
  int i= argc;
  char *connector=0;
  char connlen=0;
  if(x->connector)connector=x->connector->s_name;
  if(connector)connlen=strlen(connector);

  /* 1st get the length of the symbol */
  if(x->s)length+=strlen(x->s->s_name);
  else length-=connlen;

  length+=i*connlen;

  while(i--){
    char buffer[MAXPDSTRING];
    atom_string(argv++, buffer, MAXPDSTRING);
    length+=strlen(buffer);
  }

  if (length<0){
    outlet_symbol(x->x_obj.ob_outlet, gensym(""));
    return;
  }

  result = (char*)getbytes((length+1)*sizeof(char));

  /* 2nd create the symbol */
  if (x->s){
    char *buf = x->s->s_name;
    strcpy(result+len, buf);
    len+=strlen(buf);
    if(i && connector){
      strcpy(result+len, connector);
      len += connlen;
    }
  }
  i=argc;
  argv=x->ap;
  while(i--){
    char buffer[MAXPDSTRING];
    atom_string(argv++, buffer, MAXPDSTRING);
    strcpy(result+len, buffer);
    len += strlen(buffer);
    if(i && connector){
      strcpy(result+len, connector);
      len += connlen;
    }
  }
  result[length]=0;
  outlet_symbol(x->x_obj.ob_outlet, gensym(result));
  freebytes(result, (length+1)*sizeof(char));
}

static void list2symbol_anything(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  x->s =s;
  x->ac=argc;
  x->ap=argv;
  
  list2symbol_bang(x);
}

static void list2symbol_list(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  list2symbol_anything(x, 0, argc, argv);
}
static void *list2symbol_new(t_symbol *s, int argc, t_atom *argv)
{
  t_list2symbol *x = (t_list2symbol *)pd_new(list2symbol_class);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym(""));
  x->connector = gensym(" ");
  list2symbol_anything(x, 0, argc, argv);

  return (x);
}

static void list2symbol_free(t_list2symbol *x)
{}


static void list2symbol_setup(void)
{
  list2symbol_class = class_new(gensym("list2symbol"), (t_newmethod)list2symbol_new, 
			 (t_method)list2symbol_free, sizeof(t_list2symbol), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)list2symbol_new, gensym("l2s"), A_GIMME, 0);
  class_addbang    (list2symbol_class, list2symbol_bang);
  class_addlist    (list2symbol_class, list2symbol_list);
  class_addanything(list2symbol_class, list2symbol_anything);
  class_addmethod  (list2symbol_class, (t_method)list2symbol_connector, gensym(""), A_SYMBOL, 0);

  class_sethelpsymbol(list2symbol_class, gensym("zexy/list2symbol"));
}

/* ------------------------- symbol2list ------------------------------- */

static t_class *symbol2list_class;

typedef struct _symbol2list
{
  t_object x_obj;
  t_symbol *s, *delimiter;
  t_atom   *argv;
  int      argc, argnum; /* "argnum" is the number of reserved atoms (might be >argc) */
} t_symbol2list;

static void symbol2list_delimiter(t_symbol2list *x, t_symbol *s){
  x->delimiter = s;
}

STATIC_INLINE void string2atom(t_atom *ap, char* cp, int clen){
  char *buffer=getbytes(sizeof(char)*(clen+1));
  char *endptr[1];
  t_float ftest;
  strncpy(buffer, cp, clen);
  buffer[clen]=0;
  //  post("converting buffer '%s' %d", buffer, clen);
  ftest=strtod(buffer, endptr);
  if (*endptr == buffer){
    /* strtof() failed, we have a symbol */
    SETSYMBOL(ap, gensym(buffer));    
  } else {
    /* it is a number. */
    SETFLOAT(ap,ftest);
  }
  freebytes(buffer, sizeof(char)*(clen+1));
}
static void symbol2list_process(t_symbol2list *x)
{
  char *cc;
  char *deli;
  int   dell;
  char *cp, *d;
  int i=1;

  if (x->s==NULL){
    x->argc=0;
    return;
  }
  cc=x->s->s_name;
  cp=cc;
  
  if (x->delimiter==NULL || x->delimiter==&s_){
    i=strlen(cc);
    if(x->argnum<i){
      freebytes(x->argv, x->argnum*sizeof(t_atom));
      x->argnum=i+10;
      x->argv=getbytes(x->argnum*sizeof(t_atom));
    }
    x->argc=i;
    while(i--)string2atom(x->argv+i, cc+i, 1);
    return;
  }

  deli=x->delimiter->s_name;
  dell=strlen(deli);

  
  /* get the number of tokens */
  while((d=strstr(cp, deli))){
    if (d!=NULL && d!=cp){
      i++;
    }
    cp=d+dell;
  }

  /* resize the list-buffer if necessary */
  if(x->argnum<i){
    freebytes(x->argv, x->argnum*sizeof(t_atom));
    x->argnum=i+10;
    x->argv=getbytes(x->argnum*sizeof(t_atom));
  }
  x->argc=i;
  /* parse the tokens into the list-buffer */
  i=0;
  
    /* find the first token */
    cp=cc;
    while(cp==(d=strstr(cp,deli))){cp+=dell;}
    while((d=strstr(cp, deli))){
      if(d!=cp){
	string2atom(x->argv+i, cp, d-cp);
	i++;
      }
      cp=d+dell;
    }

    if(cp)string2atom(x->argv+i, cp, strlen(cp));
}
static void symbol2list_bang(t_symbol2list *x){
  symbol2list_process(x);
  if(x->argc)outlet_list(x->x_obj.ob_outlet, 0, x->argc, x->argv);
}
static void symbol2list_symbol(t_symbol2list *x, t_symbol *s){
  x->s = s;
  symbol2list_bang(x);
}
static void *symbol2list_new(t_symbol *s, int argc, t_atom *argv)
{
  t_symbol2list *x = (t_symbol2list *)pd_new(symbol2list_class);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym(""));

  x->argc=0;
  x->argnum=16;
  x->argv=getbytes(x->argnum*sizeof(t_atom));
  symbol2list_delimiter(x, (argc)?atom_getsymbol(argv):gensym(" "));
  
  return (x);
}

static void symbol2list_free(t_symbol2list *x)
{
}


static void symbol2list_setup(void)
{
  symbol2list_class = class_new(gensym("symbol2list"), (t_newmethod)symbol2list_new, 
			 (t_method)symbol2list_free, sizeof(t_symbol2list), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)symbol2list_new, gensym("s2l"), A_GIMME, 0);
  class_addsymbol (symbol2list_class, symbol2list_symbol);
  class_addbang   (symbol2list_class, symbol2list_bang);
  class_addmethod  (symbol2list_class, (t_method)symbol2list_delimiter, gensym(""), A_SYMBOL, 0);

  class_sethelpsymbol(symbol2list_class, gensym("zexy/symbol2list"));
}
/* global setup routine */

void z_strings_setup(void)
{
  atoi_setup();
  strcmp_setup();
  symbol2list_setup();
  list2symbol_setup();
}
