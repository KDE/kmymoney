#!bin/sh
# invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources
# the results are stored in a pseudo .cpp file to be picked up by xgettext.
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
# call xgettext on all source files. If your sources have other filename
# extensions besides .cpp, and .h, just add them in the find call.
$XGETTEXT `find . -name \*.cpp -o -name \*.h | grep -v '/tests/'` -o $podir/kmymoney.pot

# Remove this generated file again
rm rc.cpp
