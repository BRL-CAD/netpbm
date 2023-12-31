/* ppmrough.c - create a PPM image containing two colors with a ragged
   border between them
**
** Copyright (C) 2002 by Eckard Specht.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.  */

#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include "pm_c_util.h"
#include "mallocvar.h"
#include "shhopt.h"
#include "ppm.h"

static pixel** PIX;
static pixval BG_RED, BG_GREEN, BG_BLUE;


struct CmdlineInfo {
  /* All the information the user supplied in the command line,
     in a form easy for the program to use.
  */
    unsigned int left, right, top, bottom;
    unsigned int width, height, var;
    const char * bg_rgb;
    const char * fg_rgb;
    unsigned int randomseed;
    unsigned int randomseedSpec;
    unsigned int verbose;
};


static void
parseCommandLine(int argc, const char ** argv,
                 struct CmdlineInfo * const cmdlineP) {

    optEntry * option_def;
        /* Instructions to OptParseOptions2 on how to parse our options.    */
    optStruct3 opt;

    unsigned int option_def_index;

    MALLOCARRAY(option_def, 100);

    option_def_index = 0;   /* incremented by OPTENTRY */
    OPTENT3(0, "width",       OPT_UINT,   &cmdlineP->width,   NULL, 0);
    OPTENT3(0, "height",      OPT_UINT,   &cmdlineP->height,  NULL, 0);
    OPTENT3(0, "left",        OPT_UINT,   &cmdlineP->left,    NULL, 0);
    OPTENT3(0, "right",       OPT_UINT,   &cmdlineP->right,   NULL, 0);
    OPTENT3(0, "top",         OPT_UINT,   &cmdlineP->top,     NULL, 0);
    OPTENT3(0, "bottom",      OPT_UINT,   &cmdlineP->bottom,  NULL, 0);
    OPTENT3(0, "bg",          OPT_STRING, &cmdlineP->bg_rgb,  NULL, 0);
    OPTENT3(0, "fg",          OPT_STRING, &cmdlineP->fg_rgb,  NULL, 0);
    OPTENT3(0, "var",         OPT_UINT,   &cmdlineP->var,     NULL, 0);
    OPTENT3(0, "randomseed",  OPT_UINT,   &cmdlineP->randomseed,
            &cmdlineP->randomseedSpec, 0);
    OPTENT3(0, "init",        OPT_UINT,   &cmdlineP->randomseed,
            &cmdlineP->randomseedSpec, 0);
    OPTENT3(0, "verbose",     OPT_FLAG,   NULL, &cmdlineP->verbose, 0);

    /* Set the defaults */
    cmdlineP->width = 100;
    cmdlineP->height = 100;
    cmdlineP->left = cmdlineP->right = cmdlineP->top = cmdlineP->bottom = -1;
    cmdlineP->bg_rgb = NULL;
    cmdlineP->fg_rgb = NULL;
    cmdlineP->var = 10;

    opt.opt_table = option_def;
    opt.short_allowed = FALSE;  /* We have no short (old-fashioned) options */
    opt.allowNegNum = FALSE;  /* We have no parms that are negative numbers */

    pm_optParseOptions3(&argc, (char **)argv, opt, sizeof(opt), 0);

    if (argc-1 != 0)
        pm_error("There are no arguments.  You specified %d.", argc-1);

    free(option_def);
}



static void
procLeft(int          const r1,
         int          const r2,
         int          const c1,
         int          const c2, 
         unsigned int const width,
         unsigned int const var) {

    int cm, rm, c;

    if (r1 + 1 == r2)  return;
    rm = (r1 + r2) >> 1;
    cm = (c1 + c2) >> 1;
    cm += (int)floor(((float)rand() / RAND_MAX - 0.5) * var + 0.5);

    for (c = 0; c < MIN(width, MAX(0, cm)); c++)
        PPM_ASSIGN(PIX[rm][c], BG_RED, BG_GREEN, BG_BLUE);

    procLeft(r1, rm, c1, cm, width, var);
    procLeft(rm, r2, cm, c2, width, var);
}



