#if 0
=============================================================================
HERE IS AN EXAMPLE OF THE USE OF SHHOPT:


#include <shhopt.h>
int
main ( int argc, char **argv ) {

    /* initial values here are just to demonstrate what gets set and
       what doesn't by the code below.
    */
    int help_flag = 7;
    unsigned int help_spec =7;
    unsigned int height_spec =7;
    unsigned int name_spec= 7;
    char *name= "initial";
    int height=7;
    int verbose_flag=7;
    int debug_flag=7;
    char ** methodlist;
    struct optNameValue * optlist;

    optStruct3 opt;
    unsigned int option_def_index = 0;
    optEntry * option_def;
    MALLOCARRAY(option_def, 100);

    OPTENT3(0,   "help",     OPT_FLAG,       &help_flag,    &help_spec,   0);
    OPTENT3(0,   "height",   OPT_INT,        &height,       &height_spec, 0);
    OPTENT3('n', "name",     OPT_STRING,     &name,         &name_spec,   0);
    OPTENT3('v', "verbose",  OPT_FLAG,       &verbose_flag, NULL,         0);
    OPTENT3('g', "debug",    OPT_FLAG,       &debug_flag,   NULL,         0);
    OPTENT3(0,   "methods",  OPT_STRINGLIST, &methodlist,   NULL,         0);
    OPTENT3(0,   "options",  OPT_NAMELIST,   &optlist,      NULL,         0);

    opt.opt_table = option_def;
    opt.short_allowed = 1;
    opt.allowNegNum = 1;


    pm_optParseOptions3(&argc, argv, opt, sizeof(opt), 0);


    printf("argc=%d\n", argc);
    printf("help_flag=%d\n", help_flag);
    printf("help_spec=%d\n", help_spec);
    printf("height=%d\n", height);
    printf("height_spec=%d\n", height_spec);
    printf("name='%s'\n", name);
    printf("name_spec=%d\n", name_spec);
    printf("verbose_flag=%d\n", verbose_flag);
    printf("debug_flag=%d\n", verbose_flag);

    if (methodlist) {
        unsigned int i;
        printf("methods: ");
        while (methodlist[i]) {
            printf("'%s', ", methodlist[i]);
            ++i;
        }
        free(methodlist);
    } else
        printf("No -options\n");

    if (optlist) {
        unsigned int i;
        while (optlist[i].name) {
            printf("option '%s' = '%s'\n", optlist[i].name, optlist[i].value);
            ++i;
        }
        pm_optDestroyNameValueList(optlist);
    } else
        printf("No -options\n");
}

Now run this program with something like

  myprog -vg --name=Bryan --hei 4 "My first argument" --help

  or do it with opt.short_allowed=0 and

  myprog -v /etc/passwd -name=Bryan --hei=4


  If your program takes no options (so you have no OPTENT3 macro invocations),
  you need an OPTENTINIT call to establish the empty option table:

    optEntry * option_def;
    MALLOCARRAY(option_def, 1);
    OPTENTINIT;

========================================================================
#endif /* 0 */

#ifndef SHHOPT_H
#define SHHOPT_H

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* to fake out automatic code indenters */
#endif

/* constants for recognized option types. */
typedef enum {
    OPT_END,               /* nothing. used as ending element. */
    OPT_FLAG,              /* no argument following. sets variable to 1. */
    OPT_STRING,            /* string argument. */
    OPT_INT,               /* signed integer argument. */
    OPT_UINT,              /* unsigned integer argument. */
    OPT_LONG,              /* signed long integer argument. */
    OPT_ULONG,             /* unsigned long integer argument. */
    OPT_FLOAT,             /* floating point argument. */
    OPT_STRINGLIST,        /* list like "arg1,arg2.arg3" */
    OPT_NAMELIST           /* list like "key1=val1,key2=val2" */
} optArgType;


typedef struct {
    /* This structure describes a single program option in a form for
     use by the pm_optParseOptions() or pm_optParseOptions2() function.
    */
    char       shortName;  /* short option name. */
    const char *longName;  /* long option name, not including '--'. */
    optArgType type;       /* option type. */
    void       *arg;       /* pointer to variable to fill with argument,
                            * or pointer to function if type == OPT_FUNC. */
    int        flags;      /* modifier flags. */
} optStruct;

