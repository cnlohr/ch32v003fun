#!/bin/bash

echo "this will build all examples"
sleep 2

cd ../examples

for dir in */; do
	if [ -f "${dir}Makefile" ]; then
		echo "running 'make build' in ${dir}"
		(cd "${dir}" && make build)
	else
		echo "no Makefile in ${dir}"
	fi
done

cd ../build_scripts
