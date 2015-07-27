#!/bin/bash

#don't exit even if a command fails
set +e

version=0
vcs="unknown"
githash="unknown"
if ( git log -n 1 &> /dev/null); then
	#note - where a single number is currently rather misleading
	#though a version numer for approx human reading is nice
	
	#try to try and pull SVN revision from last commit
	cleverversion="$(git log -n 1 | awk '/svn/ {print $2}' | head -1 | sed 's/.*@//')"
		if [ -z $cleverversion ]; then
			#"failed to get SVN version from most recent git"
			cleverversion="$(git log -n 10 | awk '/svn/ {print $2}' | head -1 | sed 's/.*@//')"
		fi
		
	#try to try and pull SVN revision from last 10 commits
		if [ $cleverversion ]; then
			#"got clever version"
			version=$cleverversion
		fi
	
	#if not got a version using smart method, use easy method
		if [ $version == 0 ]; then
			#retrieve git information - just get number of commits
			#note - this will not match svn commits
			#note - with caves repo - start of history is cutoff? so it is unusually small number
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
