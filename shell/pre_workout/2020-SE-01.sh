#! /bin/bash

find ~ -type f -perm 644 | xargs chmod g+w

