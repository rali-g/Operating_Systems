#! /bin/bash

find ~ -maxdepth 1 -type f -user $(whoami) -exec chmod 664 {} \; 2>/dev/null
