#!/bin/bash
#Source me!!!

# this should be sourced, not executed, to work properly
if [[ "$_" == $0 ]]; then
    echo "Try sourcing me!";
    exit 0;
fi

SOURCEDFILE=`readlink -f $BASH_SOURCE`

export GRSISYS=`dirname $SOURCEDFILE`
echo "GRSISYS is now $GRSISYS"

if [ ! -d $ROOTSYS ]; then
    echo "ROOTSYS=$ROOTSYS does not point to a directory. Please rectify before installing GRSISort."
    return 0;
fi

export PATH=$GRSISYS/bin:$PATH
echo "Added $GRSISYS/bin to PATH"

export LD_LIBRARY_PATH=$GRSISYS/libraries:$LD_LIBRARY_PATH
echo "Added $GRSISYS/libraries to PATH."

export MANPATH=$MANPATH:$GRSISYS/man
echo "Added $GRSISYS/man to MANPATH."

echo
echo
echo "To avoid running this script every session add the following to your ~/.bashrc file"
echo "export GRSISYS=$GRSISYS"
echo "export PATH=\$GRSISYS/bin:\$PATH" 
echo "export LD_LIBRARY_PATH=\$GRSISYS/libraries:\$LD_LIBRARY_PATH"
echo "export MANPATH=$MANPATH:$GRSISYS/man"


# clean up variables
unset SOURCEDFILE
