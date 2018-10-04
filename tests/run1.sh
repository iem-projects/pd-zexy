#!/bin/sh

PD=${PD:=pd}
LIBDIR=${LIBDIR:=../src/.libs/}
SRCDIR=${SRCDIR:=../}
TESTDIR=${TESTDIR:=.}

SCRIPTDIR=$(realpath ${0%/*})


PD=$(which ${PD})
TEST=$1

# assume that the first component of the test-path is the object to be tested
# at least this object must not fail to create
TESTOBJ=$(realpath $TEST)
TESTOBJ=${TESTOBJ#${SCRIPTDIR}}
TESTOBJ=${TESTOBJ#/}
TESTOBJ=${TESTOBJ%%/*}

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
	| tee "${TMPFILE}"


egrep "^regression-test: ${TEST%.pd}: OK" "${TMPFILE}" >/dev/null
SUCCESS=$?


if egrep -B1 "^error: \.\.\. couldn't create" "${TMPFILE}" \
	| egrep -v "^error: \.\.\. couldn't create" \
	| awk '{print $1}' \
        | egrep "^${TESTOBJ}$" \
                >/dev/null
then
    echo "COULDN'T CREATE $TESTOBJ" 1>&2
    SUCCESS=1
fi

rm "${TMPFILE}"

exit ${SUCCESS}
