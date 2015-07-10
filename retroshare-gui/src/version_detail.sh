#!/bin/bash
#check if we're on *nix system
#write the version.html file

#don't exit even if a command fails
set +e


#alternate method from Heini
#if [[ -c /dev/null ]]; then
if (ls &> /dev/null); then
	echo "Retroshare Gui version : " > gui/help/version.html
	
		version=$(../../get_version.sh)

		vcs=$(../../get_version.sh vcs)
		githash=$(../../get_version.sh githash)
		
		echo "VCS: $vcs" >> gui/help/version.html
		if [ "$vcs" == "git" ]; then
			echo "Git Hash : $githash" >> gui/help/version.html
		fi
		echo "Revision Number: $version" >> gui/help/version.html
	date >> gui/help/version.html
	echo "" >> gui/help/version.html
	echo "" >> gui/help/version.html
fi
echo "version_detail.sh scripts finished"
exit 0
