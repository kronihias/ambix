#!/bin/bash

set -e

echo "Please run this script from the _bin folder"

PWD=`pwd`

if [ -f $PWD/../../lv2-ttl-generator/lv2_ttl_generator.exe ]; then
  GEN=$PWD/../../lv2-ttl-generator/lv2_ttl_generator.exe
  EXT=dll
else
  GEN=$PWD/../../lv2-ttl-generator/lv2_ttl_generator
  EXT=so
fi

FOLDERS=`find ./lv2 -name \*.lv2`

for i in $FOLDERS; do
  cd $i
  FILE=`ls *.$EXT | sort | head -n 1`
  $GEN ./$FILE
  cd ../../
done

