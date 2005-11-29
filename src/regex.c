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

#ifdef HAVE_REGEX_H
# include <sys/types.h>
# include <regex.h>
# include <string.h>
#endif

# define NUM_REGMATCHES 10

/*
 * regex    : see whether a regular expression matches the given symbol
 */

/* ------------------------- regex ------------------------------- */

/* match a regular expression against a string */

static t_class *regex_class;

typedef struct _regex
{
  t_object x_obj;
#ifdef HAVE_REGEX_H
  regex_t *x_regexp;
  int x_matchnum;
#endif
} t_regex;

#ifdef HAVE_REGEX_H
static char*regex_l2s(int *reslen, t_symbol*s, int argc, t_atom*argv)
{
  char *result = 0;
  int pos=0, i=0;
  t_atom*ap;
  int length=0;
  if(reslen)*reslen=length;

  /* 1st get the length of the symbol */
  if(s)length+=strlen(s->s_name);
  else length-=1;
  length+=argc;

  i=argc;
  ap=argv;
  while(i--){
    char buffer[MAXPDSTRING];
    int len=0;
    if(A_SYMBOL==ap->a_type){
      len=strlen(ap->a_w.w_symbol->s_name);
    } else {
      atom_string(ap, buffer, MAXPDSTRING);
      len=strlen(buffer);
    }
    length+=len;
    ap++;
  }

  if(length<=0)return(0);

  result = (char*)getbytes((length+1)*sizeof(char));

  if (s) {
    char *buf = s->s_name;
    strcpy(result+pos, buf);
    pos+=strlen(buf);
    if(i){
      strcpy(result+pos, " ");
      pos += 1;
    }
  }

  ap=argv;
  i=argc;
  while(i--){
    if(A_SYMBOL==ap->a_type){
      strcpy(result+pos, ap->a_w.w_symbol->s_name);
      pos+= strlen(ap->a_w.w_symbol->s_name);
    } else {
      char buffer[MAXPDSTRING];
      atom_string(ap, buffer, MAXPDSTRING);
      strcpy(result+pos, buffer);
      pos += strlen(buffer);
    }
    ap++;
    if(i){
      strcpy(result+pos, " ");
      pos++;
    }
  }

  result[length]=0;
  if(reslen)*reslen=length;
  return result;
}
#endif

static void regex_regex(t_regex *x, t_symbol*s, int argc, t_atom*argv)
{
#ifdef HAVE_REGEX_H
  char*result=0;
  int length=0;
  t_atom*ap=argv;
  int i=argc;
  int flags =  0;
  flags |= REG_EXTENDED;

  result=regex_l2s(&length, 0, argc, argv);

  if(0==result || 0==length){
    pd_error(x, "[regex]: no regular expression given");
    return;
  }


  if(x->x_regexp){
   regfree(x->x_regexp);
   freebytes(x->x_regexp, sizeof(t_regex));
   x->x_regexp=0;
  }
  x->x_regexp=(regex_t*)getbytes(sizeof(t_regex));

  if(regcomp(x->x_regexp, result, flags)) {
    pd_error(x, "[regex]: invalid regular expression: %s", result);
    if(x->x_regexp)freebytes(x->x_regexp, sizeof(t_regex));
    x->x_regexp=0;
  }

  if(result)freebytes(result, length);
#endif
}
static void regex_symbol(t_regex *x, t_symbol *s, int argc, t_atom*argv)
{
#ifdef HAVE_REGEX_H
  char*teststring=0;
  int length=0;

  int num_matches=x->x_matchnum;
  regmatch_t*match=(regmatch_t*)getbytes(sizeof(regmatch_t)*num_matches);
  t_atom*ap=(t_atom*)getbytes(sizeof(t_atom)*(1+2*num_matches));
  int ap_length=0;

  int err=0;

  if(!x->x_regexp){
    pd_error(x, "[regex]: no regular expression!");
    goto cleanup;
  }
  teststring=regex_l2s(&length, 0, argc, argv);
  if(!teststring||!length){
    pd_error(x, "[regex]: cannot evaluate string");
    goto cleanup;
  }

  err=regexec(x->x_regexp, teststring, num_matches, match, 0);

  if(err) {
    ap_length=1;
    SETFLOAT(ap, 0.f);
  } else {
    int num_results=0;
    int i=0;
    t_atom*ap2=ap+1;
    for(i=0; i<num_matches; i++){
      if(match[i].rm_so!=-1){
        if(i>0 && (match[i].rm_so==match[i-1].rm_so) && (match[i].rm_eo==match[i-1].rm_eo)){
        } else {
          SETFLOAT(ap2+0, match[i].rm_so);
          SETFLOAT(ap2+1, match[i].rm_eo);
          ap2+=2;
          num_results++;
        }
      }
    }
    ap_length=1+2*num_results;
    SETFLOAT(ap, num_results);
  }

 cleanup:
  if(teststring)freebytes(teststring, length);
  if(match)freebytes(match, sizeof(regmatch_t)*num_matches);

  if(ap){
    outlet_list(x->x_obj.ob_outlet, gensym("list"), ap_length, ap);
    freebytes(ap, sizeof(t_atom)*(1+2*num_matches));
  }
#endif
}

static void *regex_new(t_symbol *s, int argc, t_atom*argv)
{
  t_regex *x = (t_regex *)pd_new(regex_class);

  outlet_new(&x->x_obj, 0);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym("regex"));

#ifdef HAVE_REGEX_H
  x->x_regexp=0;
  x->x_matchnum=NUM_REGMATCHES;
  if(argc)regex_regex(x, gensym(""), argc, argv);
#else
  error("[regex] non-functional: compiled without regex-support!");
#endif

  return (x);
}

static void regex_free(t_regex *x)
{
#ifdef HAVE_REGEX_H
  if(x->x_regexp) {
    regfree(x->x_regexp);
    freebytes(x->x_regexp, sizeof(t_regex));
    x->x_regexp=0;
  }
#endif
}


void regex_setup(void)
{
  regex_class = class_new(gensym("regex"), (t_newmethod)regex_new, 
			 (t_method)regex_free, sizeof(t_regex), 0, A_GIMME, 0);

  class_addlist  (regex_class, regex_symbol);
  class_addmethod  (regex_class, (t_method)regex_regex, gensym("regex"), A_GIMME, 0);

  class_sethelpsymbol(regex_class, gensym("zexy/regex"));
  zexy_register("regex");
}

void z_regex_setup(void)
{
  regex_setup();
}
