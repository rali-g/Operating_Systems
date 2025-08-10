#! /bin/bash

type=$(sort -t';' -k3 /srv/sample-data/01-shell/2019-SE-01/input.data | head -8 | tail -1 | cut -d';' -f2)
sort -t';' -k3 /srv/sample-data/01-shell/2019-SE-01/input.data | head -8 | grep -E $type | head -1 | cut -d';' -f1,4 | sed 's/;/ /'

