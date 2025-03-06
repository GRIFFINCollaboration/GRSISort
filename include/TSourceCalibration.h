#ifndef TSOURCECALIBRATION_H
#define TSOURCECALIBRATION_H

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
#include "TH2.h"
#include "TPaveText.h"
#include "RVersion.h"

#include "TPeakFitter.h"
#include "TGauss.h"
#include "TSinglePeak.h"
#include "TNucleus.h"
#include "TCalibrationGraph.h"
#include "GH1D.h"
#include "Globals.h"
#include "TRedirect.h"

class TSourceTab;

std::map<TGauss*, std::tuple<double, double, double, double>> Match(std::vector<TGauss*> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab);
std::map<TGauss*, std::tuple<double, double, double, double>> SmartMatch(std::vector<TGauss*> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab);

double Polynomial(double* x, double* par);

bool FilledBin(TH2* matrix, const int& bin);

class TSourceCalibration;

class TChannelTab;

class TSourceTab {
public:
   TSourceTab(TChannelTab* parent, TGCompositeFrame* frame, GH1D* projection, const double& sigma, const double& threshold, const double& peakRatio, std::vector<std::tuple<double, double, double, double>> sourceEnergy);
   TSourceTab(const TSourceTab& rhs);
   TSourceTab(TSourceTab&&) noexcept            = default;
   TSourceTab& operator=(const TSourceTab&)     = default;
   TSourceTab& operator=(TSourceTab&&) noexcept = default;
   ~TSourceTab();

   void MakeConnections();
   void Disconnect();

   void ProjectionStatus(Event_t* event);
   void ProjectionStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   void Add(std::map<TGauss*, std::tuple<double, double, double, double>> map);
   void FindPeaks(const double& sigma, const double& threshold, const double& peakRatio, const bool& force = false, const bool& fast = true);
   void FindCalibratedPeaks(const TF1* calibration);
	void ReplacePeak(const size_t& index, const double& channel);

   void SourceEnergy(std::vector<std::tuple<double, double, double, double>> val) { fSourceEnergy = std::move(val); }

   void Status(const char* status, int position);

   TGraphErrors*        Data() const { return fData; }
   TGraphErrors*        Fwhm() const { return fFwhm; }
   TRootEmbeddedCanvas* ProjectionCanvas() const { return fProjectionCanvas; }

   void RemovePoint(Int_t oldPoint);

   void Print() const;
   void PrintLayout() const;

private:
   void BuildInterface();
   void UpdateRegions();
	bool Good(TGauss* peak);
	void UpdateFits();

   // parent
   TChannelTab* fParent{nullptr};

   // graphic elements
   TGCompositeFrame*    fSourceFrame{nullptr};
   TRootEmbeddedCanvas* fProjectionCanvas{nullptr};
   TGStatusBar*         fSourceStatusBar{nullptr};

   // storage elements
   GH1D*                                                   fProjection{nullptr};
   TGraphErrors*                                           fData{nullptr};
   TGraphErrors*                                           fFwhm{nullptr};
   double                                                  fSigma{2.};
   double                                                  fThreshold{0.05};
   double                                                  fPeakRatio{2.};
	std::vector<TPeakFitter*>                               fFits;                 ///< all good fits
	std::vector<TPeakFitter*>                               fBadFits;              ///< all bad fits (centroid err > 10%, area err > 100%, or either of them not a number at all)
	std::vector<TGauss*>                                    fPeaks;                ///< all peaks that have been fitted and are good
   std::vector<std::tuple<double, double, double, double>> fSourceEnergy;         ///< gamma rays from the source, with their energies, uncertainties in the energies, intensities, and uncertainties in the intesities
   std::vector<std::pair<double, double>>                  fRegions;
};

class TChannelTab {
public:
   TChannelTab(TSourceCalibration* parent, std::vector<TNucleus*> nuclei, std::vector<GH1D*> projections, std::string name, TGCompositeFrame* frame, double sigma, double threshold, int degree, std::vector<std::vector<std::tuple<double, double, double, double>>> sourceEnergies, TGHProgressBar* progressBar);
   TChannelTab(const TChannelTab&)                = default;
   TChannelTab(TChannelTab&&) noexcept            = default;
   TChannelTab& operator=(const TChannelTab&)     = default;
   TChannelTab& operator=(TChannelTab&&) noexcept = default;
   ~TChannelTab();

   void CreateSourceTab(size_t source);

   void MakeConnections();
   void Disconnect();

