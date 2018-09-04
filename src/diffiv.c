/*********************************************************************
 * Program: diffiv.c
 * Blame:   Mauricio Caceres Bravo <mauricio.caceres.bravo@gmail.com>
 * Created: Wed Aug  1 09:12:03 EDT 2018
 * Updated: Tue Sep  4 17:20:51 EDT 2018
 * Purpose: Stata plugin for Gandhi and Houde (2016) style instruments.
 * Note:    See stata.com/plugins for more on Stata plugins
 * Version: 0.1.0
 *********************************************************************/

/**
 * @file diffiv.c
 * @author Mauricio Caceres Bravo
 * @date 1 Sep 2018
 * @brief Stata plugin
 *
 * This file should only ever be called from diffiv.ado
 *
 * @see help diffiv
 * @see http://www.stata.com/plugins for more on Stata plugins
 */

#include "diffiv.h"

int main()
{
    return(0);
}

int WinMain()
{
    return(0);
}

STDLL stata_call(int argc, char *argv[])
{
    ST_retcode rc = 0;
    setlocale(LC_ALL, "");
    struct StataInfo *st_info = malloc(sizeof(*st_info));
    st_info->free = 0;
    clock_t timer = clock();

    if ( (rc = ssf_parse_info (st_info)) ) {
        goto exit;
    }
    if ( st_info->benchmark )
        sf_running_timer (&timer, "\tPlugin step 1: Parse stata info");

    if ( (rc = ssf_read_varlist (st_info)) ) {
        goto exit;
    }
    if ( st_info->benchmark )
        sf_running_timer (&timer, "\tPlugin step 2: Read in variables");

    if ( (rc = ssf_diffiv (st_info)) ) {
        goto exit;
    }
    if ( st_info->benchmark )
        sf_running_timer (&timer, "\tPlugin step 3: Computed pairwise differences");

    if ( (rc = ssf_write_varlist (st_info)) ) {
        goto exit;
    }
    if ( st_info->benchmark )
        sf_running_timer (&timer, "\tPlugin step 4: Copied differences back to Stata");

exit:
    ssf_free (st_info);
    free (st_info);
    return (rc);
}

/**
 * @brief Parse variable info from Stata
 *
 * @param st_info Pointer to container structure for Stata info
 * @return Stores in @st_info various info from Stata for the pugin run
 */
ST_retcode ssf_parse_info (struct StataInfo *st_info)
{
    ST_retcode rc = 0;
    GT_size i, ix, in1, in2, N;
    GT_size k, benchmark;

    in1 = SF_in1();
    in2 = SF_in2();
    N   = in2 - in1 + 1;
    ix  = 0;

    if ( (rc = sf_scalar_size("diffiv_k", &k) )) goto exit;
    if ( (rc = sf_scalar_size("diffiv_benchmark", &benchmark) )) goto exit;

    st_info->index = calloc(N, sizeof st_info->index);
    st_info->empty = calloc(k, sizeof st_info->empty);

    if ( st_info->index == NULL ) return (sf_oom_error("ssf_parse_info", "index"));
    if ( st_info->empty == NULL ) return (sf_oom_error("ssf_parse_info", "empty"));

    st_info->free = 1;

    if ( (rc = sf_get_vector_bool ("diffiv_empty", st_info->empty) )) goto exit;

    for (i = 0; i < N; i++) {
        if ( SF_ifobs(i + in1) ) {
            st_info->index[ix] = i;
            ix++;
        }
    }

    st_info->in1 = in1;
    st_info->in2 = in2;
    st_info->N   = ix;

    st_info->k = k;
    st_info->benchmark = benchmark;

exit:
    return(rc);
}

/**
 * @brief Read varlist to hash from stata
 *
 * @param st_info Pointer to container structure for Stata info
 * @return Stores in @st_info->st_charx the input variables
 */
