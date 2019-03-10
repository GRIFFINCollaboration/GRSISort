# Source this script to set up the GRSISort build that this script is part of.
#
# Conveniently an alias like this can be defined in .bashrc:
#   alias thisgrsi=". bin/thisgrsi.sh"
#
# This script if for the bash like shells, see thisgrsi.csh for csh like shells.
#
# Author: Fons Rademakers, 18/8/2006
# Adapted by Ryan Dunlop, 8/9/2015

drop_from_path()
{
   # Assert that we got enough arguments
   if test $# -ne 2 ; then
      echo "drop_from_path: needs 2 arguments"
      return 1
   fi

   p=$1
   drop=$2

   newpath=`echo $p | sed -e "s;:${drop}:;:;g" \
                          -e "s;:${drop};;g"   \
                          -e "s;${drop}:;;g"   \
                          -e "s;${drop};;g"`
}

if [ -n "${GRSISYS}" ] ; then
   old_grsisys=${GRSISYS}
fi

if [ "x${BASH_ARGV[0]}" = "x" ]; then
    if [ ! -f thisgrsi.sh ]; then
        echo ERROR: must "cd where/grsi/is" before calling ". thisgrsi.sh" for this version of bash!
        GRSISYS=; export GRSISYS
        return 1
    fi
    GRSISYS="$PWD"; export GRSISYS
else
    # get param to "."
    thisgrsi=$(dirname ${BASH_ARGV[0]})
    GRSISYS=$(cd ${thisgrsi};pwd); export GRSISYS
fi

if [ -e $GRSISYS/GRSIData ] ; then
	export GRSIDATA=$GRSISYS/GRSIData
fi
if [ -e $GRSISYS/ILLData ] ; then
	export ILLDATA=$GRSISYS/ILLData
fi
if [ -e $GRSISYS/iThembaData ] ; then
	export ITHEMBADATA=$GRSISYS/iThembaData
fi

if [ -n "${old_grsisys}" ] ; then
   if [ -n "${PATH}" ]; then
      drop_from_path "$PATH" ${old_grsisys}/bin
      PATH=$newpath
   fi
   if [ -n "${LD_LIBRARY_PATH}" ]; then
      drop_from_path $LD_LIBRARY_PATH ${old_grsisys}/lib
      LD_LIBRARY_PATH=$newpath
   fi
   if [ -n "${DYLD_LIBRARY_PATH}" ]; then
      drop_from_path $DYLD_LIBRARY_PATH ${old_grsisys}/lib
      DYLD_LIBRARY_PATH=$newpath
   fi
   if [ -n "${MANPATH}" ]; then
      drop_from_path $MANPATH ${old_grsisys}/man
      MANPATH=$newpath
   fi
fi

if [ -z "${MANPATH}" ]; then
   # Grab the default man path before setting the path to avoid duplicates 
   if `which manpath > /dev/null 2>&1` ; then
      default_manpath=`manpath`
   else
      default_manpath=`man -w 2> /dev/null`
   fi
fi

if [ -z "${PATH}" ]; then
   PATH=$GRSISYS/bin; export PATH
else
   PATH=$GRSISYS/bin:$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$GRSISYS/lib; export LD_LIBRARY_PATH       # Linux, ELF HP-UX
else
   LD_LIBRARY_PATH=$GRSISYS/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH=$GRSISYS/lib; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH=$GRSISYS/lib:$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi

if [ -z "${MANPATH}" ]; then
   MANPATH=`dirname $GRSISYS/man/man1`:${default_manpath}; export MANPATH
else
   MANPATH=`dirname $GRSISYS/man/man1`:$MANPATH; export MANPATH
fi

#if [ "x`grsi-config --arch | grep -v win32gcc | grep -i win32`" != "x" ]; then
#  GRSISYS="`cygpath -w $GRSISYS`"
#fi

#if [ "x`grsi-config --arch | grep -v win64gcc | grep -i win64`" != "x" ]; then
#  GRSISYS="`cygpath -w $GRSISYS`"
#fi

unset old_grsisys
unset thisgrsi

