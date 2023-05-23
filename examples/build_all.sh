#!/bin/bash

echo "this will build all examples"
sleep 2

for dir in */; do
	if [ -f "${dir}Makefile" ]; then
		echo "running 'make' in ${dir}"
		(cd "${dir}" && make)
	else
		echo "no Makefile in ${dir}"
	fi
done
