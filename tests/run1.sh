#!/bin/sh

PD=${PD:=pd}
LIBDIR=${LIBDIR:=../src/.libs/}
SRCDIR=${SRCDIR:=../}
TESTDIR=${TESTDIR:=.}

PD=$(which ${PD})
TEST=$1

if [ "x${PD}" = "x" ]; then
 exit 77
fi

LIBFLAGS="-path ${LIBDIR} -lib zexy -path ${SRCDIR}/abs:."
TMPFILE=$(mktemp)

${PD} \
	-noprefs -oss -nosound \
	-nogui -batch \
	${LIBFLAGS} \
	-open ${TESTDIR}/run1.pd \
	-send "test ${TEST%.pd}" 2>&1 \
	| tee "${TMPFILE}"


egrep "^regression-test: ${TEST%.pd}: OK" "${TMPFILE}" >/dev/null
SUCCESS=$?

rm "${TMPFILE}"

exit ${SUCCESS}
