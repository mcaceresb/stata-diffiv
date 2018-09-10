set seed 42
set rmsg on
set more off
set linesize 196

sysuse auto
egen j = group(make)
expand 3, gen(t)
replace price = price + 10 * rnormal()
diffiv priceIV = price, market(t) good(j)
diffiv priceIV mpgIV = price mpg if foreign, market(t) good(j) replace

clear
set obs 10
gen t = _n > 5
bys t: gen j = _n 
gen p1 = runiform()
gen p2 = runiform()
gegen k1 = sd(p1), by(j)
gegen k2 = sd(p2), by(j)
diffiv_mata p1_A3_ = p1, market(t) good(j)
diffiv_mata p2_A3_ = p2, market(t) good(j)
diffiv p1_A3 p2_A3 = p1 p2, market(t) good(j)
l

sysuse auto, clear
expand 100
bys foreign: gen j = _n
diffiv priceA3 = price, market(foreign) good(j)
diffiv_mata priceA3_ = price, market(foreign) good(j)

sysuse auto, clear
bys foreign: gen j = _n
expand 1000
diffiv priceA3 = price, market(foreign) good(j) bench
* tempfile zz
* save `"`zz'"'
