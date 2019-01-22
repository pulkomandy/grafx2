#!/bin/sh

# no argument : error
if [ ! -f $1 ] ; then
	exit 1
fi

# more than one argument : iterate
if [ $# -ne 1 ] ; then
	for arg in $@ ; do
		$0 $arg
	done | sort | uniq
	exit 0
fi

#list=$(otool -L $1 | sed '1d' | grep -v libSystem.B | grep -v /System/Library | grep -v $1 | cut -f 2 | cut -d' ' -f 1)
list=$(otool -L $1 | sed '1d' | cut -f 2 | grep -v ^/usr/lib/lib | grep -v ^/System/Library | grep -v $1 | cut -d' ' -f 1)

#echo "$1 => $list" >&2

# recursive calls
for lib in $list ; do
	echo $lib
	$0 $lib
done | sort | uniq
