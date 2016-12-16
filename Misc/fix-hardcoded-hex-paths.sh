#!/bin/bash

### Description
#
# This script replaces broken hardcoded strings in binary files (e.g. maps)
# with another string given by the user.
#
# The input and ouput strings are given by the file passed as first argument,
# which should be of the form:
#
# old string<TAB>new string
# other old string<TAB>other new string
# etc.
#
# The strings are fixed by binary replacements of their hex code. The hex
# representation of a string starts with its length on four bytes, but in
# this script we currently only fix the fourth byte (the others are typically
# null), which means we support strings up to 255 chars.
#
# The binary files to fix are passed as second and optionally more arguments.
#

### Usage
#
# ./Misc/fix-hardcoded-hex-paths.sh [file describing fixes] [maps to fix]
#
# e.g.:
#
# ./Misc/fix-hardcoded-hex-paths.sh Misc/list-hardcoded-paths-fixes.txt Data/Maps/jendraz*
#

### Functions

ascii2hex () {
    local len=$(echo -n $1 | wc -c)
    if [ $len -gt 255 ]; then echo "This poor script can't handle such long strings. Blame Akien."; exit 1; fi
    len='\x'$(printf '%02x\n' $len)
    local hexstring=$(echo -n $1 | xxd -ps | sed 's/[[:xdigit:]]\{2\}/\\x&/g')
    # Seems xxd will put some separators on long strings
    return=$len$(echo $hexstring | sed 's/ //g')
    return
}

hex2ascii () {
    local string=$(echo -n $1 | cut -c 4-)
    return=$(echo $string | sed 's/\x//g' | xxd -r -ps)
    return
}

### Script

in_file="$1"
target_files="${@:2}"

if [ ! -e $in_file ]; then echo "You must pass a valid filename as argument, got $in_file."; exit 1; fi

if [ -z "${target_files}" ]; then echo "You muss pass a list of files as second argument."; exit 1; fi

echo -e "Replacing hardcoded strings as listed in $in_file in the following files:\n\n$target_files\n"

while IFS='' read -r line || [[ -n "$line" ]]; do
    input=$(echo "$line" | cut -f1)
    output=$(echo "$line" | cut -f2)
    echo "Replacing '${input}' by '${output}'."

    ascii2hex "$input"
    in_hex=$return
    hex2ascii "$in_hex"
    if [ "$return" != "$input" ]; then echo "Something went wrong in the conversion of $input back and forth to hex."; fi

    ascii2hex "$output"
    out_hex=$return
    hex2ascii "$out_hex"
    if [ "$return" != "$output" ]; then echo "Something went wrong in the conversion of $output back and forth to hex."; fi

    for target_file in $target_files; do
        sed -i 's/'${in_hex}'/'${out_hex}'/g' "${target_file}"
    done
done < "$in_file"
