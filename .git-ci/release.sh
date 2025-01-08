#!/bin/sh
export LANG=C

: "${GIT:=git}"

changelog=ChangeLog
makefile=Makefile
if [ -z "${pkg}" ]; then
    test -e "${makefile}" && pkg=$(grep -E "^lib.name *= *" "${makefile}" | tail -1 | awk '{print $NF}')
fi
meta="${pkg}-meta.pd"

timestamp=$(date +%s)

usage() {

	cat <<EOF
usage: $0 <version>

   create and tag a release

   - the repository must be unmodified
   - <version> must not yet be tagged
   - embeds version into '${meta}'
   - embeds version into '${makefile}'
   - Adds a new entry to the '${changelog}'
   - creates a new release commit
   - tags the release commit
EOF
}

error() {
	echo "$@" 1>&2
}


makecl() {
    cat <<EOF
${pkg} ($2) RELEASED; urgency=medium

EOF
    git log "${1}"..HEAD --reverse --pretty=format:'  * %s'

    cat <<EOF


 -- $(git config --get user.name) <$(git config --get user.email)> $(date --date="@${timestamp}" +"%a, %d %b %Y %T %z")

EOF
}

if [ -z "${pkg}" ]; then
    error "Could not determine name of the package"
    exit 1
fi

version="${1#v}"
if [ -z "${version}" ]; then
	usage
	exit 1
fi

lastversion="$(git tag --sort=-creatordate | head -1)"
if [  -z "${lastversion}" ]; then
    error "unable to determine previous version"
    exit 1
fi

error "Release ${pkg}-${version}"

# check if repository is pristine
if git status --porcelain -uno | grep . >/dev/null; then
    error "repository is modified!"
    exit 1
fi


# check whether this version has already been tagged
git tag "v${version}" || exit 1
git tag -d "v${version}" >/dev/null

# update the changelog
if [ -e "${changelog}" ]; then
    tempcl=$(mktemp "${changelog}.XXXXX")
    { makecl "${lastversion}" "${version}"; cat "${changelog}"; } >"${tempcl}"
    # exit 'vi' with :cq to get a non-0 exit-code
    vi "${tempcl}" || exit $?
    cat "${tempcl}" > "${changelog}"
    rm -f "${tempcl}"
    "${GIT}" add "${changelog}"
fi

# update meta.pd
if [ -e "${meta}" ]; then
    sed -e "s|^\(#X text [0-9-]* [0-9-]* VERSION\) .*;|\1 ${version};|" -i "${meta}"
    "${GIT}" add "${meta}"
fi

# update Makefile
if [ -e "${makefile}" ]; then
    sed -e "s|\(-DVERSION\)='\"[^']*\"'|\1='\"${version}\"'|" -i "${makefile}"
    "${GIT}" add "${makefile}"
fi

# commit and tag
"${GIT}" commit -m "Release ${pkg}-${version}"
"${GIT}" tag -s -m "Release ${pkg}-${version}" "v${version}"
