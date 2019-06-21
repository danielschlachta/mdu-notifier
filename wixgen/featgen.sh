#!/bin/bash

FEATGEN=$0

if [ -d "$*" ]
then
    pushd $* > /dev/null
    find * -maxdepth 0 -exec "$FEATGEN" {} \;
    popd > /dev/null
else    
    FID=`echo $* | sed 's,[^A-Za-z0-9._],,g'`
    echo '<ComponentRef Id="'"$FID"'" />'     
fi
