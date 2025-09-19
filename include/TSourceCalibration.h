#ifndef TSOURCECALIBRATION_H
#define TSOURCECALIBRATION_H

/** \addtogroup Calibration
 *  @{
 */

#include <cstdarg>
#include <vector>
#include <string>
#include <mutex>
#include <future>
#include <queue>

#include "TFile.h"
#include "TPad.h"
#include "TLegend.h"
#include "TGFrame.h"
#include "TGTab.h"
#include "TGStatusBar.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TGProgressBar.h"
#include "TGComboBox.h"
#include "TRootEmbeddedCanvas.h"
#include "TH2.h"
#include "TPaveText.h"

#include "TPeakFitter.h"
#include "TGauss.h"
#include "TSinglePeak.h"
#include "TNucleus.h"
#include "TCalibrationGraph.h"
#include "GH1D.h"
#include "Globals.h"
#include "TRedirect.h"

class TSourceTab;

std::map<double, std::tuple<double, double, double, double>>  RoughCal(std::vector<double> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab);
std::map<TGauss*, std::tuple<double, double, double, double>> Match(std::vector<TGauss*> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab);
std::map<TGauss*, std::tuple<double, double, double, double>> SmartMatch(std::vector<TGauss*> peaks, std::vector<std::tuple<double, double, double, double>> sources, TSourceTab* sourceTab);

double Polynomial(double* x, double* par);

bool FilledBin(TH2* matrix, const int& bin);

class TSourceCalibration;

class TChannelTab;

class TSourceTab {
public:
   TSourceTab(TSourceCalibration* sourceCal, TChannelTab* channel, TGCompositeFrame* frame, GH1D* projection, const char* sourceName, std::vector<std::tuple<double, double, double, double>> sourceEnergy);
   TSourceTab(const TSourceTab& rhs);
   TSourceTab(TSourceTab&&) noexcept            = default;
   TSourceTab& operator=(const TSourceTab&)     = default;
   TSourceTab& operator=(TSourceTab&&) noexcept = default;
   ~TSourceTab();

   void MakeConnections();
   void Disconnect();

   void ProjectionStatus(Event_t* event);
   void ProjectionStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   void InitialCalibration(const bool& force);
   void Add(std::map<double, std::tuple<double, double, double, double>> map);
   void Add(std::map<TGauss*, std::tuple<double, double, double, double>> map);
   void FindCalibratedPeaks(const TF1* calibration);
   void ReplacePeak(const size_t& index, const double& channel);

   void SourceEnergy(std::vector<std::tuple<double, double, double, double>> val) { fSourceEnergy = std::move(val); }

   void Status(const char* status, int position);

   TGraphErrors*        Data() const { return fData; }
   TGraphErrors*        Fwhm() const { return fFwhm; }
   TRootEmbeddedCanvas* ProjectionCanvas() const { return fProjectionCanvas; }
   const char*          SourceName() const { return fSourceName; }

   void RemovePoint(Int_t oldPoint);

   void Print() const;
   void PrintLayout() const;

   void PrintCanvases() const;

   void Draw();

private:
   void BuildInterface();
   void UpdateRegions();
   bool Good(TGauss* peak);
   bool Good(TGauss* peak, double lowRange, double highRange) { return Good(peak) && lowRange < peak->Centroid() && peak->Centroid() < highRange; }
   void UpdateFits();
   void SetLineColors();

   // parents
   TSourceCalibration* fSourceCalibration{nullptr};
   TChannelTab*        fChannelTab{nullptr};

   // graphic elements
   TGCompositeFrame*    fSourceFrame{nullptr};
   TRootEmbeddedCanvas* fProjectionCanvas{nullptr};
   TGStatusBar*         fSourceStatusBar{nullptr};

   // storage elements
   GH1D*                                                   fProjection{nullptr};
   const char*                                             fSourceName;
   TGraphErrors*                                           fData{nullptr};
   TGraphErrors*                                           fFwhm{nullptr};
   std::vector<TPeakFitter*>                               fFits;           ///< all good fits
   std::vector<TPeakFitter*>                               fBadFits;        ///< all bad fits (centroid err > 10%, area err > 100%, or either of them not a number at all)
   std::vector<TGauss*>                                    fPeaks;          ///< all peaks that have been fitted and are good
   std::vector<std::tuple<double, double, double, double>> fSourceEnergy;   ///< gamma rays from the source, with their energies, uncertainties in the energies, intensities, and uncertainties in the intesities
   std::vector<std::pair<double, double>>                  fRegions;
};

