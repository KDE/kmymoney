#!/bin/bash
#
# Copyright 2018       Thomas Baumgart <tbaumgart@kde.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# the subdirectories containing the files
PNG_DIR=../kmymoney/pics/l10n
SVG_DIR=../kmymoney/pics/svg

# check for inkscape being available
export LANG=C
INKSCAPE=`which inkscape 2>/dev/null`
if [ -z "$INKSCAPE" ]; then
  echo "inkscape is not found in search path."
  exit 1
fi

# make sure we are started in the right directory
if [ -e "conv_splash.sh" -a -d "$PNG_DIR" -a -d "$SVG_DIR" ]; then
  FILES=`find $PNG_DIR -name startlogo.png`
  for DST_FILE in $FILES; do
    DST_DIR=`dirname $DST_FILE`
    LANG=`basename $DST_DIR`
    SRC_FILE=$SVG_DIR/startlogo_$LANG.svg
    if [ -e $SRC_FILE ]; then
      echo Converting $SRC_FILE to $DST_FILE
      $INKSCAPE -a 32:740:432:1040 -e "$DST_FILE" "$SRC_FILE" 2>/dev/null
    fi
  done
else
  echo "`basename $0` converts the svg splash screens into png"
  echo
  echo "Please make sure to start `basename $0` in the tools subdir of the"
  echo "KMyMoney source tree and that the relative directories"
  echo "'../kmymoney/pics/l10n' and '../kmymoney/pics/svg' exist"
  echo
  echo "Only existing pngs are converted. An empty file is enought to start with."
  exit 1
fi
