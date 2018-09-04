#ifndef DIFFIV_H
#define DIFFIV_H    1

#include <locale.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "difftools.c"

// Container structure for Stata-provided info
struct StataInfo {
    GT_size in1;
    GT_size in2;
    GT_size N;
    GT_size Nread;
    GT_size G;
    GT_size ng_max;
    GT_size free;
    //
    GT_size k;
    GT_bool benchmark;
    //
    GT_bool *empty;
    GT_size *index;
    GT_size *ginfo;
    GT_bool *grp;
    //
    ST_double *K;
    ST_double *X;
    ST_double *Z;
};

// Main functions
ST_retcode ssf_parse_info    (struct StataInfo *st_info);
ST_retcode ssf_read_varlist  (struct StataInfo *st_info);
ST_retcode ssf_diffiv        (struct StataInfo *st_info);
ST_retcode ssf_write_varlist (struct StataInfo *st_info);
void ssf_free (struct StataInfo *st_info);

#endif /* diffiv.h */
