#!/bin/bash

# apply kmymoney coding style (kdelibs coding style with 2 spaces indentation) to all c, cpp and header files 
# in and below the current directory excluding libkdchart
# 
# the coding style is defined in http://techbase.kde.org/Policies/Kdelibs_Coding_Style 
# 
# requirements: installed astyle 

IFS=:
for path in $PATH; do
  if test -x $path/astyle; then
    # determine version of AStyle
    VER=`$path/astyle --version 2>&1 | cut -d' ' -f4`

    # there was an option change between 1.22 and 1.23
    # we default to the newer version here as this is the future
    case $VER in
      1.2[1-2])
        OPT="--pad=oper --unpad=paren"
        ;;
      *)
        OPT="--pad-oper --unpad-paren"
        ;;
    esac

    $path/astyle --indent=spaces=2 --brackets=linux \
      --indent-labels ${OPT} \
      --one-line=keep-statements --convert-tabs \
      --indent-preprocessor \
      `find kmymoney libkgpgfile -type f  \( -name \*.c -or -name \*.cpp -or -name \*.h \)  `
    exit $?
  fi
done