   void CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);
   void SelectCanvas(Event_t* event);
   void RemovePoint(Int_t oldGraph, Int_t oldPoint);

   void          UpdateData();
   void          UpdateFwhm();
   void          UpdateChannel();
   void          SelectedTab(Int_t id);
   void          Write(TFile* output);
   void          Calibrate();
   void          Calibrate(const int& degree, const bool& force = false);
	void          Iterate(const double& maxResidual);
   void          FindPeaks(const double& sigma, const double& threshold, const double& peakRatio, const bool& force = false, const bool& fast = true);
   void          FindCalibratedPeaks();
   TGTab*        SourceTab() const { return fSourceTab; }
   TGraphErrors* Data(int channelId) const { return fSources[channelId]->Data(); }
   size_t        NumberOfSources() const { return fSources.size(); }
   std::string   Name() const { return fName; }
   int           ActiveSourceTab() const { return fActiveSourceTab; }
   TSourceTab*   SelectedSourceTab() const { return fSources[fActiveSourceTab]; };

   static void ZoomX();
   static void ZoomY();

   void PrintLayout() const;

private:
   // graphic elements
   TGCompositeFrame*        fChannelFrame{nullptr};       ///< main frame of this tab
   TGTab*                   fSourceTab{nullptr};          ///< tab for sources
   std::vector<TSourceTab*> fSources;                     ///< tabs for all sources
   TGTab*                   fCanvasTab{nullptr};          ///< tab for canvases (calibration with residuals, and FWHM)
   TGCompositeFrame*        fCalibrationFrame{nullptr};   ///< frame of tab with calibration
   TRootEmbeddedCanvas*     fCalibrationCanvas{nullptr};
   TPad*                    fResidualPad{nullptr};
   TPad*                    fCalibrationPad{nullptr};
   TGStatusBar*             fChannelStatusBar{nullptr};
   TLegend*                 fLegend{nullptr};
   TPaveText*               fChi2Label{nullptr};
   TGCompositeFrame*        fFwhmFrame{nullptr};   ///< frame of tab with fwhm
   TRootEmbeddedCanvas*     fFwhmCanvas{nullptr};
   TGHProgressBar*          fProgressBar{nullptr};

   // storage elements
   TSourceCalibration*                                                  fParent;               ///< the parent of this tab
   std::vector<TNucleus*>                                               fNuclei;               ///< the source nucleus
   std::vector<GH1D*>                                                   fProjections;          ///< vector of all projections
   std::string                                                          fName;                 ///< name of this tab
   double                                                               fSigma{2.};            ///< the sigma used in the peak finder
   double                                                               fThreshold{0.05};      ///< the threshold (relative to the largest peak) used in the peak finder
   int                                                                  fDegree{1};            ///< degree of polynomial function used to calibrate
   std::vector<std::vector<std::tuple<double, double, double, double>>> fSourceEnergies;       ///< vector with source energies and uncertainties
   TCalibrationGraphSet*                                                fData{nullptr};        ///< combined data from all sources
   TCalibrationGraphSet*                                                fFwhm{nullptr};        ///< combined fwhm from all sources
   int                                                                  fActiveSourceTab{0};   ///< id of the currently active source tab
};

class TSourceCalibration : public TGMainFrame {
public:
   TSourceCalibration(double sigma, double threshold, int degree, double peakRatio, int count...);
   TSourceCalibration(const TSourceCalibration&)                = delete;
   TSourceCalibration(TSourceCalibration&&) noexcept            = delete;
   TSourceCalibration& operator=(const TSourceCalibration&)     = delete;
   TSourceCalibration& operator=(TSourceCalibration&&) noexcept = delete;
   ~TSourceCalibration();

   void SetSource(Int_t windowId, Int_t entryId);
   void Start();

   void LineHeight(const unsigned int& val)
   {
      fLineHeight = val;
      Resize(GetSize());
   }

   void Navigate(Int_t id);
   void Fitting(Int_t id);
   void Calibrate();
	void Iterate();
   void FindPeaks();
   void FindPeaksFast();
   void FindCalibratedPeaks();
   void FindAllCalibratedPeaks();
   void SelectedTab(Int_t id);

   void NavigateFinal(Int_t id);
   void FitFinal(const int& channelId);
   void SelectedFinalTab(Int_t id);
   void SelectedFinalMainTab(Int_t id);
   void UpdateChannel(const int& channelId);
   void WriteCalibration();

