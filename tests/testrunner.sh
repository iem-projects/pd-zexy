#!/bin/sh

PD=${PD:=pd}
LIBDIR=${LIBDIR:=../src/.libs/}
SRCDIR=${SRCDIR:=../}
SCRIPTDIR=${0%/*}
TESTDIR=${TESTDIR:=${SCRIPTDIR}}
SCRIPTDIR=$(realpath ${SCRIPTDIR})

count_all=0
count_pass=0
count_fail=0
count_skip=0
count_xfail=0
count_xpass=0
count_error=0


usage() {
cat 1>&2 <<EOF
usage: $0 [options] <testpatch> [ <testpatch>... ]
	<testpatch>: Pd-patch to be tested
    options
	-v	raise verbosity
	-q	lower verbosity
        -l      show log on failure
        -x      expect test to fail
        -X      test-files starting with fail are expected to fail
EOF
exit
}
catverbose() {
  if [ $1 -le $verbosity ]; then
     cat
  else
     cat >/dev/null
  fi
}
if test "X$TERM" != Xdumb && test -t 1 2>/dev/null; then
    red='^[[0;31m'
    #    grn='^[[0;32m'
    lgn='^[[1;32m'
    blu='^[[1;34m'
    mgn='^[[0;35m'
    brg='^[[1m'
    #std='^[[m'

    red='\e[0;31m'
    lrd='\e[0;91m'
    grn="\e[32m"
    lgn='\e[1;32m'
    blu='\e[1;34m'
    mgn='\e[0;35m'
    brg='\e[1m'

    std="\e[0m"
else
    red=
    grn=
    lgn=
    blu=
    mgn=
    brg=
    std=
fi

report_success() {
if [ 1 -le $verbosity ]; then
    case "$1" in
        0)
            if [ "x${wantfail}" = "x1" ]; then
                echo "${lgn}XFAIL${std}: $2"
            else
                echo "${grn}PASS${std}: $2"
            fi
            ;;
        1)
            if [ "x${wantfail}" = "x1" ]; then
                echo "${lrd}XPASS${std}: $2"
            else
                echo "${red}FAIL${std}: $2"
            fi
            ;;
        77)
            echo "${blu}SKIP${std}: $2"
            ;;
        99)
            echo "${mgn}ERROR${std}: $2"
            ;;
        *)
            echo "${red}FAIL$1${std}: $2"
            ;;
    esac
fi
    count_all=$((count_all+1))
    case "$1" in
        0)
            if [ "x${wantfail}" = "x1" ]; then
                count_xfail=$((count_xfail+1))
            else
                count_pass=$((count_pass+1))
            fi
            ;;
        77)
            count_skip=$((count_skip+1))
            ;;
        99)
            count_error=$((count_error+1))
            ;;
        *)
            if [ "x${wantfail}" = "x1" ]; then
                count_xpass=$((count_xpass+1))
            else
                count_fail=$((count_fail+1))
            fi
            ;;
    esac
}
summary_success() {
    if [ 0 -le $verbosity ]; then
        echo ""
        echo "${grn}============================================================================${std}"
        echo "${grn}Testsuite summary"
        echo "${grn}============================================================================${std}"
        echo "${brg}TOTAL${std}\t${count_all}"
        echo "${grn}PASS${std}\t${count_pass}"
        echo "${red}FAIL${std}\t${count_fail}"
        echo "${blu}SKIP${std}\t${count_skip}"
        echo "${lgn}XFAIL${std}\t${count_xfail}"
        echo "${lrd}XPASS${std}\t${count_xpass}"
        echo "${mgn}ERROR${std}\t${count_error}"
        echo "${grn}============================================================================${std}"
    fi
}

should_fail() {
    if [ "x$1" = "xauto" ]; then
        if [ "x${2#fail}" != "x${2}" ]; then
            echo 1
        else
            echo 0
        fi
    else
        echo $1
    fi
}

check_success() {
    if [  ${wantfail} -ge 1 ]; then
        case "$1" in
            0)
                echo 1
                ;;
            77|99)
                echo $1
                ;;
            *)
                echo 0
                ;;
        esac
    else
        echo $1
    fi
}

verbosity=1
showlog=0
shouldfail=0

while getopts "vqlxXh?" opt; do
    case $opt in
        v)
            verbosity=$((verbosity+1))
            ;;
        q)
            verbosity=$((verbosity-1))
            ;;
        l)
            showlog=1
            ;;
        x)
            shouldfail=1
            ;;
        X)
            shouldfail=auto
            ;;
        :|h|\?)
            usage
            ;;
    esac
done
shift $(($OPTIND - 1))
if [ $# -lt 1 ]; then
    usage
fi


wantfail=${shouldfail}
PD=$(which ${PD})
if [ "x${PD}" = "x" ]; then
 echo "couldn't find Pd (Hint: use the PD environment variable)" 1>&2
 exit 77
fi
LIBFLAGS="-path ${LIBDIR}:${SRCDIR}/abs:. -lib ${LIBDIR}/zexy"

do_runtest() {

TEST=$1
if [  ! -e "${TEST}" ]; then
    usage
fi

# assume that the first component of the test-path is the object to be tested
# at least this object must not fail to create
TESTOBJ=$(realpath $TEST)
TESTOBJ=${TESTOBJ#${SCRIPTDIR}}
TESTOBJ=${TESTOBJ#/}
TESTOBJ=${TESTOBJ%%/*}

TMPFILE=$(mktemp)

SUCCESS=0
${VALGRIND} ${PD} \
	    -noprefs -nostdpath \
	    -oss -nosound -nrt \
	    -nogui -batch -verbose \
	    ${LIBFLAGS} \
	    -open ${TESTDIR}/run1.pd \
	    -send "test ${TEST%.pd}" \
            >"${TMPFILE}" 2>&1
SUCCESS=$?
cat "${TMPFILE}" | catverbose 3

if [ $SUCCESS -eq 0 ]; then
if egrep "^regression-test" "${TMPFILE}" >/dev/null; then
    egrep "^regression-test: ${TEST%.pd}: OK" "${TMPFILE}" >/dev/null
    SUCCESS=$?
else
    SUCCESS=77
fi
fi


if egrep -B1 "^error: \.\.\. couldn't create" "${TMPFILE}" \
	| egrep -v "^error: \.\.\. couldn't create" \
	| awk '{print $1}' \
        | egrep "^${TESTOBJ}$" \
                >/dev/null
then
    echo "COULDN'T CREATE $TESTOBJ" | catverbose 2 1>&2
    SUCCESS=1
fi

wantfail=$(should_fail $shouldfail ${TEST##*/})
SUCCESS=$(check_success $SUCCESS)

if test ${SUCCESS} -ge 1 && test ${showlog} -ge 1 && test $verbosity -le 3; then
    cat "${TMPFILE}"
fi
rm "${TMPFILE}"
report_success $SUCCESS $TEST
}


for t in "$@"; do
    do_runtest "$t"
done

if [  ${count_all} -gt 1 ]; then
    summary_success
fi

exit ${SUCCESS}
