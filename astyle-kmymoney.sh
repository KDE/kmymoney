#!/bin/bash

# apply kmymoney coding style (kdelibs coding style with 2 spaces indentation) to all c, cpp and header files 
# in and below the current directory excluding libkdchart
# 
# the coding style is defined in http://techbase.kde.org/Policies/Kdelibs_Coding_Style 
# 
# requirements: installed astyle 

astyle --indent=spaces=2 --brackets=linux \
      --indent-labels --pad=oper --unpad=paren \
      --one-line=keep-statements --convert-tabs \
      --indent-preprocessor \
      `find kmymoney libkgpgfile -type f -name '*.c'` `find kmymoney libkgpgfile -type f -name '*.cpp'` `find kmymoney libkgpgfile -type f -name '*.h'`
