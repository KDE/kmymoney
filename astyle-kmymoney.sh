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
        PAD="pad=oper"
        UNPAD="unpad=paren"
        ONELINE="one-line=keep-statements"
        ;;
      *)
        PAD="pad-oper"
        UNPAD="unpad-paren"
        ONELINE="keep-one-line-statements"
        ;;
    esac

    # run astyle with options on the set of files
    find kmymoney libkgpgfile -type f  \( -name \*.c -or -name \*.cpp -or -name \*.h \) -exec $path/astyle --indent=spaces=2 --brackets=linux \
      --indent-labels --${PAD} --${UNPAD} \
      --${ONELINE} --convert-tabs \
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
