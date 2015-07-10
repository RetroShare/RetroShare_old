#!/bin/bash

#don't exit even if a command fails
set +e

version=0001
vcs="unknown"
if ( git log -n 1 &> /dev/null); then
	#retrieve git information - just get number of commits
	#note - this will not match svn commits
	#note - with caves repo - start of history is cutoff? so it is unusually small number
	#note - should really provide hash of last git commit - this is *much* more handy for unoffical builds!
	#where a single number is currently rather misleading
	#though a version numer for approx human reading is nice!
	version=$(git rev-list HEAD | wc -l)
	vcs="git"
elif ( svn info &> /dev/null); then
	version=$(svn info | awk '/^Revision:/ {print $NF}')
	vcs="svn"
else
    version=0001
    vcs="error"
fi

#echo "version is $version from $vcs"

echo $version
