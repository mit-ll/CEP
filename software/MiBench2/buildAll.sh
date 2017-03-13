#!/bin/sh

find . -d 1 -type d \( ! -name . \) -exec bash -c "cd {} && make clean && make && cp main.bin ../{}.bin && make clean" \;
