#!/bin/bash
#check if we're on *nix system
#write the version.html file

#don't exit even if a command fails
set +e

if (ls &> /dev/null); then
	echo "Retroshare Gui version : " > gui/help/version.html
	if ( /usr/bin/git log -n 1 &> /dev/null); then
		#retrieve git information
		echo "Git version : $(git status | grep branch | cut -c 3-) $(git log -n 1 | grep commit)" >> gui/help/version.html
	fi
	if ( /usr/bin/git log -n 1 | grep svn &> /dev/null); then
		#retrieve git svn information
		echo "Svn version : $(git log -n 1 | grep svn | awk '{print $2}' | head -1)" >> gui/help/version.html
	elif ( /usr/bin/git log -n 10 | grep svn &> /dev/null); then
		#retrieve git svn information
		echo "Svn closest version : $(git log -n 10 | grep svn | awk '{print $2}' | head -1)" >> gui/help/version.html
	fi

	if ( /usr/bin/svn info &> /dev/null); then
		echo "Svn version : $(svn info | grep '^Revision:')" >> gui/help/version.html
	fi
	date >> gui/help/version.html
	echo "" >> gui/help/version.html
	echo "" >> gui/help/version.html
fi

#write the rsguiversion.h file
if ( git log -n 1 &> /dev/null); then
	#retrieve git information
	version="git : $(git status | grep branch | cut -c 6-) $(git log -n 1 | grep commit)"
fi

if ( git log -n 1 | grep svn &> /dev/null); then
	#retrieve git svn information
	version="$version  svn : $(git log -n 1 | grep svn | awk '{print $2}' | head -1 | sed 's/.*@//')"
elif ( git log -n 10 | grep svn &> /dev/null); then
	#retrieve git svn information
	version="$version  svn closest version : $(git log -n 10 | grep svn | awk '{print $2}' | head -1 | sed 's/.*@//')"
fi

if ( svn info &> /dev/null); then
	version=$(svn info | grep '^Revision:')
fi
if [[ $version != '' ]]; then
	version="$version  date : $(date +'%T %m.%d.%y')"
	echo "Writing version to util/rsguiversion.h : $version "
	sed -i "s/GUI_REVISION .*/GUI_REVISION \"$version\"/g" util/rsguiversion.h
fi
echo "version_detail.sh scripts finished"
exit 0
