#! /bin/bash

counter=0
result=$(mktemp)
while read -r line; do
    counter=$((counter + $(grep -o 'error' $line | wc -l)))
done< <(find my_logs/ -regex '.*\/[a-zA-Z0-9_]*_[0-9]*\.log')

echo $counter
