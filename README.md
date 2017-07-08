# libtemplate
C  Template Engine I wrote in 2002

# echo 'abc${foo}efg | template -k foo=bar 
abcBARefg 

# echo 'abc123efx' | template -r '/abc([0-9]*)efg/number $1/' 
number 123


