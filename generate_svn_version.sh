#!/usr/bin/env sh

HEADER="svn_version.h"
TMPFILE="$HEADER.tmp"

echo "#ifndef SVN_VERSION_H" > $TMPFILE
echo "#define SVN_VERSION_H" >> $TMPFILE
echo "" >> $TMPFILE

if [ -d .git/svn ]; then
    CMD="git svn info"
elif [ -d .git ]; then
    GIT_LOG=`git log -n1 --pretty="format:%H"`
    HOST=`hostname`
elif [ -d .svn ]; then
    CMD="svn info"
else
    CMD=""
fi

if [ "x$GIT_LOG" != "x" ]; then
    echo "#define SVN_VERSION \"$GIT_LOG ($HOST:$PWD)\"" >> $TMPFILE
elif [ "x$CMD" != "x" ]; then
    LANG=C $CMD | awk '/^URL/ { url = $2; }; \
                /^Rev/ { rev = $2; }; \
                END { print "#define SVN_VERSION \"Repository: " url \
                            " Revision: " rev "\"";
                      print "#define SVN_REPOSITORY \"" url "\""; \
                      print "#define SVN_REVISION " rev; };' \
               >> $TMPFILE
else
    echo "#include \"version.h\"" >> $TMPFILE
    echo "#define SVN_VERSION BOINC_VERSION_STRING" >> $TMPFILE
fi

echo "" >> $TMPFILE
echo "#endif" >> $TMPFILE

if cmp "$HEADER" "$TMPFILE" >/dev/null 2>&1; then
	rm -f "$TMPFILE"
else
	mv "$TMPFILE" "$HEADER"
fi
