The GainMatchGRIFFIN macro is designed to gain match a single run of GRIFFIN. It has been designed as a sequential execution of many smaller functions that will take the user from initial data set to calibration file and diagnostic images to evaluate the quality of the gainmatch. It is important to note that this is a *gain-matching* code, not an energy calibration code. Slopes and offsets are applied to each crystal so that its peaks will line up with the peaks in the crystal with the lowest channel number. A *rough*, linear, energy calibration may also be applied.

Usage:
The variables in the main() function (detailed below and identified in GainMatchGRIFFIN.C with "//-------------- VARIABLE DECLARATION SECTION --------------//") should be set for each gain-matching. When anything is changed in GainMatchGRIFFIN.C, it should be recompiled. The compilation line is given in the commented first line of the file. Once it has been compiled, the user may gain-match a run by running:

./GainMatchGRIFFIN <raw filename(s)>

from the command line. The file name of the raw data (i.e. fragmentXXXX_XXX.root) is taken as input to the compiled function. It is expected that the user will modify this macro for their own use if more nuanced control is necessary.

Output for calibrations using a single file: Two ROOT files, one .cal file, and four PNG diagnostic images.
	- calib_<raw filename>: This ROOT file contains the raw charge histograms for each channel that are created by the make_calibration_histograms function. They are named hst1, hst2, etc. where the number is the channel number. After executing the create_gainmatch_graphs function, it also contains energy vs. charge TGraphs for each of those histograms, with points for each of the peaks specified in the fittingpeaks array. If something goes wrong in the peak-finding, these TGraphs can be modified on a point-by-point basis before executing the create_GRIFFIN_cal function.
	- GRIFFINcal.cal (or whatever else you set the calFile variable to): This is the calibration file that is output from the create_GRIFFIN_cal function.
	- GRIFFIN_fitting_params.png: This PNG diagnostic canvas shows the slopes, offset, and chi^2 values from the linear fit of the graphs in the calib_<raw filename> file as a function of channel number.
	- GRIFFIN_calgraph.png: This diagnostic canvas plots all the graphs in the calib_<raw filename> file on top of each other.
	- newcal_<raw filename>: This ROOT file contains the calibrated energy histograms that are created by the recalibrate_spectra function for each channel. Again, they are named hst1, hst2, etc. This file also contains a few diagnostic histograms:
		- hstall: This is the sum of all available channel histograms.
		- residualXX: These histograms show the difference between the peak centroids of a single crystal and the peak centroids of the summed spectrum.
		- fwhm: This 2D histogram plots the FWHM for each crystal as a function of energy.
		- fwhmratio: This 2D histogram plots the FWTM/FWHM ratio for each crystal as a function of energy.
		- fwhm_all: This 2D histogram plots the FWHM for the summed spectrum (hstall) as a function of energy.
		- fwhmratio_all: This 2D histogram plots the FWTM/FWHM ratio for the summed spectrum (hstall) as a function of energy.
	- GRIFFIN_FWHM_diagnostic.png: This diagnostic canvas shows the histogram fwhm plotted as a candle plot (see below for more details on candle, or box-and-whiskers plots) and the fwhm_all histogram plotted on top, with star points. If the gain-matching has been successful, the star will be within the solid outline box.
	- GRIFFIN_FWHMratio_diagnostic.png: This diagnostic canvas shows the histogram fwhmratio plotted as a candle plot (see below for more details on candle, or box-and-whiskers plots) and the fwhmratio_all histogram plotted on top, with star points. If the gain-matching has been successful, the star will be within the solid outline box.

Calibrations using multiple fragment files:
It is occasionally useful to combine the counts from multiple subruns or runs before gain-matching (to use low-intensity peaks, for example.) This code has been set up to accomodate this. If the user lists multiple filenames in the command line call of GainMatchGRIFFIN, the output will differ slightly. All individual ROOT fragment files will have corresponding calib_<raw filename> and newcal_<raw filename> files. After these are created, however, hadd will be called to sum them into new ROOT files, respectively called calib_hst_summed.root and newcal_summed.root. The individual ROOT files will contain the charge and energy histograms listed above, but the TGraphs and the diagnostic histograms will exist on in the summed files.

The smaller functions that are used in the code are:
	- make_calibration_histograms
	- create_gainmatch_graphs
	- create_GRIFFIN_cal
	- recalibrate_spectra
	- check_calibration
More details on each of the smaller functions used in this macro are given below

