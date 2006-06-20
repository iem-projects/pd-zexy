#!/bin/sh

## TODO:
##  find zexy (either in ../src or ../)
##  if it is not there, assume it is split into externals


#SUFFIX="$$"
SUFFIX=$(date +%y%m%d-%H%M%S)

RUNTESTS_TXT=runtests.txt
RUNTESTS_LOG=log-runtests.${SUFFIX}

ls -1 */*.pd | sed 's/\.pd/;/' > ${RUNTESTS_TXT}


LIBFLAGS="-path ../src:../ -lib zexy -path ../abs/"

function run_nogui() {
 pd ${LIBFLAGS} -nogui runtests_nogui.pd > ${RUNTESTS_LOG} 2>&1 
 NUMTESTS=`grep -c . ${RUNTESTS_TXT}`
 echo "regression-test: ${NUMTESTS} tests total" >>  ${RUNTESTS_LOG}
 
 cat ${RUNTESTS_LOG} | egrep "^regression-test: " | sed -e 's/^regression-test: //'
}

function run_withgui() {
 pd ${LIBFLAGS} -stderr runtests.pd > ${RUNTESTS_LOG} 2>&1
}

if test "x$1" = "x-gui"; then
 run_withgui
else
 run_nogui
fi


