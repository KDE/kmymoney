#!/bin/sh

# Usage: ./copy_from_breeze.sh breeze-icon\
# where breeze-icon\ is a path to a root folder of a cloned breeze-icon git repository
# (must contain "icons" and "icons-dark" subfolders).

for i in $(sed -E -n -e "s/^.*{IconSet\:\:(Breeze|Common), QStringLiteral\(\"(.*)\"\)}.*/\2/p" icons.cpp); do
  for f in $(find $1 -name "$i.*"); do
     r=${f/#$1\/icons/breeze}
     d=${r%/*}
     [ -d "$d" ] || mkdir -p "$d"
     echo "copying $f to $r"
     cp "$f" "$r"
   done  
done
