#!/bin/sh

PD=${PD:=pd}
LIBDIR=${LIBDIR:=../src/.libs/}
SRCDIR=${SRCDIR:=../}
SCRIPTDIR=${0%/*}
TESTDIR=${TESTDIR:=${SCRIPTDIR}}
SCRIPTDIR=$(realpath ${SCRIPTDIR})


usage() {
cat 1>&2 <<EOF
usage: $0 [options] <testpatch>
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

reportsuccess() {
    case "$1" in
        0)
            if [ "x${shouldfail}" = "x1" ]; then
                echo "${grn}XFAIL${std}: $2"
            else
                echo "${grn}PASS${std} $2"
            fi
            ;;
        1)
            if [ "x${shouldfail}" = "x1" ]; then
                echo "${red}XPASS${std}: $2"
            else
                echo "${red}FAIL${std}: $2"
            fi
            ;;
        77)
            echo "${blu}SKIP${std}: $2"
            ;;
        99)
            echo "${mgn}HARDFAIL${std}: $2"
            ;;
        *)
            echo "${lrd}FAIL$1${std}: $2"
            ;;
    esac
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

PD=$(which ${PD})
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

if [ "x${shouldfail}" = "xauto" ]; then
    if [ "x${TEST##*/fail}" != "x${TEST}" ]; then
        shouldfail=1
    else
        shouldfail=0
    fi
fi

if [ "x${PD}" = "x" ]; then
 exit 77
fi

LIBFLAGS="-path ${LIBDIR}:${SRCDIR}/abs:. -lib ${LIBDIR}/zexy"
TMPFILE=$(mktemp)

${VALGRIND} ${PD} \
	-noprefs -nostdpath \
	-oss -nosound -nrt \
	-nogui -batch -verbose \
	${LIBFLAGS} \
	-open ${TESTDIR}/run1.pd \
	-send "test ${TEST%.pd}" 2>&1 \
	| tee "${TMPFILE}" \
	| catverbose 3


egrep "^regression-test: ${TEST%.pd}: OK" "${TMPFILE}" >/dev/null
SUCCESS=$?


if egrep -B1 "^error: \.\.\. couldn't create" "${TMPFILE}" \
	| egrep -v "^error: \.\.\. couldn't create" \
	| awk '{print $1}' \
        | egrep "^${TESTOBJ}$" \
                >/dev/null
then
    echo "COULDN'T CREATE $TESTOBJ" | catverbose 2 1>&2
    SUCCESS=1
fi

if [  ${shouldfail} -ge 1 ]; then
  case "${SUCCESS}" in
      0)
          SUCCESS=1
      ;;
      77|99)
          :
      ;;
      *)
          SUCCESS=0
      ;;
  esac
fi

if test "x${SUCCESS}" != "x0" && test ${showlog} -ge 1 && test 3 -gt $verbosity; then
    cat "${TMPFILE}"
fi
rm "${TMPFILE}"

if [ 1 -le $verbosity ]; then
    reportsuccess $SUCCESS $TEST
fi


exit ${SUCCESS}
