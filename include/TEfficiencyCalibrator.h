#ifndef TEFFICIENCYCALIBRATOR_H__
#define TEFFICIENCYCALIBRATOR_H__

#if __cplusplus >= 201703L

#include <cstdarg>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

#include "TFile.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TLegend.h"
#include "TGFrame.h"
#include "TGTab.h"
#include "TGFSComboBox.h"
#include "TGStatusBar.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TGProgressBar.h"
#include "TRootEmbeddedCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TPaveText.h"
#include "RVersion.h"

#include "TNucleus.h"
#include "TPeakFitter.h"
#include "TSinglePeak.h"
#include "TCalibrationGraph.h"

/** \addtogroup Calibration
 *  @{
 */

class TEfficiencyCalibrator;
class TEfficiencySourceTab;

class TEfficiencyTab {
/////////////////////////////////////////////////////////////////
///
/// \class TEfficiencyTab
///
/// This class is the innermost tab with one type of data for one
/// source. It automatically calculates the summing in and summing
/// out for all peaks defined in the TNucleus object provided to it
/// (if it can fit them in the singles spectrum with a large enough
/// area). Parameters for it are the range of the fits, the minimum
/// threshold for the area to be acceptablem, and the background 
/// parameter.
///
/////////////////////////////////////////////////////////////////
public:
	enum EPeakType { kRWPeak = 0, kABPeak = 1, kAB3Peak = 2, kGauss = 3 };

   TEfficiencyTab(TEfficiencySourceTab* parent, TNucleus* nucleus, std::tuple<TH1*, TH2*, TH2*> hists, TGCompositeFrame* frame, const double& range, const double& threshold, const int& bgParam, const int& verboseLevel = 0);
   ~TEfficiencyTab();

	void FindPeaks();
	void FindPeaks(const double& range, const double& threshold, const int& bgParam) {
		fRange = range;
		fThreshold = threshold;
		fBgParam = bgParam;
		FindPeaks();
	}
   void Redraw();
   void MakeConnections();
   void Disconnect();
	void Status(Int_t event, Int_t px, Int_t py, TObject* selected);

   void VerboseLevel(int val) { fVerboseLevel = val; }

	// setters
	void Range(double val) { fRange = val; }
	void Threshold(double val) { fThreshold = val; }
	void BgParam(double val) { fBgParam = val; }

	// getters
	std::vector<std::tuple<TTransition*, double, double, double, double, double, double, double, double>> Peaks() const { return fPeaks; }
	const char* GetName() const { return fNucleus->GetName(); }

private:
	void BuildInterface();

   // graphic elements
	TGCompositeFrame*    fFrame{nullptr};
	TGHorizontalFrame*   fTopFrame{nullptr};
	TRootEmbeddedCanvas* fProjectionCanvas{nullptr};
	TGStatusBar*         fStatusBar{nullptr};

   // storage elements
   TNucleus* fNucleus; ///< the source nucleus
   TEfficiencySourceTab* fParent; ///< the parent of this tab
   TH1* fSingles{nullptr}; ///< the singles histogram we're using
   TH2* fSummingOut{nullptr}; ///< the (mixed) matrix we're using for summing out
   TH2* fSummingIn{nullptr}; ///< the sum matrix we're using for summing in
	double fRange{10.}; ///< range of the fit (+- range)
   double fThreshold{100.}; ///< the threshold (relative to the largest peak) under which peaks are ignored
   int fBgParam{20}; ///< the bg parameter used to determine the background in the gamma spectra
	TPeakFitter fPeakFitter;
	EPeakType fPeakType{EPeakType::kRWPeak};
	std::vector<TH1*> fSummingInProj;
	std::vector<TH1*> fSummingInProjBg;
	std::vector<TH1*> fSummingOutProj;
	TH1* fSummingOutTotalProj;
	TH1* fSummingOutTotalProjBg;
	std::vector<std::tuple<TTransition*, double, double, double, double, double, double, double, double>> fPeaks;
	std::vector<TObject*> fFitFunctions; ///< vector with all fits of the singles histogram
   int fVerboseLevel{0}; ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
};

