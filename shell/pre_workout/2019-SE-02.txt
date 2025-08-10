#! /bin/bash

tmp=$(mktemp)
res=$(mktemp)

cut -d':' -f1,5,6 /etc/passwd | grep '/home/SI/' | sed -E 's/s([0-9]+:)/\1/' | sed -E 's/(,.*):/:/' > $tmp

while read -r line; do
    dir=$(echo $line | cut -d':' -f3)
    if [[ -d $dir ]]; then
        curr=$(stat -c %Z "$dir")
        if [[ $curr -ge 1551168000 && $curr -le 1551176100 ]]; then
            echo $line | cut -d':' -f1,2 | tr ':' ' '
        fi
    fi
done < $tmp

rm $tmp
rm $res
