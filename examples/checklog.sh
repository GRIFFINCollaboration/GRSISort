#!/bin/bash
# shellcheck disable=all

# simple script to check grsisort log-files if sorts have finished correctly
# checks if the last line is "bye,bye" (good) or "Deleting thread" (bad)

LOGFILEDIR=<directory where all the log-files are, without trailing slash>

firstRun=1
lastRun=1

if [ $# -eq 0 ] ; then
   echo "usage: $0 <first run> <last run>"
   exit
fi

if [ $# -ge 1 ] ; then
   firstRun=$1
   outputFileName="logcheck$firstRun.txt"
fi

if [ $# -ge 2 ] ; then
   lastRun=$2
   outputFileName="logcheck$firstRun-$lastRun.txt"
else
   lastRun=$firstRun
fi

echo -e "\nChecking logs for runs from $firstRun to $lastRun" > $outputFileName

# loop from first to last run
for run in `seq $firstRun $lastRun` ; do
   echo "   Checking run $run" >> $outputFileName
	# loop over all subruns
   for (( subrun=0; subrun<=999; subrun++ )) ; do
		printf -v logFile "$LOGFILEDIR/log%05d_%03d.txt" $run $subrun
		# we assume all sub-runs are sorted in order so if the log-file of a sub-run
		# doesn't exist, we are done with this run
      if [ ! -e ${logFile} ] ; then
         break
      else
			# get the last line of the log-file
         lastLine=$( tail -n 1 ${logFile} )
			# check if the first seven letters of the last line are "bye,bye"
         check1=${lastLine::7}
         if [ "${check1}" != "bye,bye" ] ; then
            echo "      Run ${run} Subrun ${subrun} has issues" >> $outputFileName      
         fi
			# check if the first fifteen letters of the last line are "Deleting thread"
         check2=${lastLine::15}
         if [ "${check2}" == "Deleting thread" ] ; then
            echo "         Stuck deleting thread" >> $outputFileName      
         fi
      fi
   done
   echo "   Run $run checked" >> $outputFileName
done

echo -e "Checked logs for runs from $firstRun to $lastRun\n" >> $outputFileName
