#!/bin/bash

echo "this will clean all examples and their 'compile_commands.json'"
echo "'clangd' relies on these files to find the include paths"
sleep 2

cd ../examples

for dir in */; do
	if [ -f "${dir}Makefile" ]; then
		echo "running 'make clean' and 'make clangd_clean' in ${dir}"
		(cd "${dir}" && make clean && make clangd_clean)
	else
		echo "no Makefile in ${dir}"
	fi
done

cd ../build_scripts
