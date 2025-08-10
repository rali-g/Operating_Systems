#! /bin/bash

find / -type f -user $(whoami) 2>/dev/null -regex '^.*\.blend[0-9]+$'
