#!/bin/sh
filesdir=$1
searchstr=$2

finder () { #inputs are filesdir and searchstr
	MATCHING_LINES=$(grep -r "$2" "$filesdir" | wc -l)
	NUMBER_FILES=$(grep -l "$2" "$filesdir"/* | wc -l)
	#OUTPUT= grep -l "$2" "$filesdir"/*
	echo $NUMBER_FILES
	echo $MATCHING_LINES

}

find_match_lines () {
	grep -r "$2" "$filesdir" | wc -l
}

find_number_files () {
	grep -l "$2" "$filesdir"/* | wc -l
}

if [ $# -ne 2 ] 
then #invalid number of arguments
	echo "failed: invalid number of arguemnts"	
	exit 1

elif [ ! -d "$filesdir" ] 
then #filesdir doesnt represent valid dir
	echo "failed: $filesdir doesnt represent a directory on the filesystem"
	exit 1
else
	num_files=$(find_number_files $1 $2)
	matching_lines=$(find_match_lines $1 $2)
	echo "The number of files are $num_files and the number of matching lines are $matching_lines"
	exit 0
fi

