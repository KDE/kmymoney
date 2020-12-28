#!/bin/sh

# Usage: ./copy_from_breeze.sh breeze-icon\
# where breeze-icon\ is a path to a root folder of a cloned breeze-icon git repository
# (must contain "icons" and "icons-dark" subfolders).

used_in_code=$(sed -E -n -e "s/^.*{Icon\:\:[[:alnum:]]*, QStringLiteral\(\"(.*)\"\)},/\1/p" icons.cpp)
additional=" index application-x-kmymoney edit-undo edit-redo document-print"
all_icons=$used_in_code$additional

rm -r breeze breeze-dark

for i in $all_icons; do
  for f in $(find $1 -name "$i.*"); do
     r=${f/#$1\/icons/breeze}
     d=${r%/*}
     [ -d "$d" ] || mkdir -p "$d"
     echo "copying $f to $r"
     cp "$f" "$r"
   done
done

rm icons.qrc