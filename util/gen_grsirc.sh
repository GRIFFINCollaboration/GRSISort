#!/bin/bash

MAJOR_ROOT_VERSION=$(root-config --version | cut -d '.' -f1)
MINOR_ROOT_VERSION=$(root-config --version | cut -d '.' -f2 | cut -d '/' -f1)

if [ "$MAJOR_ROOT_VERSION" -le 5 ]
then
	echo "Wrong ROOT version ${MAJOR_ROOT_VERSION}.${MINOR_ROOT_VERSION}, need at least ROOT 6!";
	exit 1;
fi

if [ "$MAJOR_ROOT_VERSION" -eq 6 ] && [ "$MINOR_ROOT_VERSION" -le 8 ]
then
	sed -i -e 's/\(Rint.PromptColor:[[:space:]]*\)\(%\)\([0-9A-Fa-f]\{6\}\)/\1#\3/g' .grsirc 
else
	sed -i -e 's/\(Rint.PromptColor:[[:space:]]*\)\(#\)\([0-9A-Fa-f]\{6\}\)/\1%\3/g' .grsirc 
fi
