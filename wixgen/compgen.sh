#!/bin/bash

COMPGEN=$0

if [ -d "$*" ]
then
    echo '<Directory Id="'"$*"'Folder" Name="'$*'">' 
    pushd $* > /dev/null
    find * -maxdepth 0 -exec "$COMPGEN" {} \;
    popd > /dev/null
    echo '</Directory>'
else
    FILE=`pwd`/$*
    FID=`echo $* | sed 's,[^A-Za-z0-9._],,g'` 
    echo '<Component Id="'"$FID"'" Guid="'`uuidgen`'">'
    echo '  <File Id="'"$FID"'" Source="'`echo "$FILE" | sed 's,/,\\\\,g'`'" KeyPath="yes" Checksum="yes"/>'
    echo '</Component>'
fi
