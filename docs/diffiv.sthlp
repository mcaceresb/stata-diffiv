{smcl}
{* *! version 0.3.0 09Sep2018}{...}
{viewerdialog diffiv "dialog diffiv"}{...}
{vieweralsosee "[R] diffiv" "mansection R diffiv"}{...}
{viewerjumpto "Syntax" "diffiv##syntax"}{...}
{viewerjumpto "Description" "diffiv##description"}{...}
{viewerjumpto "Examples" "diffiv##examples"}{...}
{viewerjumpto "References" "diffiv##references"}{...}
{title:Title}

{p2colset 5 17 17 2}{...}
{p2col :{cmd:diffiv} {hline 2}}Fast implementation of Gandhi and Houde (2016) style instruments{p_end}
{p2colreset}{...}

{marker syntax}{...}
{title:Syntax}

{phang}
To run basic OLS

{p 8 15 2}
{cmd:diffiv}
{newvar} = {varlist}
{ifin}
{cmd:,}
{opth market(varlist)}
[{c -(}{opth good(varlist)}{c |}{opt thresh:old(varlist)}{c )-} {opt replace}]

{p2colreset}{...}
{p 4 6 2}

{marker description}{...}
{title:Description}

{pstd}
This package implements the A3 version of the instruments proposed by
Gandhi and Houde (2016). Currently this package is only available in
Stata for Unix (Linux) and OSX.

{pstd}
The user must additionally specify a variable identifying a good, in
which case each individual good's standard deviation is used as a
threshold, or a threshold variable.

{marker example}{...}
{title:Examples}

{phang2}{cmd:. sysuse auto}{p_end}
{phang2}{cmd:. egen j = group(make)}{p_end}
{phang2}{cmd:. expand 2, gen(t)}{p_end}
{phang2}{cmd:. diffiv priceIV = price, market(t) good(j)}{p_end}
{phang2}{cmd:. diffiv priceIV mpgIV = price mpg if foreign, market(t) good(j) replace}{p_end}

{marker references}{...}
{title:References}

{pstd}
Gandhi, Amit and Houde, Jean-Francois. 2016.  Measuring Substitution Patterns
in Differentiated Product Industries. {it:Working Paper}.