typedef struct {
    /* This structure describes a single program option in a form for
     use by the pm_optParseOptions3() function.
    */
    char       shortName;  /* short option name. */
    const char *longName;  /* long option name, not including '--' or '-' */
    optArgType type;       /* option type. */
    void       *arg;
        /* pointer to variable in which to return option's argument (or TRUE
           if it's a flag option), or pointer to function if
           type == OPT_FUNC.  If the option is specified multiple times, only
           the rightmost one affects this return value.
        */
    unsigned int *specified;
        /* pointer to variable in which to return the number of times that
           the option was specified.  If NULL, don't return anything.
        */
    int        flags;      /* modifier flags. */
} optEntry;


typedef struct {
    /* This structure describes the options of a program in a form for
       use with the pm_optParseOptions2() function.
       */
    unsigned char short_allowed;  /* boolean */
        /* The syntax may include short (i.e. one-character) options.
           These options may be stacked within a single token (e.g.
           -abc = -a -b -c).  If this value is not true, the short option
           member of the option table entry is meaningless and long
           options may have either one or two dashes.
           */
    unsigned char allowNegNum;  /* boolean */
        /* Anything that starts with - and then a digit is a numeric
           parameter, not an option
           */
    optStruct *opt_table;
} optStruct2;

typedef struct {
    /* Same as optStruct2, but for pm_optParseOptions3() */
    unsigned char short_allowed;
    unsigned char allowNegNum;
    optEntry *opt_table;
} optStruct3;


/* You can use OPTENTRY to assign a value to a dynamically or automatically
   allocated optStruct structure with minimal typing and easy readability.

   Here is an example:

       unsigned int option_def_index = 0;
       optStruct *option_def = malloc(100*sizeof(optStruct));
       OPTENTRY('h', "help",     OPT_FLAG, &help_flag, 0);
       OPTENTRY(0,   "alphaout", OPT_STRING, &alpha_filename, 0);
*/

/* If you name your variables option_def and option_def_index like in the
   example above, everything's easy.  If you want to use OPTENTRY with other
   variables, define macros OPTION_DEF and OPTION_DEF_INDEX before calling
   OPTENTRY.
*/

#ifndef OPTION_DEF
#define OPTION_DEF option_def
#endif
#ifndef OPTION_DEF_INDEX
#define OPTION_DEF_INDEX option_def_index
#endif

#define OPTENTRY(shortvalue,longvalue,typevalue,outputvalue,flagvalue) {\
    OPTION_DEF[OPTION_DEF_INDEX].shortName = (shortvalue); \
    OPTION_DEF[OPTION_DEF_INDEX].longName = (longvalue); \
    OPTION_DEF[OPTION_DEF_INDEX].type = (typevalue); \
    OPTION_DEF[OPTION_DEF_INDEX].arg = (outputvalue); \
    OPTION_DEF[OPTION_DEF_INDEX].flags = (flagvalue); \
    OPTION_DEF[++OPTION_DEF_INDEX].type = OPT_END; \
    }

/* OPTENT3 is the same as OPTENTRY except that it also sets the "specified"
   element of the table entry (so it assumes OPTION_DEF is a table of optEntry
   instead of optStruct).  This is a pointer to a variable that the parser
   will set to the number of times that the option appears in the command
   line.

   Here is an example:

       unsigned int option_def_index = 0;
       MALLOCARRAY_NOFAIL(option_def, 100);
       OPTENT3('h', "help",     OPT_FLAG, &help_flag, 0);
       OPTENT3(0,   "alphaout", OPT_STRING, &alpha_filename, 0);
*/

#define OPTENT3(shortvalue,longvalue,typevalue,outputvalue,specifiedvalue, \
                flagvalue) {\
    OPTION_DEF[OPTION_DEF_INDEX].specified = (specifiedvalue); \
    OPTENTRY(shortvalue, longvalue, typevalue, outputvalue, flagvalue) \
    }

#define OPTENTINIT \
    do {OPTION_DEF_INDEX=0; \
        OPTION_DEF[OPTION_DEF_INDEX].type = OPT_END; \
    } while (0)


struct optNameValue {
    const char * name;
    const char * value;
};



void
pm_optSetFatalFunc(void (*f)(const char *, ...));

void
pm_optParseOptions(int *argc, char *argv[],
                   optStruct opt[], int allowNegNum);
void
pm_optParseOptions2(int * const argc_p, char *argv[], const optStruct2 opt,
                 const unsigned long flags);
void
pm_optParseOptions3(int * const argc_p, char *argv[], const optStruct3 opt,
                 const unsigned int optStructSize, const unsigned long flags);

void
pm_optDestroyNameValueList(struct optNameValue * const list);

#ifdef __cplusplus
}
#endif

#endif
