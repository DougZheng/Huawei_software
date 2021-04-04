#!/bin/bash
sh build.sh
cd bin
time ./CodeCraft-2021 <../../2_data/training-1.txt >../../2_data/training-1.out 2>tmp1.out
time ./CodeCraft-2021 <../../2_data/training-2.txt >../../2_data/training-2.out 2>tmp2.out
cd ../../checker
./2_checker