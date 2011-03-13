#!/bin/bash

# apply kmymoney coding style (kdelibs coding style with 2 spaces indentation) to all c, cpp and header files 
# in and below the current directory excluding libkdchart
# 
# the coding style is defined in http://techbase.kde.org/Policies/Kdelibs_Coding_Style 
# 
# requirements: installed astyle 

ASTYLE_VERSION=1.23

IFS=:
for path in $PATH; do
  if test -x $path/astyle; then
    # determine version of AStyle
    VER=`$path/astyle --version 2>&1 | cut -d' ' -f4`

    # check if the needed astyle version is installed
    if [ "$VER" != "$ASTYLE_VERSION" ]; then
      echo "You have astyle version $VER installed. The required astyle version is $ASTYLE_VERSION, please install the required version to use this script."
      exit
    fi

    # run astyle with options on the set of files
    find kmymoney libkgpgfile -type f  \( -name \*.c -or -name \*.cpp -or -name \*.h \) -exec $path/astyle --indent=spaces=2 --brackets=linux \
      --indent-labels --pad-oper --unpad-paren \
      --keep-one-line-statements --convert-tabs \
      --indent-switches --indent-cases \
      --indent-preprocessor {} \;

    # process the same set of files to replace "foreach(" with "foreach ("
    find kmymoney libkgpgfile -type f  \( -name \*.c -or -name \*.cpp -or -name \*.h \) | while read a; do
      sed -e "s/foreach(/foreach (/" $a > $a.tmp
      diff $a $a.tmp > /dev/null
      if test $? -ne 0; then
        echo "formatted  "$a
        mv $a.tmp $a
      else
        echo "unchanged* "$a
        rm $a.tmp
      fi
    done

    exit $?
  fi
done
