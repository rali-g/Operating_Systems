#! /bin/bash

grep -E '[2468]+' /srv/sample-data/01-shell/2016-SE-01/philip-j-fry.txt \
| grep -v '[a-w]' | wc -l

