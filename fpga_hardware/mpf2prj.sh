#!/bin/bash

while IFS='' read -r line || [[ -n "$line" ]]; do
    if [[ $line == Project_File_*=\ cores/* ]]; then
        echo verilog work \"${line##*= }\" >> orpsoc_top.prj
    fi
done < "$1"