static void
procRight(int          const r1,
          int          const r2,
          int          const c1,
          int          const c2,
          unsigned int const width,
          unsigned int const var) {

    int cm, rm, c;

    if (r1 + 1 == r2)  return;
    rm = (r1 + r2) >> 1;
    cm = (c1 + c2) >> 1;
    cm += (int)floor(((float)rand() / RAND_MAX - 0.5) * var + 0.5);

    for (c = cm; c < width; c++)
        PPM_ASSIGN(PIX[rm][c], BG_RED, BG_GREEN, BG_BLUE);

    procRight(r1, rm, c1, cm, width, var);
    procRight(rm, r2, cm, c2, width, var);
}



static void
procTop(int          const c1,
        int          const c2,
        int          const r1,
        int          const r2,
        unsigned int const height,
        unsigned int const var) {

    int rm, cm, r;

    if (c1 + 1 == c2)  return;
    cm = (c1 + c2) >> 1;
    rm = (r1 + r2) >> 1;
    rm += (int)floor(((float)rand() / RAND_MAX - 0.5) * var + 0.5);

    for (r = 0; r < MIN(height, MAX(0, rm)); r++)
        PPM_ASSIGN(PIX[r][cm], BG_RED, BG_GREEN, BG_BLUE);

    procTop(c1, cm, r1, rm, height, var);
    procTop(cm, c2, rm, r2, height, var);
}



static void
procBottom(int          const c1,
           int          const c2,
           int          const r1,
           int          const r2,
           unsigned int const height,
           unsigned int const var) {

    int rm, cm, r;

    if (c1 + 1 == c2)  return;
    cm = (c1 + c2) >> 1;
    rm = (r1 + r2) >> 1;
    rm += (int)floor(((float)rand() / RAND_MAX - 0.5) * var + 0.5);

    for (r = rm; r < height; r++)
        PPM_ASSIGN(PIX[r][cm], BG_RED, BG_GREEN, BG_BLUE);

    procBottom(c1, cm, r1, rm, height, var);
    procBottom(cm, c2, rm, r2, height, var);
}



