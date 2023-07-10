#ifndef TEFFICIENCYCALIBRATOR_H__
#define TEFFICIENCYCALIBRATOR_H__

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
	enum EPeakType { kRWPeak, kABPeak, kAB3Peak, kGauss };

   TEfficiencyTab(TEfficiencySourceTab* parent, TNucleus* nucleus, std::tuple<TH1*, TH2*, TH2*> hists, TGCompositeFrame* frame, const double& range, const double& threshold, const int& bgParam);
   ~TEfficiencyTab();

	void FindPeaks();
   void MakeConnections();
   void Disconnect();

   void VerboseLevel(int val) { fVerboseLevel = val; }

	// setters
	void Range(double val) { fRange = val; }
	void Threshold(double val) { fThreshold = val; }
	void BgParam(double val) { fBgParam = val; }

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
	std::vector<std::tuple<double, double, double, double, double, double, double, double, double, double>> fPeaks;
   int fVerboseLevel{0}; ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
};

class TEfficiencySourceTab {
/////////////////////////////////////////////////////////////////
///
/// \class TEfficiencySourceTab
///
/// This class is the outer tab with all the types of data for one
/// source (single/addback, suppressed/unsuppressed). It creates
/// the tabs for each data type and has some navigational buttons.
///
/////////////////////////////////////////////////////////////////
public:
	enum EEntry { kRangeEntry, kThresholdEntry, kBgParamEntry, kPeakTypeBox };

public:
   TEfficiencySourceTab(TEfficiencyCalibrator* parent, TNucleus* nucleus, std::vector<std::tuple<TH1*, TH2*, TH2*>> hists, TGCompositeFrame* frame, const double& range, const double& threshold, const int& bgParam, TGHProgressBar* progressBar);
   ~TEfficiencySourceTab();

	void CreateTabs();
   void MakeConnections();
   void Disconnect();

   void VerboseLevel(int val) { fVerboseLevel = val; for(auto& tab : fEfficiencyTab) { tab->VerboseLevel(val); } }

private:
   // graphic elements
	TGCompositeFrame*            fFrame{nullptr};
	TGTab*                       fDataTab{nullptr}; ///< tab for channels
	std::vector<TEfficiencyTab*> fEfficiencyTab;
	TGHProgressBar*              fProgressBar{nullptr};
	TGHButtonGroup*      fNavigationGroup{nullptr};
	TGTextButton*        fPreviousButton{nullptr};
	TGTextButton*        fNextButton{nullptr};
	TGGroupFrame*        fParameterFrame{nullptr};
	TGLabel*             fRangeLabel{nullptr};
	TGNumberEntry*       fRangeEntry{nullptr};
	TGLabel*             fBgParamLabel{nullptr};
	TGNumberEntry*       fBgParamEntry{nullptr};
	TGLabel*             fThresholdLabel{nullptr};
	TGNumberEntry*       fThresholdEntry{nullptr};
	TGComboBox*          fPeakTypeBox{nullptr};

   // storage elements
   TNucleus* fNucleus; ///< the source nucleus
   TEfficiencyCalibrator* fParent; ///< the parent of this tab
	double fRange{10.}; ///< range of the fit (+- range)
   double fThreshold{100.}; ///< the threshold (relative to the largest peak) under which peaks are ignored
   int fBgParam{20}; ///< the bg parameter used to determine the background in the gamma spectra
   int fVerboseLevel{0}; ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
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
public:
	enum ESources { k22Na, k56Co, k60Co, k133Ba, k152Eu, k241Am	};
	enum EEntry { kStartButton, kSourceBox = 100, kSigmaEntry = 200, kThresholdEntry = 300, kDegreeEntry = 400 };

   TEfficiencyCalibrator(double range, double threshold, int n...);
	~TEfficiencyCalibrator();

	void SetSource(Int_t windowId, Int_t entryId);
	void Start();

	int Degree() { if(fDegreeEntry != nullptr) fDegree = fDegreeEntry->GetNumber(); return fDegree; }

	void LineHeight(const unsigned int& val) { fLineHeight = val; Resize(GetSize()); }

	using TGWindow::HandleTimer;
	void HandleTimer();
	void SecondWindow();

	void VerboseLevel(int val) { fVerboseLevel = val; }

private:
	void DeleteElement(TGFrame* element);
   void BuildFirstInterface();
   void MakeFirstConnections();
	void DisconnectFirst();
	void DeleteFirst();
   void BuildSecondInterface();
   void MakeSecondConnections();
	void DisconnectSecond();
	void Navigate(Int_t id);
	void SelectedTab(Int_t id);

	int fVerboseLevel{0}; ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
	double fRange{20.};
	double fThreshold{100.};
   int fBgParam{20}; ///< the bg parameter used to determine the background in the gamma spectra
	std::vector<TFile*> fFiles;
	std::vector<std::vector<std::tuple<TH1*, TH2*, TH2*>>> fHistograms; ///< for each type of data (suppressed, addback) in the file a vector with three histograms for each source
	std::vector<TNucleus*> fSources;
	std::vector<TEfficiencySourceTab*> fEfficiencySourceTab;
	TFile* fOutput{nullptr};

	TGTextButton*        fEmitter{nullptr};

   int fDegree{1}; ///< degree of polynomial function used to calibrate

	unsigned int fLineHeight{20}; ///< Height of text boxes and progress bar

	// graphic elements
	std::vector<TGLabel*> fSourceLabel;
	std::vector<TGComboBox*> fSourceBox;
	TGVerticalFrame*     fLeftFrame{nullptr}; ///< Left frame for the source tabs
   TGVerticalFrame*     fRightFrame{nullptr}; ///< Right frame for the efficiency data and fit
	TRootEmbeddedCanvas* fEfficiencyCanvas{nullptr};
	TGStatusBar*         fStatusBar{nullptr};
	TGTab*               fSourceTab{nullptr};
	TGHProgressBar*      fProgressBar{nullptr};
	TGNumberEntry*       fDegreeEntry{nullptr};
	TGLabel*             fDegreeLabel{nullptr};
   TGTextButton*        fStartButton{nullptr};
	
   /// \cond CLASSIMP
   ClassDefOverride(TEfficiencyCalibrator, 1); // Class to determine efficiency calibrations
   /// \endcond
};
/*! @} */
#endif
