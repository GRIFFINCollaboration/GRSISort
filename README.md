#GRSISort

A lean, mean, sorting machine. 

To compile for a .sh shell execute 'source SOURCEME.sh' in your terminal from the GRSISort Directory

To avoid running this script every session, add the following to your **~/.bashrc**
```
export GRSISYS=/path/to/dir/of/GRSISort
export PATH=$GRSISYS/bin:$PATH
export LD_LIBRARY_PATH=$GRSISYS/libraries:$LD_LIBRARY_PATH
```

To compile using a .crsh or .tcsh shell execute 'source SOURCEME.csh' in your terminal from the GRSISort Directory

To avoid running this script every session, add the following in your **~/.cshrc**  or **~/.tcshrc** 
```
setenv GRSISYS /path/to/dir/of/GRSISort
set path = ($path $GRSISYS/bin)
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:$GRSISYS/libraries
```

once complete type make in the GRSISort directory

-----------------------------------------
Running
-----------------------------------------
grsisort will open up a root session in the grsi environment
Commands:

grsisort NAMEOFMIDASFILE.mid -> Converts the midas file into a fragment tree
Adding the flag -suppress_error suppresses errors in the data stream if there are any

grsisort -s NAME_OF_FRAGMENT_TREE.root -> Starts sorting the fragment tree and creates the histograms defined in users/UserInitObj.h file.


More to follow.....

or better yet, switch to a bash shell :)