ST_retcode ssf_read_varlist (struct StataInfo *st_info)
{
    ST_retcode rc = 0;
    ST_double z;
    GT_size g, i, j, k, sel, ng, nempty, pos;

    st_info->grp = calloc(st_info->N, sizeof st_info->grp);
    st_info->K   = calloc(st_info->N * st_info->k, sizeof st_info->K);
    st_info->X   = calloc(st_info->N * st_info->k, sizeof st_info->X);
    st_info->Z   = calloc(st_info->N * st_info->k, sizeof st_info->Z);

    if ( st_info->grp == NULL ) return (sf_oom_error("ssf_read_varlist", "grp"));
    if ( st_info->K   == NULL ) return (sf_oom_error("ssf_read_varlist", "K"));
    if ( st_info->X   == NULL ) return (sf_oom_error("ssf_read_varlist", "X"));
    if ( st_info->Z   == NULL ) return (sf_oom_error("ssf_read_varlist", "Z"));

    st_info->free = 2;

    // Read in variables
    for (i = 0; i < st_info->N; i++) {
        sel = st_info->index[i] + st_info->in1;

        // First variable is grp
        if ( (rc = SF_vdata(1, sel, &z)) ) goto exit;
        st_info->grp[i] = (GT_bool) z;

        // Next k are thresholds
        for (k = 0; k < st_info->k; k++) {
            j = k + st_info->k * i;
            if ( (rc = SF_vdata(2 + k, sel, st_info->K + j)) ) goto exit;
        }

        // Next k are sources
        for (k = 0; k < st_info->k; k++) {
            j = k + st_info->k * i;
            if ( (rc = SF_vdata(2 + st_info->k + k, sel, st_info->X + j)) ) goto exit;
        }
    }

    // Initialize IV
    for (i = 0; i < st_info->N; i++) {
        for (k = 0; k < st_info->k; k++) {
            j = k + st_info->k * i;
            st_info->Z[j] = 0;
        }
    }

    // Empty if applicable
    nempty = 0;
    for (k = 0; k < st_info->k; k++) {
        nempty += st_info->empty[k];
    }

    if ( nempty ) {
        pos = 2 + st_info->k + st_info->k;
        for (i = 0; i < SF_nobs(); i++) {
            for (k = 0; k < st_info->k; k++) {
                if ( st_info->empty[k] ) {
                    if ( (rc = SF_vstore(pos + k, i + 1, SV_missval)) ) goto exit;
                }
            }
        }
    }

    // Allocate memory for groups
    st_info->ginfo = sf_panelsetup(st_info->grp, st_info->N, &(st_info->G));
    if ( st_info->ginfo == NULL ) return (sf_oom_error("ssf_read_varlist", "ginfo"));
    st_info->free = 3;

    // Largest group
    st_info->ng_max = 0;
    for (g = 0; g < st_info->G; g++) {
        ng = st_info->ginfo[g + 1] - st_info->ginfo[g];
        if ( ng > st_info->ng_max ) st_info->ng_max = ng;
    }

exit:
    return(rc);
}

/**
 * @brief Compute pairwise differences
 *
 * @param st_info Pointer to container structure for Stata info
 * @return Computes pairwise differences in Z
 */
ST_retcode ssf_diffiv (struct StataInfo *st_info)
{
    ST_double xi, xii, s;
    GT_size i, ii, j, k, seli, selii, starts, ends;

    for (j = 0; j < st_info->G; j++) {
        starts = st_info->ginfo[j];
        ends   = st_info->ginfo[j + 1];
        for (i = starts; i < ends; i++) {
            for (ii = starts; ii < ends; ii++) {
                for (k = 0; k < st_info->k; k++) {
                    seli  = k + st_info->k * i;
                    selii = k + st_info->k * ii;
                    s     = st_info->K[seli];
                    xi    = st_info->X[seli];
                    xii   = st_info->X[selii];
                    if ( fabs(xi - xii) <= s ) {
                        st_info->Z[seli] += xii;
                    }
                }
            }
            for (k = 0; k < st_info->k; k++) {
                seli = k + st_info->k * i;
                st_info->Z[seli] -= st_info->X[seli];
            }
        }
    }

    return(0);
}

/**
 * @brief Write instruments back to stata
 *
 * @param st_info Pointer to container structure for Stata info
 * @return Stores in stata the difference IV
 */
ST_retcode ssf_write_varlist (struct StataInfo *st_info)
{
    ST_retcode rc = 0;
    GT_size i, j, k, sel, pos;
    pos = 2 + st_info->k + st_info->k;
    for (i = 0; i < st_info->N; i++) {
        sel = st_info->index[i] + st_info->in1;
        for (k = 0; k < st_info->k; k++) {
            j = k + st_info->k * i;
            if ( (rc = SF_vstore(pos + k, sel, st_info->Z[j])) ) goto exit;
        }
    }

exit:
    return(rc);
}

/**
 * @brief Clean up st_info
 *
 * @param st_info Pointer to container structure for Stata info
 * @return Frees memory allocated to st_info objects
 */
void ssf_free (struct StataInfo *st_info)
{
    if ( st_info->free >= 1 ) {
        free (st_info->index);
    }
    if ( st_info->free >= 2 ) {
        free (st_info->grp);
        free (st_info->K);
        free (st_info->X);
        free (st_info->Z);
    }
    if ( st_info->free >= 3 ) {
        free (st_info->ginfo);
    }
}
