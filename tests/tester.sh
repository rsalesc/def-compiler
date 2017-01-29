#!/bin/bash
i=1
for file in dahia/*.def; do
    basename $file
    ./$1 $file  out_`basename $file`.txt
    diff out_`basename $file`.txt dahia/ans_`basename $file`.txt
    echo
    i=$((i + 1))
done