   double Sigma() { return fSigmaEntry->GetNumber(); }
   double Threshold() { return fThresholdEntry->GetNumber(); }
   int    Degree()
   {
      if(fDegreeEntry != nullptr) { fDefaultDegree = static_cast<int>(fDegreeEntry->GetNumber()); }
      return fDefaultDegree;
   }
   double PeakRatio()
   {
      if(fPeakRatioEntry != nullptr) { fDefaultPeakRatio = fPeakRatioEntry->GetNumber(); }
      return fDefaultPeakRatio;
   }
   double MaxResidual()
   {
      if(fMaxResidualEntry != nullptr) { fDefaultMaxResidual = fMaxResidualEntry->GetNumber(); }
      return fDefaultMaxResidual;
   }
   bool                                                    WriteNonlinearities() { return fWriteNonlinearities->IsDown(); }
   std::vector<std::tuple<double, double, double, double>> SourceEnergy(const size_t& i) { return fSourceEnergy.at(i); }
   void                                                    CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   using TGWindow::HandleTimer;
   void HandleTimer();
   void SecondWindow();
   void FinalWindow();

   static void PanelWidth(int val) { fPanelWidth = val; }
   static void PanelHeight(int val) { fPanelHeight = val; }
   static void StatusbarHeight(int val) { fStatusbarHeight = val; }

   static int PanelWidth() { return fPanelWidth; }
   static int PanelHeight() { return fPanelHeight; }
   static int StatusbarHeight() { return fStatusbarHeight; }

   // no getters for these as they are only used in this class
   static void ParameterHeight(int val) { fParameterHeight = val; }
   static void SourceboxWidth(int val) { fSourceboxWidth = val; }
   static void DigitWidth(int val) { fDigitWidth = val; }

   static void       VerboseLevel(EVerbosity val) { fVerboseLevel = val; }
   static EVerbosity VerboseLevel() { return fVerboseLevel; }

   static void        LogFile(std::string val) { fLogFile = std::move(val); }
   static std::string LogFile() { return fLogFile; }

   static void MaxIterations(int val) { fMaxIterations = val; }
   static int  MaxIterations() { return fMaxIterations; }

   static void   FitRange(int val) { fFitRange = val; }
   static double FitRange() { return fFitRange; }

   static void AcceptBadFits(bool val) { fAcceptBadFits = val; }
   static bool AcceptBadFits() { return fAcceptBadFits; }

   static void Fast(bool val) { fFast = val; }
   static bool Fast() { return fFast; }

   static void UseCalibratedPeaks(bool val) { fUseCalibratedPeaks = val; }
   static bool UseCalibratedPeaks() { return fUseCalibratedPeaks; }

   static void MinIntensity(double val) { fMinIntensity = val; }
   static double MinIntensity() { return fMinIntensity; }

   static void ZoomX();

   void PrintLayout() const;

private:
   enum EEntry : int {
      kStartButton,
      kSourceBox           = 100,
      kSigmaEntry          = 200,
      kThresholdEntry      = 300,
      kDegreeEntry         = 400,
      kPeakRatioEntry      = 500,
      kMaxResidualEntry    = 600,
      kWriteNonlinearities = 700
   };
	// the numbering of these two enums needs to match the order in which the buttons are created
   enum ENavigate : int {
      kPrevious      = 1,
      kDiscard       = 2,
      kAccept        = 3,
      kAcceptAll     = 4,
      kWrite         = 5,
      kNext          = 6
   };
   enum EFitting : int {
      kFindPeaks       = 1,
      kFindPeaksFast   = 2,
      kFindPeaksCal    = 3,
      kFindPeaksCalAll = 4,
      kIterate         = 5,
      kCalibrate       = 6
   };

   void BuildFirstInterface();
   void MakeFirstConnections();
   void DisconnectFirst();
   void DeleteFirst();
   void BuildSecondInterface();
   void MakeSecondConnections();
   void AcceptChannel(const int& channelId = -1);
   void DisconnectSecond();
   void DeleteSecond();
   void BuildThirdInterface();
   void MakeThirdConnections();
   void AcceptFinalChannel(const int& channelId = -1);
   void DisconnectThird();
   void DeleteElement(TGFrame* element);

   //TGHorizontalFrame* fTopFrame{nullptr};
   TGHorizontalFrame*        fBottomFrame{nullptr};
   TGVerticalFrame*          fLeftFrame{nullptr};
   TGVerticalFrame*          fRightFrame{nullptr};
   TGTextButton*             fStartButton{nullptr};
   TGTab*                    fTab{nullptr};
   std::vector<TChannelTab*> fChannelTab;

