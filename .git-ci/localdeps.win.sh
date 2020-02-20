#!/bin/sh

## puts dependencies besides the binary

## usage: $0 <binary> [<binary2>...]

CP="cp -v"
#CP=cp

error() {
   echo "$@" 1>&2
}

NTLDD=$(which ntldd 2>/dev/null)

if [ "x${NTLDD}" = "x" ]; then
  error "no 'ntldd' binary found"
  exit 0
fi


list_deps() {
  ${NTLDD} -R "$1" \
	| grep -i mingw \
	| awk '{print $3}' \
	| grep -i mingw \
	| sed -e 's|\\|/|g'
}

file2arch() {
  if file "$1" | grep -w "PE32+" >/dev/null; then
    echo "x64"
    return
  fi
  if file "$1" | grep -w "PE32" >/dev/null; then
    echo "x32"
    return
  fi
}

install_deps () {
local outdir=$2
local idepfile
local odepfile
local archext
local dep
if [ "x${outdir}" = "x" ]; then
  outdir=${1%/*}
fi
if [ "x${outdir}" = "x" ]; then
  outdir=.
fi

list_deps "$1" | while read dep; do
  idepfile=$(basename "${dep}")
  odepfile=${idepfile}
  archext=$(file2arch "${dep}")
  if [ "x${archext}" != "x" ]; then
    odepfile=$(echo ${idepfile} | sed -e "s|\.dll|.${archext}|")
  fi
  if [ "x${idepfile}" = "x${odepfile}" ]; then
	archext=""
  fi
  if [ -e "${outdir}/${odepfile}" ]; then
    error "skipping already localized depdendency ${dep}"
  else
    ${CP} "${dep}" "${outdir}/${odepfile}"
  fi


  if [ "x${archext}" != "x" ]; then
    sed -b -e "s|${idepfile}|${odepfile}|g" -i "${outdir}/${odepfile}" "${outdir}"/*."${archext}" "$1"
  fi
done
}


for f in "$@"; do
   if [ -e "${f}" ]; then
       install_deps "${f}"
   fi
done