class TEfficiencySourceTab {
/////////////////////////////////////////////////////////////////
///
/// \class TEfficiencySourceTab
///
/// This class is the outer tab with all the sources data for one
/// data type (single/addback, suppressed/unsuppressed). It creates
/// the tabs for each source and has some navigational buttons.
///
/////////////////////////////////////////////////////////////////
public:
	enum EEntry { kRangeEntry, kThresholdEntry, kBgParamEntry, kCalibrationUncertaintyEntry, kPeakTypeBox, kDegreeEntry, kPlotEfficiencyCheck, kPlotUncorrEfficiencyCheck, kPlotPeakAreaCheck, kPlotSummingInCheck, kPlotSummingOutCheck };

public:
   TEfficiencySourceTab(TEfficiencyCalibrator* parent, std::vector<TNucleus*> nucleus, std::vector<std::tuple<TH1*, TH2*, TH2*>> hists, TGCompositeFrame* frame, const std::string& dataType, const double& range, const double& threshold, TGHProgressBar* progressBar, const int& verboseLevel = 0);
   ~TEfficiencySourceTab();

	void CreateTabs();
   void MakeConnections();
   void Disconnect();

	TEfficiencyTab::EPeakType PeakType() { if(fPeakTypeBox == nullptr) return TEfficiencyTab::EPeakType::kRWPeak; if(fVerboseLevel > 3) { std::cout<<"peak type box "<<fPeakTypeBox<<std::flush<<", getting selected "<<fPeakTypeBox->GetSelected()<<std::endl; } return static_cast<TEfficiencyTab::EPeakType>(fPeakTypeBox->GetSelected()); }

   void VerboseLevel(int val) { fVerboseLevel = val; for(auto& tab : fEfficiencyTab) { tab->VerboseLevel(val); } }

	void Status(Int_t event, Int_t px, Int_t py, TObject* selected);
	void DrawGraph();
	void UpdateEfficiencyGraph();
	void FitEfficiency();
	void FittingControl(Int_t id);

	int Degree() { if(fDegreeEntry != nullptr) fDegree = fDegreeEntry->GetNumber(); return fDegree; }

	TCalibrationGraphSet* EfficiencyGraph() { return fEfficiencyGraph; }

	static double EfficiencyDebertin(double* x, double* par);
	static double EfficiencyRadware(double* x, double* par);
	static double EfficiencyPolynomial(double* x, double* par);

private:
	void ReadValues();

   // graphic elements
	TGCompositeFrame*            fFrame{nullptr};
	TGVerticalFrame*     fLeftFrame{nullptr}; ///< Left frame for the source tabs
	TGTab*                       fDataTab{nullptr}; ///< tab for channels
	std::vector<TEfficiencyTab*> fEfficiencyTab;
	TGGroupFrame*        fFittingParameterFrame{nullptr};
	TGLabel*             fRangeLabel{nullptr};
	TGNumberEntry*       fRangeEntry{nullptr};
	TGLabel*             fBgParamLabel{nullptr};
	TGNumberEntry*       fBgParamEntry{nullptr};
	TGLabel*             fThresholdLabel{nullptr};
	TGNumberEntry*       fThresholdEntry{nullptr};
	TGComboBox*          fPeakTypeBox{nullptr};
	TGHButtonGroup*      fFittingControlGroup{nullptr};
	TGTextButton*        fRefitButton{nullptr};
	TGTextButton*        fRefitAllButton{nullptr};
   TGVerticalFrame*     fRightFrame{nullptr}; ///< Right frame for the efficiency data and fit
	TRootEmbeddedCanvas* fEfficiencyCanvas{nullptr};
	TLegend*             fLegend{nullptr};
	TGStatusBar*         fStatusBar{nullptr};
	TGGroupFrame*        fGraphParameterFrame{nullptr};
	TGLabel*             fDegreeLabel{nullptr};
	TGNumberEntry*       fDegreeEntry{nullptr};
	TGLabel*             fCalibrationUncertaintyLabel{nullptr};
	TGNumberEntry*       fCalibrationUncertaintyEntry{nullptr};
	TGGroupFrame*        fPlotOptionFrame{nullptr};
	TGCheckButton*       fPlotEfficiencyCheck{nullptr};
	TGCheckButton*       fPlotUncorrEfficiencyCheck{nullptr};
	TGCheckButton*       fPlotPeakAreaCheck{nullptr};
	TGCheckButton*       fPlotSummingInCheck{nullptr};
	TGCheckButton*       fPlotSummingOutCheck{nullptr};
	TGTextButton*        fRecalculateButton{nullptr};