class TChannelTab {
public:
   TChannelTab(TSourceCalibration* sourceCal, std::vector<TNucleus*> nuclei, std::vector<GH1D*> projections, std::string name, TGCompositeFrame* frame, std::vector<std::vector<std::tuple<double, double, double, double>>> sourceEnergies, TGHProgressBar* progressBar);
   TChannelTab(const TChannelTab&)                = default;
   TChannelTab(TChannelTab&&) noexcept            = default;
   TChannelTab& operator=(const TChannelTab&)     = default;
   TChannelTab& operator=(TChannelTab&&) noexcept = default;
   ~TChannelTab();

   void MakeConnections();
   void Disconnect();
   void Initialize(const bool& force);

   void CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);
   void SelectCanvas(Event_t* event);
   void RemovePoint(Int_t oldGraph, Int_t oldPoint);

   void          UpdateData();
   void          UpdateFwhm();
   void          UpdateChannel();
   void          SelectedTab(Int_t id);
   void          Write(TFile* output);
   void          Calibrate();
   void          Calibrate(int degree);
   void          RecursiveRemove(const double& maxResidual);
   void          FindCalibratedPeaks();
   void          FindAllCalibratedPeaks();
   TGTab*        SourceTab() const { return fSourceTab; }
   TGraphErrors* Data(int channelId) const { return fSources[channelId]->Data(); }
   size_t        NumberOfSources() const { return fSources.size(); }
   std::string   Name() const { return fName; }
   int           ActiveSourceTab() const { return fActiveSourceTab; }
   TSourceTab*   SelectedSourceTab() const { return fSources[fActiveSourceTab]; }

   //TSourceCalibration* SourceCalibration() const { return fSourceCalibration; }

   static void ZoomX();
   static void ZoomY();

   void PrintLayout() const;

   void PrintCanvases() const;

   void Draw();

private:
   void BuildInterface();
   void CreateSourceTab(size_t source);

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
   TPaveText*               fCalLabel{nullptr};
   TPaveText*               fInfoLabel{nullptr};
   TGCompositeFrame*        fFwhmFrame{nullptr};   ///< frame of tab with fwhm
   TRootEmbeddedCanvas*     fFwhmCanvas{nullptr};
   TGCompositeFrame*        fInitFrame{nullptr};   ///< frame of tab with initial calibration
   TRootEmbeddedCanvas*     fInitCanvas{nullptr};
   TGHProgressBar*          fProgressBar{nullptr};

   // storage elements
   TSourceCalibration*                                                  fSourceCalibration;    ///< the parent of this tab
   std::vector<TNucleus*>                                               fNuclei;               ///< the source nucleus
   std::vector<GH1D*>                                                   fProjections;          ///< vector of all projections
   std::string                                                          fName;                 ///< name of this tab
   std::vector<std::vector<std::tuple<double, double, double, double>>> fSourceEnergies;       ///< vector with source energies and uncertainties
   TCalibrationGraphSet*                                                fData{nullptr};        ///< combined data from all sources
   TCalibrationGraphSet*                                                fFwhm{nullptr};        ///< combined fwhm from all sources
   TCalibrationGraphSet*                                                fInit{nullptr};        ///< combined initial data from all sources
   int                                                                  fActiveSourceTab{0};   ///< id of the currently active source tab
};

class TSourceCalibration : public TGMainFrame {
public:
   TSourceCalibration(double sigma, double threshold, int degree, int count...);
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
   void Remove();
   using TObject::RecursiveRemove;
   void RecursiveRemove();
   void FindSourcePeaks();
   void FindChannelPeaks();
   void FindAllPeaks();
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

   std::mutex& GraphicsMutex() { return fGraphicsMutex; }

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
   static void NumberOfThreads(size_t val) { fNumberOfThreads = val; }

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

   static void   MinIntensity(double val) { fMinIntensity = val; }
   static double MinIntensity() { return fMinIntensity; }

   static void BadBins(const std::vector<int>& val) { fBadBins = val; }
   static void BadBin(const int& val) { fBadBins.push_back(val); }
   static void ResetBadBins() { fBadBins.clear(); }

   static void ZoomX();

   void PrintLayout() const;

   void PrintCanvases() const;

private:
   enum EEntry : std::uint8_t {
      kStartButton         = 0,
      kSourceBox           = 10,
      kSigmaEntry          = 20,
      kThresholdEntry      = 30,
      kDegreeEntry         = 40,
      kMaxResidualEntry    = 50,
      kApplyToAll          = 60,
      kWriteNonlinearities = 70
   };
   // the numbering of these two enums needs to match the order in which the buttons are created
   enum ENavigate : std::uint8_t {
      kPrevious  = 1,
      kDiscard   = 2,
      kAccept    = 3,
      kAcceptAll = 4,
      kWrite     = 5,
      kNext      = 6
   };
   enum EFitting : std::uint8_t {
      kFindSourcePeaks  = 1,
      kFindChannelPeaks = 2,
      kFindAllPeaks     = 3,
      kCalibrate        = 4,
      kRecursiveRemove  = 5
   };

