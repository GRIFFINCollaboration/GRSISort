# Source this script to help report Errors in GRSISort
# Author by Ryan Dunlop, 22/10/2015
echo SYSTEM       = `uname`
echo GRSISYS      = $GRSISYS
echo ROOTSYS      = $ROOTSYS

echo ROOT Version = `root-config --version`
echo '\nComputer and Path to File that failed: \n'

cd ${GRSISYS}
echo GRSISort Branch = `git rev-parse --abbrev-ref HEAD`
echo '\nLast Commit: ' 

git log -1

echo '\nPut Error Here:\n\n\n\n'

echo 'Last Working Commit: Put commit here\n\n'

echo 'What I have tried so far\n\n\n'
