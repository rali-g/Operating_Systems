#! /bin/bash

inode=$(find /home/velin -type f -exec stat -c %Y" "%i {} \; 2>/dev/null | sort -rn | head -1 | cut -d' ' -f2)

min=100000
min_file=$(mktemp)

while read -r line; do
    if [[ $(stat -c %i $line) -eq $inode ]]; then
        count=$(echo $line | tr -cd '/' | wc -c)
        if [[ $count -lt $min ]]; then
            min=$count
            echo $line > $min_file
        fi
    fi
done < <(find /home/velin -type f 2>/dev/null)

cat "$min_file"
rm "$min_file"
