#!/bin/sh

## TODO:
##  find zexy (either in ../src or ../)
##  if it is not there, assume it is split into externals

if [ "x${PD}" = "x" ]
then
 PD=pd
fi


#SUFFIX="$$"
SUFFIX=$(date +%y%m%d-%H%M%S)

RUNTESTS_TXT=runtests.txt
RUNTESTS_LOG=log-runtests.${SUFFIX}

LIBFLAGS="-path ../src:../ -lib zexy -path ../abs/"

function list_tests() {
#  find . -mindepth 2  -name "*.pd" | sed 's|\.pd$|;|' 
 ls -1 */*.pd | sed 's|\.pd$|;|'
}

function debug() {
 :
# echo $@
}


function evaluate_tests() {
 local logfile
 local testfile
 local numtests

 testfile=$1
 logfile=$2

 debug "now evaluating results in ${logfile} (${testfile}"

 numtests=$(grep -c . ${testfile})
 debug "number of tests = ${numtests}"
 echo "regression-test: ${numtests} tests total" >>  ${logfile}
 debug "show results"
 cat ${logfile} | egrep "^regression-test: " | sed -e 's/^regression-test: //'
}


function run_nogui() {
 debug "running test without gui"
 ${PD} ${LIBFLAGS} -nogui runtests_nogui.pd > ${RUNTESTS_LOG} 2>&1 
 debug "testing done"
 evaluate_tests ${RUNTESTS_TXT} ${RUNTESTS_LOG}
 debug "testing finished"
}

function run_withgui() {
 debug "running test with gui"
 ${PD} ${LIBFLAGS} -stderr runtests.pd > ${RUNTESTS_LOG} 2>&1
 debug "testing completed, no evaluation will be done; see ${RUNTESTS_LOG} for results"
}

list_tests > ${RUNTESTS_TXT}

if test "x$1" = "x-gui"; then
 run_withgui
else
 run_nogui
fi


