#! /bin/bash

most_success=$(tail +2 /srv/sample-data/01-shell/2020-SE-02/spacex.txt | awk -F'|' '/Success/ {sum[$2]++} END{for(var in sum) {print sum[var], var}}' \
| sort -nr | head -1 | cut -d' ' -f2)

while read line; do
    if echo $line | grep -q "$most_success" && echo $line | grep -q "Success"; then
        echo $(echo $line | cut -d'|' -f4)
        exit 0;
    fi
done< <(tail +2 /srv/sample-data/01-shell/2020-SE-02/spacex.txt | sort -nr)
