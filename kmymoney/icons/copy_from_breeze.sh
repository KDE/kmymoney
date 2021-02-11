#!/bin/bash

# Usage: ./copy_from_breeze.sh breeze-icons\
# where breeze-icon\ is a path to a root folder of a cloned breeze-icon git repository
# (must contain "icons" and "icons-dark" subfolders).

function show_help()
{
    echo "Syntax: $0 <breeze-icon-dir>"
    echo
    echo "  <breeze-icon-dir> must contain an icons and an icons-dark subdirectory"
}

# make sure, src_dir does not end in '/' and is present
src_dir=${1%/}
if [ -z "$src_dir" ]; then
    echo "Missing breeze-icon-dir argument"
    show_help
    exit 1
fi

# check for src_dir being a directory
if [ ! -d $src_dir ]; then
    echo "$src_dir is not a directory"
    show_help
    exit 1
fi

# check for src_dir/icons being a directory
if [ ! -d $src_dir/icons ]; then
    echo "$src_dir does not have an 'icons' subdirectory"
    show_help
    exit 1
fi

# check for src_dir/icons-dark being a directory
if [ ! -d $src_dir/icons-dark ]; then
    echo "$src_dir does not have an 'icons-dark' subdirectory"
    show_help
    exit 1
fi

scriptdir=$(dirname $(readlink -e $0))
[ -n "$scriptdir" ] || exit 1
cd $scriptdir

if [ ! -r icons.cpp ]; then
    echo "icons.cpp source file not found in $scriptdir"
    exit 1
fi

used_in_code=$(sed -E -n -e "s/^.*\{Icon\:\:[[:alnum:]]*, QStringLiteral\(\"(.*)\"\)\},/\1/p" icons.cpp)
additional=" index application-x-kmymoney edit-undo edit-redo document-print"
all_icons=$used_in_code$additional

# check if we have the dos2unix tool available
DOS2UNIX=$(which dos2unix 2>/dev/null)

rm -rf breeze breeze-dark

for i in $all_icons; do
    for src in $(find $src_dir -name "$i.*"); do
        dest=${src/#$src_dir\/icons/breeze}
        dest_dir=${dest%/*}
        [ -d "$dest_dir" ] || mkdir -p "$dest_dir"
        echo "copying $src to $dest"
        cp "$src" "$dest"
        [ -z "$DOS2UNIX" ] || $DOS2UNIX $dest
    done
done



# remove the generated file to force regeneration
rm -f icons.qrc
