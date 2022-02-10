# Source this script to set up the GRSISort build that this script is part of.
#
# Conveniently an alias like this can be defined in ~/.cshrc:
#   alias thisgrsi "source thisgrsi.sh"
#
# This script if for the csh like shells, see thisgrsi.sh for bash like shells.
#
# Author: Fons Rademakers, 18/8/2006

if ($?GRSISYS) then
   set old_grsisys="$GRSISYS"
endif

# $_ should be source .../thisgrsi.csh
set ARGS=($_)
if ("$ARGS" != "") then
   set thisgrsi="`dirname ${ARGS[2]}`"
else
   # But $_ might not be set if the script is source non-interactively.
   # In [t]csh the sourced file is inserted 'in place' inside the
   # outer script, so we need an external source of information
   # either via the current directory or an extra parameter.
   if ( -e thisgrsi.csh ) then
      set thisgrsi=${PWD}
   else if ( -e bin/thisgrsi.csh ) then 
      set thisgrsi=${PWD}/bin
   else if ( "$1" != "" ) then
      if ( -e ${1}/bin/thisgrsi.csh ) then
         set thisgrsi=${1}/bin
      else if ( -e ${1}/thisgrsi.csh ) then
         set thisgrsi=${1}
      else 
         echo "thisgrsi.csh: ${1} does not contain a GRSI installation"
      endif 
   else
      echo 'Error: The call to "source where_GRSISort_is/bin/thisgrsi.csh" can not determine the location of the GRSI installation'
      echo "because it was embedded another script (this is an issue specific to csh)."
      echo "Use either:"
      echo "   cd where_GRSISort_is; source bin/thisgrsi.csh"
      echo "or"
      echo "   source where_GRSISort_is/bin/thisgrsi.csh where_GRSISort_is" 
   endif
endif

if ($?thisgrsi) then 

setenv GRSISYS "`(cd ${thisgrsi};pwd)`"

if ( -e $GRSISYS/GRSIData ) then
	setenv GRSIDATA $GRSISYS/GRSIData
endif
if ( -e $GRSISYS/ILLData ) then
	setenv ILLDATA $GRSISYS/ILLData
endif
if ( -e $GRSISYS/iThembaData ) then
	setenv ITHEMBADATA $GRSISYS/iThembaData
endif

