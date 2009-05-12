#!/bin/sh
#
# This is a wrapper that executes stats.pl for all .po files in the
# given directory
#
# This wrapper is executed by 'make message-stats'.
# It is a custom target as defined in po/CMakeLists.txt
#
# The script requires to find the kmymoney2.pot file in the same
# directory in order to check if the po file is based on the current
# pot file.
#
#***************************************************************************
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#***************************************************************************

# Change to the directory provided on the command line.
cd $*

echo "<?xml version='1.0' standalone='yes'?>"
echo "<!DOCTYPE issuelist>"
echo "<translist>"
for i in *.po; do
  perl ./stats.pl ./$i;
done;
echo "</translist>"
echo "<?xml version='1.0'?>"