The most commonly used variables are declared in the main() function at the bottom of GainMatchGRIFFIN.C. The variables that are hard-coded at the time of compilation are:
	- minchannel: lowest GRIFFIN channel number for gain-matching.
	- maxchannel: highest GRIFFIN channel number for gain-matching.
	- channelstoskip: a vector of integers that lists channels that should not be gain-matched, but that are between minchannel and maxchannel.
	- xbins_charge: the number of bins used in creating the raw charge histograms.
	- xmin_charge: the minimum bound on the charge axis in the raw charge histograms.
	- xmax_charge: the maximum bound on the charge axis in the raw charge histograms.
	- integration: the integration parameter by which charge is divided before it is calibrated. This should ideally be set to the integration parameter of the GRIFFIN digitizers, which have a default of 512.
	- roughenergy: set to kTRUE if a rough energy calibration is desired. set to kFALSE if not.
	- largepeak: the value of the largest peak. If you use a rough energy calibration, this should be in keV. Otherwise, it may be in charge units.
	- fittingpeaks: a vector of peaks used for fitting. While these values may be approximate, they should be in keV if a rough energy calibration is desired.
	- mindistance: the +/- value that sets the search window when looking for peaks in the raw histograms. Should be in units of your raw histogram (most likely charge).
	- twopeaks: a boolean variable that tells the code to expect two equally strong large peaks. To use, set to true and then declare largepeak2.
	- largepeak2: if there are two nearly equally large peaks (say in 60Co), set this value to the value of the second largest peak. If you use a rough energy calibration, this should be in keV. Otherwise, it may be in charge units.
	- calFile: the name of your output calibration file.
	- xbins_energy: the number of bins used in creating the calibrated energy histograms.
	- xmin_energy: the minimum bound on the charge axis in the calibrated energy histograms.
	- xmax_energy: the maximum bound on the charge axis in the calibrated energy histograms.
	- peaks to check: a list of peaks (again, in keV if you'd like a rough energy calibration) that will be checked to evaluate the quality of the fit. Does not have to be the same as the fittingpeaks vector, but may.
	- windowsize: the +/0 value that sets the search window when looking for peaks in the calibrated histograms. If you have requested a rough energy calibration, should be in keV.
	- do_make_calibration_histograms, do_create_gainmatch_graphs, etc.: these Booleans turn on and off the smaller functions within the code. It is expected that the user will not want to execute all of the functions at all times. As an example, the histograms that are going to be used for calibration (raw "Charge" spectra) are likely to only be created once. If the inital gain-match is unsatisfactory, the user can set do_make_calibration_histograms to false and recompile the macros for faster running.

More details on the smaller functions:
make_calibration_histograms: 
This function takes in the FragmentTree ROOT file and outputs a ROOT file containing histograms for all channels between minchannel and maxchannel, excluding those in the channelstoskip vector. Limits are binning of the histograms are also defined in the function call.

create_gainmatch_graphs:
This function finds peaks in the uncalibrated spectra and correlates them with calibrated values. It takes in the ROOT file created in make_calibration_histograms that contains the uncalibrated spectra and adds graphs to that file. Those graphs show the uncalibrated peak value vs. the calibrated peak value for an individual crystal. The calibrated values are NOT set to the values given by the user in the fittingpeaks vector, but are instead set to the values of the peaks found in the first spectrum - the histogram associated with channel number minchannel. If a rough energy calibration is selected, a linear transformation is applied to convert the calibrated charge values to roughly calibrated energy values.

create_GRIFFIN_cal:
This function takes in the graphs created in create_gainmatch_graphs, linearly fits them, and creates a calibration file that may later be used to calibrate GRIFFIN data. It also outputs two diagnostic images: an overlay of all the graphs used, with points colored to represent the color of the crystal they show, and histograms showing the slope, offset, and chi^2 of each linear fit.

recalibrate_spectra:
This function takes in the original data file and the newly create calibration file. It outputs a new ROOT file with calibrated histograms.

check_calibration:
This function takes in the ROOT file with calibrated histograms that was created in the recalibrate_spectra function and measures the FWHM and FWTM of the peaks listed in the peakstocheck vector. It also sums all the channels together and measures the FWHM and FWTM of that summed histogram. It then outputs two diagnostic images: one comparing the distribution of FWHM among all the crystals to the FWHM of the summed spectrum and another comparing the distribution of the FWTM/FWHM ratio among all the crystals to the FWTM/FWHM ratio of the summed spetrcum. The distribution is displayed using the "candle" draw option in ROOT. The mean of the distribution is given with an open circle. The median is given by a solid horizontal line. The dashed line spans the full range of the data, from minimum to maximum values. The box shows the range of the middle 50% of the data; the limits are the first and third quartile lines. 25% of the data is outside and below the box, and 25% of the data is outside and above the box.

