#!/bin/sh
writefile=$1
writestr=$2
DIR=$(dirname "$writefile")
#echo $DIR
if [ $# -ne 2 ] 
then #invalid number of arguments
	echo "failed: invalid number of arguemnts"	
	exit 1
else
	if [ ! -d $DIR ] 
	then #Create dir
		echo "Created dir"
		mkdir -p "$DIR" 
		touch $writefile 
	else
		if [ ! -e $writefile ] 
		then #create file
			touch $writefile 
		fi
	fi
	echo $writestr > $writefile || ( echo "Failed" && exit 1)
	exit 0
fi


