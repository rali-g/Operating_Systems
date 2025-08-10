#! /bin/bash

awk -F: '/SI/ {printf $5":"$6"\n"}' /srv/sample-data/01-shell/2018-SE-03/example-passwd | sed -E 's/,(.*):/:/' | sort -t '/' -k4

