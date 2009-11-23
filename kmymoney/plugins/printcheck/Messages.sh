#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp
$XGETTEXT $XGETTEXT_FLAGS *.cpp -o $podir/printcheck.pot
rm -f rc.cpp

