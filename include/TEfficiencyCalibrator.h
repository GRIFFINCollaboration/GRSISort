#ifndef TEFFICIENCYCALIBRATOR_H__
#define TEFFICIENCYCALIBRATOR_H__

/** \addtogroup Calibration
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TEfficiencyCalibrator
///
/// This is a class that determines the efficiency from source
/// data.
/// It expects a list of files with a 1D singles histogram and time random
/// corrected 2D histograms of energy vs. (suppressed) energy and
/// energy vs. sum of energies. In case of suppressed data only the
/// x-axis of the former 2D histogram should use suppressed data,
/// the y-axis needs to be unsuppressed data!
/// If the file name contains a source isotope at the beginning of 
/// the name, the source selection will be automatic, otherwise
/// a window will pop up with the option to select the isotope for
/// each file.
///
/// The efficiency curves can be:
/// e(E) = ln E + (ln E)/E + (ln E)^2/E + (ln E)^3/E + (ln E)^4/E,
/// or the radware form
/// ln(e(E)) = ((a1 + a2 x + a3 x^2)^-a7 + (a4 + a5 y + a6 y^2)^-a7)^-1/a7
/// with x = ln(E/100), y = ln(E/1000)
/// or a polynomial ln(e(E)) = sum i 0->8 a_i (ln(E))^i (Ryan's & Andrew's PhD theses)
///
/////////////////////////////////////////////////////////////////

class TEfficiencyTab {
public:
	enum EPeakType { kRWPeak, kABPeak, kAB3Peak, kGaussian };

   TEfficiencyTab(TSourceCalibration* parent, TNucleus* nucleus, std::tuple<TH1*, TH2*, TH2*> hists, TGCompositeFrame* frame, const double& sigma, const double& threshold, const int& degree, TGHProgressBar* progressBar);
   ~TEfficiencyTab();

	void FindPeaks();
   void MakeConnections();
   void Disconnect();

   void VerboseLevel(int val) { fVerboseLevel = val; for(auto channel : fChannel) channel->VerboseLevel(val); }

private:
   // graphic elements
   TGCompositeFrame* fFrame{nullptr}; ///< main frame of this tab
   TGHProgressBar*      fProgressBar{nullptr};

   // storage elements
   TNucleus* fNucleus; ///< the source nucleus
   TSourceCalibration* fParent; ///< the parent of this tab
   TH1* fSingles{nullptr}; ///< the singles histogram we're using
   TH2* fMatrix{nullptr}; ///< the (mixed) matrix we're using
   TH2* fSumMatrix{nullptr}; ///< the sum matrix we're using
   double fSigma{2.}; ///< the sigma used in the peak finder
   double fThreshold{0.05}; ///< the threshold (relative to the largest peak) used in the peak finder
   int fDegree{1}; ///< degree of polynomial function used to calibrate
	TPeakFitter fPeakFitter;
	EPeakType fPeakType{EPeakType::kRWPeak};
	std::vector<TH1*> fSummingInProj;
	std::vector<TH1*> fSummingInProjBg;
	std::vector<TH1*> fSummingOutProj;
	TH1* fSummingOutTotalProj;
	TH1* fSummingOutTotalProjBg;
	std::tuple<double, double, double, double, double, double, double, double, double, double> fPeaks;
   int fVerboseLevel{0}; ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
};

class TEfficiencyCalibrator : public TGMainFrame {
public:
   TEfficiencyCalibrator(double sigma, double threshold, int n...);
	~TEfficiencyCalibrator();

private:
   void BuildFirstInterface();
   void MakeFirstConnections();

	double fSigma;
	double fThreshold;
	std::vector<TFile*> fFiles;
	std::vector<std::vector<std::tuple<TH1*, TH2*, TH2*>>> fHistograms; ///< for each type of data (suppressed, addback) in the file a vector with three histograms for each source
	std::vector<TNucleus*> fSources;

	// graphic elements
	TGVerticalFrame*     fLeftFrame{nullptr}; ///< Left frame for the source tabs
   TGVerticalFrame*     fRightFrame{nullptr}; ///< Right frame for the efficiency data and fit
	TRootEmbeddedCanvas* fEfficiencyCanvas{nullptr};
	TGStatusBar*         fStatusBar{nullptr};
	TGTab*               fSourceTab{nullptr};
	TGHProgressBar*      fProgressBar{nullptr};
	TGNumberEntry*       fDegreeEntry{nullptr};
	TGLabel*             fDegreeLabel{nullptr};
	
   /// \cond CLASSIMP
   ClassDefOverride(TEfficiencyCalibrator, 1); // Class to determine efficiency calibrations
   /// \endcond
};
/*! @} */
#endif
