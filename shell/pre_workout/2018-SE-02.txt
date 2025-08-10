#! /bin/bash

find /home/pesho -type f -printf '%T@ %i %n\n' | sort -nr | head -1 \
| awk '$3 > 1 {print $0}' | cut -d' ' -f3