int
main(int argc, const char * argv[]) {

    struct CmdlineInfo cmdline;
    pixel bgcolor, fgcolor;
    pixval fg_red, fg_green, fg_blue;
    int rows, cols, row;
    int left, right, top, bottom;

    pm_proginit(&argc, argv);

    parseCommandLine(argc, argv, &cmdline);

    srand(cmdline.randomseedSpec ? cmdline.randomseed : pm_randseed());

    cols = cmdline.width;
    rows = cmdline.height;
    left = cmdline.left;
    right = cmdline.right;
    top = cmdline.top;
    bottom = cmdline.bottom;

    if (cmdline.bg_rgb)
        bgcolor = ppm_parsecolor(cmdline.bg_rgb, PPM_MAXMAXVAL);
    else
        PPM_ASSIGN(bgcolor, 0, 0, 0);
    BG_RED = PPM_GETR(bgcolor);
    BG_GREEN = PPM_GETG(bgcolor);
    BG_BLUE = PPM_GETB(bgcolor);

    if (cmdline.fg_rgb)
        fgcolor = ppm_parsecolor(cmdline.fg_rgb, PPM_MAXMAXVAL);
    else
        PPM_ASSIGN(fgcolor, PPM_MAXMAXVAL, PPM_MAXMAXVAL, PPM_MAXMAXVAL);
    fg_red = PPM_GETR(fgcolor);
    fg_green = PPM_GETG(fgcolor);
    fg_blue = PPM_GETB(fgcolor);

    if (cmdline.verbose) {
        pm_message("width is %d, height is %d, variance is %d.", 
                   cols, rows, cmdline.var);
        if (left >= 0)
            pm_message("ragged left border is required");
        if (right >= 0)
            pm_message("ragged right border is required");
        if (top >= 0)
            pm_message("ragged top border is required");
        if (bottom >= 0)
            pm_message("ragged bottom border is required");
        pm_message("background is %s",
                   ppm_colorname(&bgcolor, PPM_MAXMAXVAL, 1));
        pm_message("foreground is %s",
                   ppm_colorname(&fgcolor, PPM_MAXMAXVAL, 1));
        if (cmdline.randomseedSpec)
            pm_message("srand() initialized with seed %u", cmdline.randomseed);
    }

    /* Allocate memory for the whole pixmap */
    PIX = ppm_allocarray(cols, rows);

    /* First, set all pixel to foreground color */
    for (row = 0; row < rows; row++) {
        unsigned int col;
        for (col = 0; col < cols; ++col)
            PPM_ASSIGN(PIX[row][col], fg_red, fg_green, fg_blue);
    }
    /* Make a ragged left border */
    if (left >= 0) {
        int const left_c1 = left;
        int const left_c2 = left;
        int const left_r1 = 0;
        int const left_r2 = rows - 1;

        unsigned int col;

        for (col = 0; col < left_c1; ++col)
            PPM_ASSIGN(PIX[left_r1][col], BG_RED, BG_GREEN, BG_BLUE);
        for (col = 0; col < left_c2; ++col)
            PPM_ASSIGN(PIX[left_r2][col], BG_RED, BG_GREEN, BG_BLUE);

        procLeft(left_r1, left_r2, left_c1, left_c2, cmdline.width, cmdline.var);
    }

    /* Make a ragged right border */
    if (right >= 0) {
        int const right_c1 = cols - right - 1;
        int const right_c2 = cols - right - 1;
        int const right_r1 = 0;
        int const right_r2 = rows - 1;

        unsigned int col;

        for (col = right_c1; col < cols; col++)
            PPM_ASSIGN(PIX[right_r1][col], BG_RED, BG_GREEN, BG_BLUE);
        for (col = right_c2; col < cols; col++)
            PPM_ASSIGN(PIX[right_r2][col], BG_RED, BG_GREEN, BG_BLUE);

        procRight(right_r1, right_r2, right_c1, right_c2, 
                   cmdline.width, cmdline.var);
    }

    /* Make a ragged top border */
    if (top >= 0) {
        int const top_r1 = top;
        int const top_r2 = top;
        int const top_c1 = 0;
        int const top_c2 = cols - 1;

        unsigned int row;

        for (row = 0; row < top_r1; ++row)
            PPM_ASSIGN(PIX[row][top_c1], BG_RED, BG_GREEN, BG_BLUE);
        for (row = 0; row < top_r2; ++row)
            PPM_ASSIGN(PIX[row][top_c2], BG_RED, BG_GREEN, BG_BLUE);

        procTop(top_c1, top_c2, top_r1, top_r2, cmdline.height, cmdline.var);
    }

    /* Make a ragged bottom border */
    if (bottom >= 0) {
        int const bottom_r1 = rows - bottom - 1;
        int const bottom_r2 = rows - bottom - 1;
        int const bottom_c1 = 0;
        int const bottom_c2 = cols - 1;

        unsigned int row;

        for (row = bottom_r1; row < rows; ++row)
            PPM_ASSIGN(PIX[row][bottom_c1], BG_RED, BG_GREEN, BG_BLUE);
        for (row = bottom_r2; row < rows; ++row)
            PPM_ASSIGN(PIX[row][bottom_c2], BG_RED, BG_GREEN, BG_BLUE);

        procBottom(bottom_c1, bottom_c2, bottom_r1, bottom_r2, 
                   cmdline.height, cmdline.var);
    }

    /* Write pixmap */
    ppm_writeppm(stdout, PIX, cols, rows, PPM_MAXMAXVAL, 0);

    ppm_freearray(PIX, rows);

    pm_close(stdout);

    return 0;
}