   void              BuildFirstInterface();
   void              MakeFirstConnections();
   void              DisconnectFirst();
   void              DeleteFirst();
   void              BuildSecondInterface();
   void              MakeSecondConnections();
   void              AcceptChannel(const int& channelId = -1);
   void              DisconnectSecond();
   void              DeleteSecond();
   void              BuildThirdInterface();
   void              MakeThirdConnections();
   void              AcceptFinalChannel(const int& channelId = -1);
   void              DisconnectThird();
   void              DeleteElement(TGFrame* element);
   TGTransientFrame* CreateProgressbar(const char* format);

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
   TGTextButton*   fFindAllPeaksButton{nullptr};
   TGTextButton*   fFindChannelPeaksButton{nullptr};
   TGTextButton*   fFindSourcePeaksButton{nullptr};
   TGTextButton*   fRecursiveRemoveButton{nullptr};
   TGGroupFrame*   fParameterFrame{nullptr};
   TGLabel*        fSigmaLabel{nullptr};
   TGNumberEntry*  fSigmaEntry{nullptr};
   TGLabel*        fThresholdLabel{nullptr};
   TGNumberEntry*  fThresholdEntry{nullptr};
   TGLabel*        fDegreeLabel{nullptr};
   TGNumberEntry*  fDegreeEntry{nullptr};
   TGLabel*        fMaxResidualLabel{nullptr};
   TGNumberEntry*  fMaxResidualEntry{nullptr};
   TGGroupFrame*   fSettingsFrame{nullptr};
   TGCheckButton*  fApplyToAll{nullptr};
   TGCheckButton*  fWriteNonlinearities{nullptr};
   TGHProgressBar* fProgressBar{nullptr};

   std::vector<TGLabel*>                                                fMatrixNames;
   std::vector<TGComboBox*>                                             fSourceBox;
   std::vector<TNucleus*>                                               fSource;
   std::vector<std::vector<std::tuple<double, double, double, double>>> fSourceEnergy;   ///< vector to hold source energy, energy uncertainty, intensity, and intensity uncertainty
   std::vector<int>                                                     fActiveBins;
   std::vector<int>                                                     fActualChannelId;
   std::vector<const char*>                                             fChannelLabel;
   std::queue<std::pair<int, std::future<TChannelTab*>>>                fFutures;

   std::vector<TH2*> fMatrices;
   int               fNofBins{0};   ///< Number of filled bins in first matrix

   static std::string fLogFile;             ///< name of log file, if empty no log file is written
   TRedirect*         fRedirect{nullptr};   ///< redirect, created in constructor and destroyed in destructor if a log file name has been provided

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

   double                  fDefaultSigma{2.};         ///< The default sigma used for the peak finding algorithm, can be changed later.
   double                  fDefaultThreshold{0.05};   ///< The default threshold used for the peak finding algorithm, can be changed later. Co-56 source needs a much lower threshold, 0.01 or 0.02, but that makes it much slower too.
   int                     fDefaultDegree{1};         ///< The default degree of the polynomial used for calibrating, can be changed later.
   double                  fDefaultMaxResidual{2.};   ///< The default maximum residual accepted when trying to iteratively find peaks
   static int              fMaxIterations;            ///< Maximum iterations over combinations in Match and SmartMatch
   static int              fFitRange;                 ///< range in sigma used for fitting peaks (peak position - range to peas position + range)
   static bool             fAcceptBadFits;            ///< Do we accept peaks where the fit was bad (position or area uncertainties too large or not numbers)
   static bool             fFast;                     ///< Do we use the fast peak finding method on startup or not.
   static bool             fUseCalibratedPeaks;       ///< Do we use the initial calibration to find more peaks in the source spectra?
   static double           fMinIntensity;             ///< Minimum intensity of source peaks to be considered when trying to find them in the spectrum.
   static size_t           fNumberOfThreads;          ///< Maximum number of threads to run while creating the channel tabs
   static std::vector<int> fBadBins;                  ///< Bins of the 2D matrix to be ignored even if there are enough counts in them

   std::mutex fGraphicsMutex;   ///< mutex to lock changes to graphics

   TFile* fOutput{nullptr};

   static EVerbosity fVerboseLevel;   ///< Changes verbosity from kQuiet to kAll

   /// \cond CLASSIMP
   ClassDefOverride(TSourceCalibration, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
