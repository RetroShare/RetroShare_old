#!/bin/bash

#get date in int-like format YYYYMMDD
version=$(git log -n 1 --date=short --pretty=format:' %ad '|sed 's/-//g')

#version emulating SVN - unused
#svn_emu_version=$(($(git rev-list --count HEAD)+8613-2391))
#+8613-2391 to approx matchching SVN number on caves conversion

githash=$(git log --format="%H" -n 1)
gitinfo=$(git describe --tags)
gitbranch=$(git rev-parse --abbrev-ref HEAD)
