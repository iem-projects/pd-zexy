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

#include "zexy.h"
#include <stdlib.h>
#include <string.h>

#ifndef NT
# define STATIC_INLINE static
#endif

/*
 * symbol2list: convert a symbol into a list (with given delimiters)
*/

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

void symbol2list_setup(void)
{
  symbol2list_class = class_new(gensym("symbol2list"), (t_newmethod)symbol2list_new, 
			 (t_method)symbol2list_free, sizeof(t_symbol2list), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)symbol2list_new, gensym("s2l"), A_GIMME, 0);
  class_addsymbol (symbol2list_class, symbol2list_symbol);
  class_addbang   (symbol2list_class, symbol2list_bang);
  class_addmethod  (symbol2list_class, (t_method)symbol2list_delimiter, gensym(""), A_SYMBOL, 0);

  class_sethelpsymbol(symbol2list_class, gensym("zexy/symbol2list"));
  zexy_register("symbol2list");
}
