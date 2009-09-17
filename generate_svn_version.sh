#!/usr/bin/env sh

echo "#ifndef SVN_VERSION_H" > svn_version.h
echo "#define SVN_VERSION_H" >> svn_version.h
echo "" >> svn_version.h

if [ -d .git ]; then
    CMD="git svn info"
elif [ -d .svn ]; then
    CMD="svn info"
else
    CMD=""
fi

if [ "x$CMD" != "x" ]; then
    $CMD | awk '/^URL/ { url = $2; }; \
                /^Rev/ { rev = $2; }; \
                END { print "#define SVN_VERSION \"Repository: " url \
                            " Revision: " rev "\""; };' \
               >> svn_version.h
else
    echo "#include \"version.h\"" >> svn_version.h
    echo "#define SVN_VERSION BOINC_VERSION_STRING" >> svn_version.h
fi

echo "" >> svn_version.h
echo "#endif" >> svn_version.h
