*! version 0.2.0 04Sep2018
*! Gandhi and Houde (2016) style instruments (A3; equation 10)

/*

This implements the A3 version of the instruments proposed by Gandhi and
Houde (2016). Given

    d_{jt,k} = x_jt - x_kt

the differences between the characteristics of products j and k in market t,
construct

    A^3_j(x_t) = ∑_{j ≠ j'} 1(|d_{jt, j'}| < k) x_jt

where k is some proximity threshold (e.g. the standard deviation of
x_jt across markets; in that case this would be the sum of competitor's
characteristics given they are within a standard devaition of j's
characteristics).

Reference: Gandhi, Amit and Houde, Jean-Fran. 2016 Measuring
Substitution Patterns in Differentiated Products Industries. Working
paper. https://pdfs.semanticscholar.org/63a9/b2cd63c4dc08524a6892a800bba302047fad.pdf

*/

capture program drop diffiv_mata
program diffiv_mata, sortpreserve
    syntax anything(equalok), /// instrument = characteristic
        good(varlist)         /// group for proximity threshold (st dev)
        market(varlist)       /// group for pairwise differences
        [replace]              // replace variable, if it exists

    gettoken Z X: anything, p(=)
    gettoken _ X: X,        p(=)

    confirm var `X'
    cap confirm new var `Z'
    local rc = _rc
    local X `X'
    local Z `Z'

    if ( `rc' == 0 ) {
        mata (void) st_addvar("`:set type'", "`Z'")
    }
    else {
        if ( "`replace'" == "" ) {
            disp as err "variable `Z' already defined"
            exit `rc'
        }
        else {
            confirm name `Z'
        }
    }

    qui {
        tempvar K grp
        egen `K' = sd(`X'), by(`good')
        replace `K' = 0 if mi(`K')
        bysort `market': gen byte `grp' = (_n == 1)
    }
    mata sumDiff()
end

cap mata mata drop sumDiff()
mata
void function sumDiff()
{
    real colvector _grp, _X, _x, _K, _k
    real colvector _A3, _a3
    real scalar i, j, starts, ends

    st_view(_grp, ., st_local("grp"))
    st_view(_X,   ., st_local("X"))
    st_view(_K,   ., st_local("K"))
    st_view(_A3,  ., st_local("Z"))

    N  = st_nobs()
    J  = sum(_grp)
    si = selectindex(_grp) \ (N + 1)

    for (j = 1; j <= J; j++) {
        starts = si[j]
        ends   = si[j + 1]
        nj     = ends - starts

        st_subview(_x,  _X,  (starts, ends - 1), .)
        st_subview(_k,  _K,  (starts, ends - 1), .)
        st_subview(_a3, _A3, (starts, ends - 1), .)

        for (i = 1; i <= nj; i++) {
            // _a3[i] = sum((abs(_x :- _x[i]) :<= _k[i]) :* _x) - _x[i]
            _a3[i] = sum(select(_x, abs(_x :- _x[i]) :<= _k[i])) - _x[i]
        }
    }
}
end