   TGTextButton*   fEmitter{nullptr};
   TGHButtonGroup* fNavigationGroup{nullptr};
   TGTextButton*   fPreviousButton{nullptr};
   TGTextButton*   fDiscardButton{nullptr};
   TGTextButton*   fAcceptButton{nullptr};
   TGTextButton*   fAcceptAllButton{nullptr};
   TGTextButton*   fWriteButton{nullptr};
   TGTextButton*   fNextButton{nullptr};
   TGHButtonGroup* fFittingGroup{nullptr};
   TGTextButton*   fCalibrateButton{nullptr};
   TGTextButton*   fIterateButton{nullptr};
   TGTextButton*   fFindPeaksButton{nullptr};
   TGTextButton*   fFindPeaksFastButton{nullptr};
   TGTextButton*   fFindPeaksCalButton{nullptr};
   TGTextButton*   fFindPeaksCalAllButton{nullptr};
   TGGroupFrame*   fParameterFrame{nullptr};
   TGLabel*        fSigmaLabel{nullptr};
   TGNumberEntry*  fSigmaEntry{nullptr};
   TGLabel*        fThresholdLabel{nullptr};
   TGNumberEntry*  fThresholdEntry{nullptr};
   TGLabel*        fDegreeLabel{nullptr};
   TGNumberEntry*  fDegreeEntry{nullptr};
   TGLabel*        fPeakRatioLabel{nullptr};
   TGNumberEntry*  fPeakRatioEntry{nullptr};
   TGLabel*        fMaxResidualLabel{nullptr};
   TGNumberEntry*  fMaxResidualEntry{nullptr};
   TGCheckButton*  fWriteNonlinearities{nullptr};
   TGHProgressBar* fProgressBar{nullptr};

   std::vector<TGLabel*>                                                fMatrixNames;
   std::vector<TGComboBox*>                                             fSourceBox;
   std::vector<TNucleus*>                                               fSource;
   std::vector<std::vector<std::tuple<double, double, double, double>>> fSourceEnergy;   ///< vector to hold source energy, energy uncertainty, intensity, and intensity uncertainty
   std::vector<int>                                                     fActiveBins;
   std::vector<int>                                                     fActualChannelId;
   std::vector<const char*>                                             fChannelLabel;

   std::vector<TH2*> fMatrices;
   int               fNofBins{0};   ///< Number of filled bins in first matrix

   static std::string fLogFile;   ///< name of log file, if empty no log file is written
	TRedirect* fRedirect{nullptr}; ///< redirect, created in constructor and destroyed in destructor if a log file name has been provided

   // graphic settings
   unsigned int fLineHeight{20};    ///< Height of text boxes and progress bar
   static int   fPanelWidth;        ///< Width of one panel
   static int   fPanelHeight;       ///< Height of one panel
   static int   fStatusbarHeight;   ///< Height of status bar (also extra height needed for tabs)
   static int   fParameterHeight;   ///< Height of the frame for the parameters
   static int   fSourceboxWidth;    ///< Width of the box to select which source each histogram is from
   static int   fDigitWidth;        ///< Number of digits used for parameter entries (if they are floating point)

   int fWaitMs{100};   ///< How many milliseconds we wait before we process the navigation input (to avoid double triggers?)

   int fOldErrorLevel;   ///< Used to store old value of gErrorIgnoreLevel (set to kError for the scope of the class)

   double        fDefaultSigma{2.};         ///< The default sigma used for the peak finding algorithm, can be changed later.
   double        fDefaultThreshold{0.05};   ///< The default threshold used for the peak finding algorithm, can be changed later. Co-56 source needs a much lower threshold, 0.01 or 0.02, but that makes it much slower too.
   int           fDefaultDegree{1};         ///< The default degree of the polynomial used for calibrating, can be changed later.
   double        fDefaultPeakRatio{2.};     ///< The default ratio between found peaks and peaks in the source (per region).
   double        fDefaultMaxResidual{2.};   ///< The default maximum residual accepted when trying to iteratively find peaks
   static int    fMaxIterations;            ///< Maximum iterations over combinations in Match and SmartMatch
   static int    fFitRange;                 ///< range in sigma used for fitting peaks (peak position - range to peas position + range)
   static bool   fAcceptBadFits;            ///< Do we accept peaks where the fit was bad (position or area uncertainties too large or not numbers)
   static bool   fFast;                     ///< Do we use the fast peak finding method on startup or not.
   static bool   fUseCalibratedPeaks;       ///< Do we use the initial calibration to find more peaks in the source spectra?
   static double fMinIntensity;             ///< Minimum intensity of source peaks to be considered when trying to find them in the spectrum.

   TFile* fOutput{nullptr};

   static EVerbosity fVerboseLevel;   ///< Changes verbosity from kQuiet to kAll

   /// \cond CLASSIMP
   ClassDefOverride(TSourceCalibration, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

#endif
