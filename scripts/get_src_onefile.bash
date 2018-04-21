#! /bin/bash
for i in src/level1/*.c src/level2/*.c src/level3/*.c include/*.h makefile; do
  echo "-----------------------------"
  echo "$i"
  echo "-----------------------------"
  cat "$i"
  echo
done
