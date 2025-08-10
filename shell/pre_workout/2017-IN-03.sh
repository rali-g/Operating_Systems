#! /bin/bash

awk -F: '{print $1 "*" $5 "*" $6}' /srv/sample-data/01-shell/2017-IN-03/example-passwd \
| sed 's/[, ]/*/g' | awk -F* '$3 ~ /a$/ && /Inf/ {print $1}' | cut -c 3-4 | sort -nr | uniq -c

