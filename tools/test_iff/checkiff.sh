#!/bin/sh

# set executable
PARSEIFF=./parseiff
if [ ! -x ${PARSEIFF} ] ; then
	PARSEIFF="$(dirname $0)/parseiff"
fi

if [ "$1" = "" ] ; then
	echo "Usage: $0 <file|directory>"
	echo ""
	echo "Just run the parser on files. For directories, list files with parsing errors."
	exit 1
fi

if [ -d "$1" ] ; then
	find "$1" |
		while IFS= read line; do
			if [ -f "$line" ] && file "$line" | grep -q "IFF data" ; then
				${PARSEIFF} "$line" > /dev/null
				#if ! ${PARSEIFF} "$line" > /dev/null ; then
				#	echo "error in $line"
				#fi
			fi
		done
else
	${PARSEIFF} $1
fi
