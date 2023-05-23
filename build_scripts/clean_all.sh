#!/bin/bash

echo "this will clean all examples"
sleep 2

for dir in */; do
	if [ -f "${dir}Makefile" ]; then
		echo "running 'make clean' in ${dir}"
		(cd "${dir}" && make clean)
	else
		echo "no Makefile in ${dir}"
	fi
done
