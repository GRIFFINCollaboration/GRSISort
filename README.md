# GRSISort

A lean, mean, sorting machine.    

**For Update Information** Please Join the [Analysis_User](https://github.com/orgs/GRIFFINCollaboration/teams/analysis_users) team 

**There is a wiki!!  use the link->** [WIKI](http://github.com/GRIFFINCollaboration/GRSISort/wiki)

To compile for a .sh shell execute `source SOURCEME.sh` in your terminal from the GRSISort Directory

To avoid running this script every session, add the following to your **~/.bashrc**
```
export GRSISYS=/path/to/dir/of/GRSISort
export PATH=$GRSISYS/bin:$PATH
export LD_LIBRARY_PATH=$GRSISYS/libraries:$LD_LIBRARY_PATH
```

To compile using a .crsh or .tcsh shell execute `source SOURCEME.csh` in your terminal from the GRSISort Directory

To avoid running this script every session, add the following in your **~/.cshrc**  or **~/.tcshrc** 
```
setenv GRSISYS /path/to/dir/of/GRSISort
set path = ($path $GRSISYS/bin)
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:$GRSISYS/libraries
```
Warning: compiling in csh has been known to be an issue. Use sh if possible.


Once complete type `make` in the GRSISort directory

-----------------------------------------
Running
-----------------------------------------
grsisort will open up a root session in the grsi environment

Commands:

# Unpacking the MIDAS File
The strategy used by GRSISort is to unpack the MIDAS file and convert the information into a ROOT Tree format. This tree contains <a href="https://rawgit.com/wiki/r3dunlop/GRSISort/technical-docs/ROOT-Gen-Html/htmldoc/TFragment.html#TFragment:Data_Members" target="_blank">TFragments</a> and is essentially all of the information from the MIDAS File with higher compression and accessibility.
  
**grsisort NAME_OF_MIDAS_FILE.mid**  -> Converts the midas file into a fragment tree

**Single char options can now strung together, simular to other linux programs**
**word length options must have -- in front, also simular to other linux programs**

Currently accepted flags for the midas sort:
  * __--suppress_error__,   suppresses errors from failed data parsing appearing in stdout
  * __--log_error__,        sends errors from failed data parsing to file, will not send errors if supress error option is also used!
  * __--no_waveforms__,     does not add the wave form stored for each event in the mid file to the fragment tree
  * __-q__,                quits the grsi environment after completing the midas sort
  * __--work_harder__,      inputting a `<macro>.C` file, runs that macro on the sorted file

If multiple **.mid** files are included at the time of sort, this can even include the wildcard character `*`, grsisort will sequentially sort all of these MIDAS files and write them to their own fragment trees.

# Sorting the Fragment Tree
**grsisort [-as ] NAME_OF_FRAGMENT_TREE.root**   -> Starts sorting the fragment tree into (-s) user defined hists and/or (-a) analysis trees.
  * Histograms are defined in **users/UserInitObj.h**
  * How to fill the histogram is defined in **users/UserFillObj.h** 
  
If **-a** or **-s** are left off of the command, the fragment tree will be loaded into the grsi environment for the user to enjoy.

Currently accepted flags for the fragment tree sort:
  * __-a__,                creates an analysis tree to be used for doing a full analysis of the data
  * __-s__,                sorts the fragment tree into user defined hits. 
  * __-q__,                quits the grsi environment after completeing the fragment sort
  * __--no_speed__,         suppresses the output from the PROOF speedometer
  * __--work_harder__,      inputting a `<macro>.C` file, runs that macro on the sorted file

If multiple fragment trees are included at the time of the sort, or are created from multiple MIDAS files during the **autosort**, they will be sorted sequentially. However, it should be noted that although different fragment trees will be sorted into different analysis trees, if a **-s** sort is called, all of the fragment trees will be histogrammed into a single **hists.root**. If you would like to output multiple **hists.root** files, please run an external script to call grsisort multiple times.

-----------------------------------------
 Utilities
-----------------------------------------
Utilities, such as analysis scripts, asre kept in the **util** directory. Below are a list of useful utilities and how to use them.
  * **Root2Rad**, Converts the 1D and 2D histograms in a root file to Radware .spe and .mat formats
  To compile:
```
   g++ Root2Rad.cxx -oRoot2Rad `root-config --cflags --libs`
```
  To Run:
```
   ./Root2Rad NAME_OF_ROOT_FILE.root
```

**Scripts**
  * **auto_eff.C**, Reads in a root file and calculates the efficiency of each HPGe Crystal based on a specfic source and activity


More to follow...

