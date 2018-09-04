#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gttypes.h"
#include "spi/stplugin.h"
#include "difftools.h"

/**
 * @brief Short wrapper to print to Stata
 *
 * Basic wrapper to print formatted strings to Stata
 *
 * @param *fmt a string to format
 * @param ... Arguments to pass to pritnf
 * @return Prints to Stata's console
 */
void sf_printf (const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    char buf[BUF_MAX];
    vsprintf (buf, fmt, args);
    SF_display (buf);
    // printf (buf);
    va_end (args);
}

/**
 * @brief Short wrapper to print error to Stata
 *
 * Basic wrapper to print formatted error strings to Stata
 *
 * @param *fmt a string to format
 * @param ... Arguments to pass to pritnf
 * @return Prints to Stata's console
 */
void sf_errprintf (const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    char buf[BUF_MAX];
    vsprintf (buf, fmt, args);
    SF_error (buf);
    va_end (args);
}

/**
 * @brief Update a running timer and print a message to satata console
 *
 * Prints a messasge to Stata that the running timer @timer was last set
 * @diff seconds ago. It then updates the timer to the current time.
 *
 * @param timer clock object containing time since last udpate
 * @param msg message to print before # of seconds
 * @return Print time since last update to Stata console
 */
void sf_running_timer (clock_t *timer, const char *msg)
{
    double diff  = (double) (clock() - *timer) / CLOCKS_PER_SEC;
    sf_printf (msg);
    sf_printf ("; %.3f seconds.\n", diff);
    *timer = clock();
}

/**
 * @brief Read scalar into unsigned integer
 *
 * @param st_scalar name of Stata scalar
 * @param sval Scalar value
 * @return Read scalar into GT_size variable
 */
ST_retcode sf_scalar_size (char *st_scalar, GT_size *sval)
{
    ST_double _double;
    ST_retcode rc = SF_scal_use(st_scalar, &_double);
    if ( rc == 0 ) {
        *sval = (GT_size) _double;
    }
    return (rc);
}

/**
 * @brief Panel setup based on vector
 */
ST_retcode sf_oom_error (char *step_desc, char *obj_desc)
{
    sf_errprintf ("%s: Unable to allocate memory for object '%s'.\n", step_desc, obj_desc);
    return (1702);
}

/**
 * @brief Set up variables for panel hashes
 *
 * Using an index array, generate info array with start and
 * ending positions of each group in the sorted vector.
 *
 * @param group 0/1 array with the start of each group
 * @J Number of groups
 *
 * @return info arary with start and end positions of each group
 */
GT_size * sf_panelsetup(GT_bool *group, GT_size N, GT_size *G)
{
    GT_size i = 0, j = 0;
    GT_size *ginfo;
    GT_bool *gptr;

    *G = 0;
    for (gptr = group; gptr < group + N; gptr++)
        *G += *gptr;

    ginfo = calloc(*G + 1, sizeof ginfo);
    if ( ginfo == NULL ) {
        return (NULL);
    }
    else {
        for (gptr = group; gptr < group + N; gptr++, i++) {
            if ( *gptr ) ginfo[j++] = i;
        }
        ginfo[j] = N;
        return (ginfo);
    }
}

/**
 * @brief Parse stata vector into C array
 *
 * @param st_matrix name of stata matrix to get
 * @param v array where to store the vector
 * @return Store min and max of @x
 */
ST_retcode sf_get_vector_bool (char *st_matrix, GT_bool *v)
{
    ST_double z;
    ST_retcode rc = 0;

    GT_size i;
    GT_size ncol = SF_col(st_matrix);
    GT_size nrow = SF_row(st_matrix);
    if ( (ncol > 1) & (nrow > 1) ) {
        sf_errprintf("tried to read a "GT_size_cfmt" by "GT_size_cfmt" matrix into an array\n", nrow, ncol);
        return (198);
    }
    else if ( (ncol == 0) & (nrow == 0) ) {
        sf_errprintf("tried to read a "GT_size_cfmt" by "GT_size_cfmt" matrix into an array\n", nrow, ncol);
        return (198);
    }

    if ( ncol > 1 ) {
        for (i = 0; i < ncol; i++) {
            if ( (rc = SF_mat_el(st_matrix, 1, i + 1, &z)) ) return (rc);
            v[i] = (GT_bool) z;
        }
    }
    else {
        for (i = 0; i < nrow; i++) {
            if ( (rc = SF_mat_el(st_matrix, i + 1, 1, &z)) ) return (rc);
            v[i] = (GT_bool) z;
        }
    }

    return (rc);
}
