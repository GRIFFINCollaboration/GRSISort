# Run this script to help report Errors in GRSISort
# Author by Ryan Dunlop, 22/10/2015
echo SYSTEM       = `uname`
echo GRSISYS      = $GRSISYS
echo ROOTSYS      = $ROOTSYS

echo ROOT Version = `root-config --version`
printf "\nComputer and Path to File that failed: \n"

LASTDIR=$PWD
cd $GRSISYS
echo GRSISort Branch = `git rev-parse --abbrev-ref HEAD`
printf "\nLast Commit: " 

git log -1

printf "\nPut Error Here:\n\n\n\n"

printf "Last Working Commit: Put commit here\n\n"

printf "What I have tried so far\n\n\n"
cd $LASTDIR
