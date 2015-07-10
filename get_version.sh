#!/bin/bash

#don't exit even if a command fails
set +e

version=0
vcs="unknown"
githash="unknown"
if ( git log -n 1 &> /dev/null); then
	#retrieve git information - just get number of commits
	#note - this will not match svn commits
	#note - with caves repo - start of history is cutoff? so it is unusually small number
	#note - should really provide hash of last git commit - this is *much* more handy for unoffical builds!
	#where a single number is currently rather misleading
	#though a version numer for approx human reading is nice!
	
	#this could still do some smart things HERE to try and pull SVN revision from logs
	
	#if not got a version using smart method, use easy method
        if [ $version == 0 ]; then
            version=$(git rev-list HEAD | wc -l)
        fi
	
	vcs="git"
	githash=$(git log --format="%H" -n 1)
	#BUT we are hopefully moving away from SVN ASAP!
elif ( svn info &> /dev/null); then
	version=$(svn info | awk '/^Revision:/ {print $NF}')
	vcs="svn"
else
    version=0001
    vcs="error"
fi

#echo "version is $version from $vcs"
if [ "$1" == "vcs" ]; then
    echo $vcs
    exit 0
fi


if [ "$1" == "githash" ]; then
    echo $githash
    exit 0
fi

echo $version
