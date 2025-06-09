#!/bin/bash
# shellcheck disable=all

DATADIR=<your data directory here>
SORTOPTIONS="your grsisort options here, e.g. --recommended"
CALFILES="any and all cal-files you want to use"
ANALYSISDIR="." # directory for analysis trees
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
		echo "${DATADIR}/run${run}_000.mid does not exist"
      continue
   fi
	# loop over all subruns that haven't been changed in the last three minutes
	# (only important when using this during an experiment)
   for midasFile in `find ${DATADIR} -maxdepth 1 -amin +3 -name "run${run}_???.mid" | sort` ; do
      subrun=$(basename $midasFile | cut -d '_' -f2 | cut -d '.' -f1)
		analysisFile="$ANALYSISDIR/analysis${run}_${subrun}.root"
		# if the analysis file exists, we don't re-run the analysis
      if [ -e $analysisFile ] ; then
         continue
      fi
		# touching the file means it exists now and if we run this script in multiple terminal
		# the other scripts won't try and start sorting this file
      touch $analysisFile
      logFile="${LOGFILES}/log${run}_${subrun}.txt"
		# print the sort command to the log-file
      echo "grsisort $SORTOPTIONS $CALFILES $midasFile" | tee $logFile
		# start sorting and append all output to the log-file
		# (2>&1 redirects stderr to stdout so we get error messages in the log file as well)
      grsisort $SORTOPTIONS $CALFILES $midasFile 2>&1 | tee -a $logFile
		# if necessary you can make certain file-permissions are set correctly
		# e.g. when sorting the same data for multiple people
      chmod 664 analysis${run}_${subrun}.root $logFile
		# moved the output files into the right directories, or if that fails, remove the file we touched at the beginning
		mv analysis${run}_${subrun}.root $analysisFile || rm $analysisFile
		if [ -e fragment${run}_${subrun}.root ] ; then
			mv fragment${run}_${subrun}.root ${FRAGMENTTREES}/fragment${run}_${subrun}.root
		fi
   done
done
