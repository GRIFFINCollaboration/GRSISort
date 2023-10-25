#!/bin/bash

DATADIR=<your data directory here>
SORTOPTIONS="your grsisort options here, e.g. --recommended"
CALFILES="any and all cal-files you want to use"
ANALYSISTREES="." # directory for analysis trees
FRAGMENTTREES="." # directory for fragment trees
LOGFILES="." # directory for log files

firstRun=1
lastRun=1

if [ $# -eq 0 ] ; then
   echo "usage: $0 <first run> <last run>"
   exit
fi

if [ $# -ge 1 ] ; then
   firstRun=$1
fi

if [ $# -ge 2 ] ; then
   lastRun=$2
else
   lastRun=$firstRun
fi

# loop over all runs from first to last
for run in `seq $firstRun $lastRun` ; do
	# check if the midas-file of the first subrun exists
   if [ ! -e ${DATADIR}/run${run}_000.mid ] ; then
      continue
   fi
	# loop over all subruns that haven't been changed in the last three minutes
	# (only important when using this during an experiment)
   for midasFile in `find ${DATADIR} -maxdepth 1 -amin +3 -name "run${run}_???.mid"` ; do
      subrun=$(basename $midasFile | cut -d '_' -f2 | cut -d '.' -f1)
		# if the analysis file exists, we don't re-run the analysis
      if [ -e ${ANALYSISTREES}/analysis${run}_${subrun}.root ] ; then
         continue
      fi
		# touching the file means it exists now and if we run this script in multiple terminal
		# the other scripts won't try and start sorting this file
      touch ${ANALYSISTREES}/analysis${run}_${subrun}.root
      logFile="${LOGFILES}/log${run}_${subrun}.txt"
		# print the sort command to the log-file
      echo "grsisort $SORTOPTIONS $CALFILES $midasFile" | tee $logFile
		# start sorting and append all output to the log-file
		# (2>&1 redirects stderr to stdout so we get error messages in the log file as well)
      grsisort $SORTOPTIONS $CALFILES $midasFile 2>&1 | tee -a $logFile
		# if necessary you can make certain file-permissions are set correctly
		# e.g. when sorting the same data for multiple people
      chmod 664 ${ANALYSISTREES}/analysis${run}_${subrun}.root fragment${run}_${subrun}.root log${run}_${subrun}.txt
		# moved the output files into the right directories, or if that fails, remove the file we touched at the beginning
		mv analysis${run}_${subrun}.root ${ANALYSISTREES}/analysis${run}_${subrun}.root || rm ${ANALYSISTREES}/analysis${run}_${subrun}.root
      mv fragment${run}_${subrun}.root ${FRAGMENTTREES}/fragment${run}_${subrun}.root || rm ${ANALYSISTREES}/analysis${run}_${subrun}.root
   done
done
