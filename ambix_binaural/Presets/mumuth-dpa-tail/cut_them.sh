#!/bin/bash
echo "Bash version ${BASH_VERSION}..."

for i in 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 020 021 022 023 024 025 026 027 028 029
do
   echo "Processing file $i"
	sox dpa-{$i}.wav dpa-{$i}_cut.wav trim 0.050
done