   // storage elements
	std::vector<TNucleus*> fNucleus; ///< the source nuclei
   TEfficiencyCalibrator* fParent; ///< the parent of this tab
	std::string fDataType; ///< data type of this tab
	double fRange{10.}; ///< range of the fit (+- range)
   double fThreshold{100.}; ///< the threshold (relative to the largest peak) under which peaks are ignored
   int fBgParam{20}; ///< the bg parameter used to determine the background in the gamma spectra
   int fVerboseLevel{0}; ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
	int fDegree{0}; ///< degree of fit function (0 = debertin form, 1 = radware form, everything else polynomial ln(e(E)) = sum i 0->8 a_i (ln(E))^i (Ryan's & Andrew's PhD theses)
	double fCalibrationUncertainty{1.}; ///< calibration uncertainty (peaks are rejected if the centroid and energy difference is larger than centroid and energy uncertainties plus this)
	TCalibrationGraphSet* fEfficiencyGraph{nullptr}; ///< the combined efficiency graph from all sources
	TCalibrationGraphSet* fUncorrEfficiencyGraph{nullptr}; ///< the combined uncorrected efficiency graph from all sources
	TCalibrationGraphSet* fPeakAreaGraph{nullptr}; ///< the combined peak area graph from all sources
	TCalibrationGraphSet* fSummingInGraph{nullptr}; ///< the combined summing in correction graph from all sources
	TCalibrationGraphSet* fSummingOutGraph{nullptr}; ///< the combined summing out correction graph from all sources
	TF1* fEfficiency{nullptr}; ///< fit of efficiency
	TDirectory* fMainDirectory{nullptr}; ///< main directory (should be the file we're writing to)
	TDirectory* fSubDirectory{nullptr}; ///< subdirectory this tab writes the graphs to
};

class TEfficiencyCalibrator : public TGMainFrame {
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

public:
	enum ESources { k22Na, k56Co, k60Co, k133Ba, k152Eu, k241Am	};
	enum EEntry { kStartButton, kSourceBox = 100, kSigmaEntry = 200, kThresholdEntry = 300 };

   TEfficiencyCalibrator(double range, double threshold, int n...);
	~TEfficiencyCalibrator();

	void SetSource(Int_t windowId, Int_t entryId);
	void Start();

	void LineHeight(const unsigned int& val) { fLineHeight = val; Resize(GetSize()); }

	using TGWindow::HandleTimer;
	void HandleTimer();
	void SecondWindow();

	static int LineHeight() { return fLineHeight; }
	static int WindowWidth() { return fWindowWidth; }

	void VerboseLevel(int val) { fVerboseLevel = val; }

	std::vector<TCalibrationGraphSet*> EfficiencyGraphs() { return fEfficiencyGraph; }
	size_t NumberOfEfficiencyGraphs() { return fEfficiencyGraph.size(); }
	TCalibrationGraphSet* EfficiencyGraph(size_t i) { return fEfficiencyGraph.at(i); }

private:
	void DeleteElement(TGFrame* element);
   void BuildFirstInterface();
   void MakeFirstConnections();
	void DisconnectFirst();
	void DeleteFirst();
   void BuildSecondInterface();
   void MakeSecondConnections();
	void DisconnectSecond();

	int fVerboseLevel{4}; ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
	double fRange{20.};
	double fThreshold{100.};
	std::vector<TFile*> fFiles;
	std::vector<TNucleus*> fSources;
	std::vector<std::string> fDataType; ///< type of each data set
	std::vector<std::vector<std::tuple<TH1*, TH2*, TH2*>>> fHistograms; ///< for each type of data (suppressed, addback) in the file a vector with three histograms for each source
	std::vector<TEfficiencySourceTab*> fEfficiencySourceTab;
	std::vector<TCalibrationGraphSet*> fEfficiencyGraph;
	TFile* fOutput{nullptr};

	TGTextButton*        fEmitter{nullptr};

	static unsigned int fLineHeight; ///< Height of text boxes and progress bar
	static unsigned int fWindowWidth; ///< Width of window

	// graphic elements
	std::vector<TGLabel*> fSourceLabel;
	std::vector<TGComboBox*> fSourceBox;
	TGTab*               fSourceTab{nullptr};
	TGHProgressBar*      fProgressBar{nullptr};
   TGTextButton*        fStartButton{nullptr};
	
   /// \cond CLASSIMP
   ClassDefOverride(TEfficiencyCalibrator, 1); // Class to determine efficiency calibrations
   /// \endcond
};
/*! @} */
#endif
#endif
