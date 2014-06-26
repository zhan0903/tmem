#!/bin/bash

for v in 1 2 4 8 16 32
do 
    for i in 8 64 4096
    do
         ./ttool -r -t $v -s $i
        echo "#$v $i"
        sleep 10
    done
done
