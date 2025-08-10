#! /bin/bash

find -type f -size 0 -exec echo rm -fr {} \;
find ~ -type f -user "$(whoami)" -printf '%p %s\n' 2>/dev/null | sort -rn | head -5 | awk '{print $1}'| xargs echo rm -fr

