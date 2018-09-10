*! version 0.3.0 09Sep2018
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

capture program drop diffiv
program diffiv, sortpreserve
    if ( inlist("`c(os)'", "MacOSX") | strpos("`c(machine_type)'", "Mac") ) local c_os_ macosx
    else local c_os_: di lower("`c(os)'")

    if ( !inlist("`c_os_'", "unix", "macosx") ) {
        disp as err `"-diffiv- is not available for `c_os_'."'
        exit 198
    }

    diffiv_timer on 90
    syntax anything(equalok) /// instrument = characteristic
        [if] [in]            /// [if condition] [in start / end]
        ,                    ///
        market(varlist)      /// group for pairwise differences
        [THRESHold(varlist)] /// proximity threshold
        [good(varlist)]      /// group for proximity threshold (st dev)
        [replace]            /// replace variable, if it exists
        [BENCHmark]           // benchmark all the things

    gettoken Z X: anything, p(=)
    gettoken _ X: X,        p(=)

    confirm var `X'
    cap confirm new var `Z'
    local rc = _rc
    local X `X'
    local Z `Z'

    local kx: list sizeof X
    local kz: list sizeof Z
    if ( `kx' != `kz' ) {
        disp as err "specify the same number of inputs and outputs"
        exit 198
    }

    matrix diffiv_empty = 0
    if ( `rc' == 0 ) {
        mata (void) st_addvar(J(1, `kz', `"`:set type'"'), tokens(`"`Z'"'))
    }
    else {
        if ( "`replace'" == "" ) {
            disp as err "variable `Z' already defined"
            exit `rc'
        }
        else {
            confirm name `Z'
            matrix diffiv_empty = J(1, `:list sizeof Z', 0)
            local i = 1
            local newvars
            foreach var of local Z {
                cap confirm new var `var'
                if ( _rc == 0 ) {
                    local newvars `newvars' `var'
                }
                else if (`"`if'`in'"' != "") {
                    matrix diffiv_empty[1, `i++'] = 1
                }
            }
            if ( `"`newvars'"' != "" ) {
                qui mata (void) st_addvar(J(1, `:list sizeof newvars', `"`:set type'"'), invtokens(`"`newvars'"'))
            }
        }
    }

    if ( (`"`threshold'"' == "") & (`"`good'"' == "") ) {
        disp as err "specify one of {opt threshold()} or {opt good()}"
    }

    if ( (`"`threshold'"' != "") & (`"`good'"' != "") ) {
        disp as err "specify only one of {opt threshold()} or {opt good()}"
    }

    marksample touse, novarlist
    markout `touse' `market', strok
    qui {
        if ( `"`threshold'"' == "" ) {
            local K
            forvalues k = 1 / `kz' {
                tempvar K`k'
                qui egen `K`k'' = sd(`:word `k' of `X''), by(`good')
                qui replace `K`k'' = 0 if mi(`K`k'')
                local K `K' `K`k''
            }
        }
        else {
            local K `threshold'
            if ( `:list sizeof K' != `kz' ) {
                disp as err "specify the same number of threshold variables"
                exit 198
            }
            foreach var of varlist `K' {
                qui count if (`var' < 0)
                if ( `r(N)' > 0 ) {
                    disp as err "threshold() variable cannot have negative values"
                    exit 198
                }
            }
        }
        markout `touse' `K'

        tempvar grp
        sort `touse' `market'
        by `touse' `market': gen byte `grp' = (_n == 1) if `touse'
    }

    if ( "`benchmark'" != "" ) {
        diffiv_timer info 90 `"Parsed groups and threshold"', prints(1)
    }

    scalar diffiv_k         = `kz'
    scalar diffiv_benchmark = "`benchmark'" != ""
    cap noi plugin call diffiv_plugin `grp' `K' `X' `Z' if `touse' `in'
    local rc = _rc

    if ( ("`benchmark'" != "") & (`rc' == 0) ) {
        diffiv_timer info 90 `"Ran pairwise differences"', prints(1)
        diffiv_timer off 90
    }

    clean_exit
    exit `rc'
end

capture program drop diffiv_timer
program diffiv_timer, rclass
    syntax anything, [prints(int 0) end off]
    tokenize `"`anything'"'
    local what  `1'
    local timer `2'
    local msg   `"`3'; "'

    if ( inlist("`what'", "start", "on") ) {
        cap timer off `timer'
        cap timer clear `timer'
        timer on `timer'
    }
    else if ( inlist("`what'", "info") ) {
        timer off `timer'
        qui timer list
        return scalar t`timer' = `r(t`timer')'
        return local pretty`timer' = trim("`:di %21.4gc r(t`timer')'")
        if ( `prints' ) di `"`msg'`:di trim("`:di %21.4gc r(t`timer')'")' seconds"'
        timer off `timer'
        timer clear `timer'
        timer on `timer'
    }

    if ( "`end'`off'" != "" ) {
        timer off `timer'
        timer clear `timer'
    }
end

capture program drop clean_exit
program clean_exit
    cap scalar drop diffiv_k
    cap scalar drop diffiv_benchmark
    cap matrix drop diffiv_empty
end

if ( inlist("`c(os)'", "MacOSX") | strpos("`c(machine_type)'", "Mac") ) local c_os_ macosx
else local c_os_: di lower("`c(os)'")

cap program drop diffiv_plugin
program diffiv_plugin, plugin using("diffiv_`c_os_'.plugin")
