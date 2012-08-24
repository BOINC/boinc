#!/usr/bin/env bash

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
    BRANCH=`git branch | sed -n 's/^\* *//p'`
    remote=`git config --get branch.$BRANCH.remote`
    URL=`git config --get remote.$remote.url`
    DATE=`git log -n1 --pretty="format:%ct"`
elif [ -d .svn ]; then
    CMD="svn info"
else
    CMD=""
fi

if [ "x$GIT_LOG" != "x" ]; then
    echo "#define SVN_VERSION \"$GIT_LOG [$URL] ($HOST:$PWD [$BRANCH])\"" >> $TMPFILE
    echo "$GIT_LOG" | sed 's/^\(........\).*/#define GIT_REVISION 0x\1/' >> $TMPFILE
    echo "#define GIT_DATE $DATE" >> $TMPFILE
    test "x$URL" = "x" ||
        echo "$URL" |
            sed 's/.git$//;s%.*://%%;s/[^/]*@//;s/[^a-zA-Z0-9]/_/g;s/__*/_/g;
                 y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/;
                 s/^/#define REPOSITORY_/;s/$/ 1/' >> $TMPFILE
elif [ "x$CMD" != "x" ]; then
    LANG=C 
    URL=`$CMD | awk '
                /^URL/ { url = $2; };
                /^Rev/ { rev = $2; };
                END { print "#define SVN_VERSION \"Repository: " url \
                            " Revision: " rev "\"" >> "'"$TMPFILE"'";
                      print "#define SVN_REPOSITORY \"" url "\"" >> "'"$TMPFILE"'";
                      print "#define SVN_REVISION " rev >> "'"$TMPFILE"'";
                      print url };'`
    echo $URL | 
        sed 's%.*://%%;s/[^/]*@//;s/[^a-zA-Z0-9]/_/g;s/__*/_/g;
             y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/;
             s/^/#define REPOSITORY_/;s/$/ 1/' >> $TMPFILE
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
