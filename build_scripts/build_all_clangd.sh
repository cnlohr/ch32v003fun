#!/bin/bash

echo "this will build all examples with 'bear' to generate 'compile_commands.json'"
echo "'clangd' relies on these files to find the include paths"
sleep 2

if ! command -v bear --help &> /dev/null
then
	echo "'bear' could not be found"
	echo "please install it from your package manager"
	exit
fi

for dir in */; do
	if [ -f "${dir}Makefile" ]; then
		echo "running 'bear -- make' in ${dir}"
		(cd "${dir}" && bear -- make)
	else
		echo "no Makefile in ${dir}"
	fi
done
