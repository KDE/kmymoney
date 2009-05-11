#!bin/sh
# invoke the extractrc script on all .ui, .rc, and .kcfg files in the sources
# the results are stored in a pseudo .cpp file to be picked up by xgettext.
$EXTRACTRC `find . -name \*.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
# if your application contains tips-of-the-day, call preparetips as well.
cd kmymoney2/ && $PREPARETIPS > tips.cpp
cd ..
# call xgettext on all source files. If your sources have other filename
# extensions besides .cc, .cpp, and .h, just add them in the find call.
$XGETTEXT `find . -name \*.cc -o -name \*.cpp -o -name \*.h` -o $podir/kmymoney2.pot

# Remove these two generated files again
rm kmymoney2/tips.cpp rc.cpp
