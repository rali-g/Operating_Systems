#! /bin/bash

find . -type f -maxdepth 1 -printf "%n %p\n" 2>/dev/null \
| sort -nr | head -5 | awk '{printf "%s\n", $2}'

