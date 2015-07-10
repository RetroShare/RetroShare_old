#!/bin/bash

#don't exit even if a command fails
set +e


version=$(../../get_version.sh)

if [[ ${version} != '' ]]; then
	echo "Writing version to retroshare/rsautoversion.h : ${version}"
	echo "#define RS_REVISION_NUMBER $version" > retroshare/rsautoversion.h
fi
echo "script auto_version.sh finished normally"
exit 0
