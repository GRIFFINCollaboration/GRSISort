#!/bin/tcsh 
# source me!!!

# this should be sourced, not executed, to work properly
set sourced=($_)
if ("$sourced" == "") then
    echo "Try sourcing me!"
    exit 0;
endif

set sourcedfile=`readlink -f $sourced[2]`

setenv GRSISYS `dirname $sourcedfile`
echo "GRSISYS is now $GRSISYS"

if ( ! -d $ROOTSYS ) then
    echo "ROOTSYS=$ROOTSYS does not point to a directory. Please rectify before installing GRSISort."
endif

set path = ($path $GRSISYS/bin)
echo "Added $GRSISYS/bin to PATH"
 
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:$GRSISYS/libraries
echo "Added $GRSISYS/libraries to PATH."

echo "To avoid running this script every session, add the following in your ~/.cshrc or ~/.tcshrc file"
echo 'setenv GRSISYS $GRSISYS'
echo 'set path = ($path $GRSISYS/bin)'
echo 'setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:$GRSISYS/libraries'

#clean up the variables
unset sourced sourcedfile
