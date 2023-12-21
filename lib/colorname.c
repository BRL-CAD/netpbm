/* colorname.c - colorname routines, not dependent on Netpbm formats
**
** Taken from libppm4.c May 2002.

** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1      /* Make sure strdup() is in string.h */
#endif
#define _XOPEN_SOURCE 500  /* Make sure strdup() is in string.h */

#include "netpbm/pm_c_util.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include "netpbm/nstring.h"
#include "netpbm/mallocvar.h"

#include "colorname.h"

static int lineNo;



void
pm_canonstr(char * arg) {
/*----------------------------------------------------------------------------
   Modify string 'arg' to canonical form: lower case, no white space.
-----------------------------------------------------------------------------*/
    char * srcCursor;
    char * dstCursor;

    for (srcCursor = arg, dstCursor = arg; *srcCursor; ++srcCursor) {
        if (!ISSPACE(*srcCursor)) {
            *dstCursor++ =
                ISUPPER(*srcCursor) ? tolower(*srcCursor) : *srcCursor;
        }
    }
}



FILE *
pm_openColornameFile(const char * const fileName, const int must_open) {
/*----------------------------------------------------------------------------
   Open the colorname dictionary file.  Its file name is 'fileName', unless
   'fileName' is NULL.  In that case, its file name is the value of the
   environment variable whose name is RGB_ENV (e.g. "RGBDEF").  Except
   if that environment variable is not set, it is RGB_DB1, RGB_DB2,
   or RGB_DB3 (e.g. "/usr/lib/X11/rgb.txt"), whichever exists.
   
   'must_open' is a logical: we must get the file open or die.  If
   'must_open' is true and we can't open the file (e.g. it doesn't
   exist), exit the program with an error message.  If 'must_open' is
   false and we can't open the file, just return a null pointer.
-----------------------------------------------------------------------------*/
    const char *rgbdef = NULL;
    FILE *f = NULL;

    if (fileName == NULL) {
        if ((rgbdef = getenv(RGBENV))==NULL) {
            /* The environment variable isn't set, so try the hardcoded
               default color name dictionary locations.
            */
            if ((f = fopen(RGB_DB1, "r")) == NULL &&
                (f = fopen(RGB_DB2, "r")) == NULL &&
                (f = fopen(RGB_DB3, "r")) == NULL && must_open) {
                pm_error("can't open color names dictionary file named "
                         "%s, %s, or %s "
                         "and Environment variable %s not set.  Set %s to "
                         "the pathname of your rgb.txt file or don't use "
                         "color names.", 
                         RGB_DB1, RGB_DB2, RGB_DB3, RGBENV, RGBENV);
            }
        } else {            
            /* The environment variable is set */
            if ((f = fopen(rgbdef, "r")) == NULL && must_open)
                pm_error("Can't open the color names dictionary file "
                         "named %s, per the %s environment variable.  "
                         "errno = %d (%s)",
                         rgbdef, RGBENV, errno, strerror(errno));
        }
    } else {
        f = fopen(fileName, "r");
        if (f == NULL && must_open)
            pm_error("Can't open the color names dictionary file '%s'.  "
                     "errno = %d (%s)", fileName, errno, strerror(errno));
        
    }
    lineNo = 0;
    return(f);
}



struct colorfile_entry
pm_colorget(FILE * const f) {
/*----------------------------------------------------------------------------
   Get next color entry from the color name dictionary file 'f'.

   If eof or error, return a color entry with NULL for the color name.

   Otherwise, return color name in static storage within.
-----------------------------------------------------------------------------*/
    char buf[200];
    static char colorname[200];
    bool gotOne;
    bool eof;
    struct colorfile_entry retval;
    char * rc;

    gotOne = FALSE;  /* initial value */
    eof = FALSE;
    while (!gotOne && !eof) {
        lineNo++;
        rc = fgets(buf, sizeof(buf), f);
        if (rc == NULL)
            eof = TRUE;
        else {
            if (buf[0] != '#' && buf[0] != '\n' && buf[0] != '!' &&
                buf[0] != '\0') {
                if (sscanf(buf, "%ld %ld %ld %[^\n]",
                           &retval.r, &retval.g, &retval.b, colorname)
                    == 4 )
                    gotOne = TRUE;
                else {
                    if (buf[strlen(buf)-1] == '\n')
                        buf[strlen(buf)-1] = '\0';
                    pm_message("can't parse color names dictionary Line %d:  "
                               "'%s'",
                               lineNo, buf);
                }
            }
        }
    }
    if (gotOne)
        retval.colorname = colorname;
    else
        retval.colorname = NULL;
    return retval;
}



void
pm_parse_dictionary_namen(char   const colorname[],
                          tuplen const color) {

    FILE * fP;
    bool gotit;
    bool colorfileExhausted;
    struct colorfile_entry colorfileEntry;
    char * canoncolor;

    fP = pm_openColornameFile(NULL, TRUE);  /* exits if error */
    canoncolor = strdup(colorname);

    if (!canoncolor)
        pm_error("Failed to allocate memory for %u-byte color name",
                 (unsigned)strlen(colorname));

    pm_canonstr(canoncolor);

    for(gotit = FALSE, colorfileExhausted = FALSE;
        !gotit && !colorfileExhausted; ) {

        colorfileEntry = pm_colorget(fP);
        if (colorfileEntry.colorname) {
            pm_canonstr(colorfileEntry.colorname);
            if (streq(canoncolor, colorfileEntry.colorname))
                gotit = TRUE;
        } else
            colorfileExhausted = TRUE;
    }
    fclose(fP);

    if (!gotit)
        pm_error("unknown color '%s'", colorname);

    color[PAM_RED_PLANE] = (samplen)colorfileEntry.r / PAM_COLORFILE_MAXVAL;
    color[PAM_GRN_PLANE] = (samplen)colorfileEntry.g / PAM_COLORFILE_MAXVAL;
    color[PAM_BLU_PLANE] = (samplen)colorfileEntry.b / PAM_COLORFILE_MAXVAL;

    free(canoncolor);
}



void
pm_parse_dictionary_name(char    const colorname[],
                         pixval  const maxval,
                         int     const closeOk,
                         pixel * const colorP) {

    double const epsilon = 1.0/65536.0;

    tuplen color;
    pixval r, g, b;

    MALLOCARRAY_NOFAIL(color, 3);

    pm_parse_dictionary_namen(colorname, color);

    r = ppm_unnormalize(color[PAM_RED_PLANE], maxval);
    g = ppm_unnormalize(color[PAM_GRN_PLANE], maxval);
    b = ppm_unnormalize(color[PAM_BLU_PLANE], maxval);

    if (!closeOk) {
        if (maxval != PAM_COLORFILE_MAXVAL) {
            if (fabs((double)r / maxval - color[PAM_RED_PLANE]) > epsilon ||
                fabs((double)g / maxval - color[PAM_GRN_PLANE]) > epsilon ||
                fabs((double)b / maxval - color[PAM_BLU_PLANE]) > epsilon) {
                pm_message("WARNING: color '%s' cannot be represented "
                           "exactly with a maxval of %u.  "
                           "Approximating as (%u,%u,%u).  "
                           "(The color dictionary uses maxval %u, so that "
                           "maxval will always work).",
                           colorname, maxval, r, g, b,
                           PAM_COLORFILE_MAXVAL);
            }
        }
    }

    PPM_ASSIGN(*colorP, r, g, b);
}