if ($?old_grsisys) then
   setenv PATH `echo $PATH | sed -e "s;:$old_grsisys/bin:;:;g" \
                                 -e "s;:$old_grsisys/bin;;g"   \
                                 -e "s;$old_grsisys/bin:;;g"   \
                                 -e "s;$old_grsisys/bin;;g"`
   if ($?LD_LIBRARY_PATH) then
      setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | \
                             sed -e "s;:$old_grsisys/lib:;:;g" \
                                 -e "s;:$old_grsisys/lib;;g"   \
                                 -e "s;$old_grsisys/lib:;;g"   \
                                 -e "s;$old_grsisys/lib;;g"`
		if ( -e $GRSISYS/GRSIData ) then
			setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | \
											sed -e "s;:$old_grsisys/GRSIData/lib:;:;g" \
												 -e "s;:$old_grsisys/GRSIData/lib;;g"   \
												 -e "s;$old_grsisys/GRSIData/lib:;;g"   \
												 -e "s;$old_grsisys/GRSIData/lib;;g"`
		endif
		if ( -e $GRSISYS/ILLData ) then
			setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | \
											sed -e "s;:$old_grsisys/ILLData/lib:;:;g" \
												 -e "s;:$old_grsisys/ILLData/lib;;g"   \
												 -e "s;$old_grsisys/ILLData/lib:;;g"   \
												 -e "s;$old_grsisys/ILLData/lib;;g"`
		endif
		if ( -e $GRSISYS/ITHEMBAData ) then
			setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | \
											sed -e "s;:$old_grsisys/ITHEMBAData/lib:;:;g" \
												 -e "s;:$old_grsisys/ITHEMBAData/lib;;g"   \
												 -e "s;$old_grsisys/ITHEMBAData/lib:;;g"   \
												 -e "s;$old_grsisys/ITHEMBAData/lib;;g"`
		endif
   endif
   if ($?DYLD_LIBRARY_PATH) then
      setenv DYLD_LIBRARY_PATH `echo $DYLD_LIBRARY_PATH | \
                             sed -e "s;:$old_grsisys/lib:;:;g" \
                                 -e "s;:$old_grsisys/lib;;g"   \
                                 -e "s;$old_grsisys/lib:;;g"   \
                                 -e "s;$old_grsisys/lib;;g"`
		if ( -e $GRSISYS/GRSIData ) then
			setenv DYLD_LIBRARY_PATH `echo $DYLD_LIBRARY_PATH | \
											sed -e "s;:$old_grsisys/GRSIData/lib:;:;g" \
												 -e "s;:$old_grsisys/GRSIData/lib;;g"   \
												 -e "s;$old_grsisys/GRSIData/lib:;;g"   \
												 -e "s;$old_grsisys/GRSIData/lib;;g"`
		endif
		if ( -e $GRSISYS/ILLData ) then
			setenv DYLD_LIBRARY_PATH `echo $DYLD_LIBRARY_PATH | \
											sed -e "s;:$old_grsisys/ILLData/lib:;:;g" \
												 -e "s;:$old_grsisys/ILLData/lib;;g"   \
												 -e "s;$old_grsisys/ILLData/lib:;;g"   \
												 -e "s;$old_grsisys/ILLData/lib;;g"`
		endif
		if ( -e $GRSISYS/ITHEMBAData ) then
			setenv DYLD_LIBRARY_PATH `echo $DYLD_LIBRARY_PATH | \
											sed -e "s;:$old_grsisys/ITHEMBAData/lib:;:;g" \
												 -e "s;:$old_grsisys/ITHEMBAData/lib;;g"   \
												 -e "s;$old_grsisys/ITHEMBAData/lib:;;g"   \
												 -e "s;$old_grsisys/ITHEMBAData/lib;;g"`
		endif
   endif
   if ($?MANPATH) then
      setenv MANPATH `echo $MANPATH | \
                             sed -e "s;:$old_grsisys/man:;:;g" \
                                 -e "s;:$old_grsisys/man;;g"   \
                                 -e "s;$old_grsisys/man:;;g"   \
                                 -e "s;$old_grsisys/man;;g"`
   endif
endif


if ($?MANPATH) then
# Nothing to do
else
   # Grab the default man path before setting the path to avoid duplicates 
   if ( -X manpath ) then
      set default_manpath = `manpath`
   else
      set default_manpath = `man -w`
   endif
endif

set path = ($GRSISYS/bin $path)

if ($?LD_LIBRARY_PATH) then
   setenv LD_LIBRARY_PATH $GRSISYS/lib:$LD_LIBRARY_PATH      # Linux, ELF HP-UX
else
   setenv LD_LIBRARY_PATH $GRSISYS/lib
endif

if ($?GRSIDATA) then
	setenv LD_LIBRARY_PATH $GRSIDATA/lib:$LD_LIBRARY_PATH
endif

if ($?ILLDATA) then
	setenv LD_LIBRARY_PATH $ILLDATA/lib:$LD_LIBRARY_PATH
endif

if ($?ITHEMBADATA) then
	setenv LD_LIBRARY_PATH $ITHEMBADATA/lib:$LD_LIBRARY_PATH
endif

if ($?DYLD_LIBRARY_PATH) then
   setenv DYLD_LIBRARY_PATH $GRSISYS/lib:$DYLD_LIBRARY_PATH  # Mac OS X
else
   setenv DYLD_LIBRARY_PATH $GRSISYS/lib
endif

if ($?SHLIB_PATH) then
   setenv SHLIB_PATH $GRSISYS/lib:$SHLIB_PATH                # legacy HP-UX
else
   setenv SHLIB_PATH $GRSISYS/lib
endif

if ($?LIBPATH) then
   setenv LIBPATH $GRSISYS/lib:$LIBPATH                      # AIX
else
   setenv LIBPATH $GRSISYS/lib
endif

if ($?PYTHONPATH) then
   setenv PYTHONPATH $GRSISYS/lib:$PYTHONPATH
else
   setenv PYTHONPATH $GRSISYS/lib
endif

if ($?MANPATH) then
   setenv MANPATH `dirname $GRSISYS/man/man1`:$MANPATH
else
   setenv MANPATH `dirname $GRSISYS/man/man1`:$default_manpath
endif

endif # if ("$thisgrsi" != "")

set thisgrsi=
set old_grsisys=

