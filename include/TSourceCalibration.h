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

#include "GPeak.h"
#include "TSinglePeak.h"
#include "TNucleus.h"
#include "TCalibrationGraph.h"

std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> Match(std::vector<std::tuple<double, double, double, double>> peaks, std::vector<std::tuple<double, double, double, double>> source);
std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> SmartMatch(std::vector<std::tuple<double, double, double, double>> peaks, std::vector<std::tuple<double, double, double, double>> source);

double Polynomial(double* x, double* par);

bool FilledBin(TH2* matrix, const int& bin);

class TSourceCalibration;

class TChannelTab;

class TSourceTab {
public:
   TSourceTab(TChannelTab* parent, TGCompositeFrame* frame, TH1D* projection, const double& sigma, const double& threshold, const int& degree, const std::vector<std::tuple<double, double, double, double>>& sourceEnergy);
   TSourceTab(const TSourceTab& rhs);
   TSourceTab(TSourceTab&&) noexcept            = default;
   TSourceTab& operator=(const TSourceTab&)     = default;
   TSourceTab& operator=(TSourceTab&&) noexcept = default;
   ~TSourceTab();

   void MakeConnections();
   void Disconnect();

   void ProjectionStatus(Event_t* event);
   void ProjectionStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   void Add(std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> map);
   void FindPeaks(const double& sigma, const double& threshold, const bool& force = false, const bool& fast = true);

   void SourceEnergy(std::vector<std::tuple<double, double, double, double>> val) { fSourceEnergy = std::move(val); }

   TGraphErrors* Data() { return fData; }

   void PrintLayout() const;

private:
   void BuildInterface();
	void UpdateRanges();
	void UpdateRange(const size_t& index);
	void DrawRanges();
	void DrawRange(const size_t& index);

   // parent
   TChannelTab* fParent{nullptr};

   // graphic elements
   TGCompositeFrame*    fSourceFrame{nullptr};
   TRootEmbeddedCanvas* fProjectionCanvas{nullptr};
   TGStatusBar*         fSourceStatusBar{nullptr};
	std::vector<TBox*>   fRangeBox;

   // storage elements
   TH1D*                                                   fProjection{nullptr};
   TGraphErrors*                                           fData{nullptr};
   double                                                  fSigma{2.};
   double                                                  fThreshold{0.05};
   int                                                     fDegree{1};
   std::vector<GPeak*>                                     fPeaks;
   std::vector<std::tuple<double, double, double, double>> fSourceEnergy;
	std::vector<std::pair<double, double>>                  fRanges;
};

class TChannelTab {
public:
   TChannelTab(TSourceCalibration* parent, std::vector<TNucleus*> nuclei, std::vector<TH1D*> projections, std::string name, TGCompositeFrame* frame, double sigma, double threshold, int degree, std::vector<std::vector<std::tuple<double, double, double, double>>> sourceEnergies, TGHProgressBar* progressBar);
   TChannelTab(const TChannelTab&)                = default;
   TChannelTab(TChannelTab&&) noexcept            = default;
   TChannelTab& operator=(const TChannelTab&)     = default;
   TChannelTab& operator=(TChannelTab&&) noexcept = default;
   ~TChannelTab();

   void CreateSourceTab(size_t source);

   void MakeConnections();
   void Disconnect();

   void CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   void          UpdateData();
   void          UpdateChannel();
   void          SelectedTab(Int_t id);
   void          Write(TFile* output);
   void          Calibrate();
   void          Calibrate(const int& degree, const bool& force = false);
   void          FindPeaks(const double& sigma, const double& threshold, const bool& force = false, const bool& fast = true);
   TGTab*        SourceTab() { return fSourceTab; }
   TGraphErrors* Data(int channelId) { return fSources[channelId]->Data(); }
   size_t        NumberOfSources() { return fSources.size(); }
	std::string   Name() { return fName; }
	int           ActiveSourceTab() { return fActiveSourceTab; }

   static void ZoomX();
   static void ZoomY();

   void PrintLayout() const;

private:
   // graphic elements
   TGCompositeFrame*        fChannelFrame{nullptr};   ///< main frame of this tab
   TGVerticalFrame*         fLeftFrame{nullptr};      ///< left frame with source tabs
   TGVerticalFrame*         fRightFrame{nullptr};     ///< right frame with calibration
   TGTab*                   fSourceTab{nullptr};      ///< tab for sources
   std::vector<TSourceTab*> fSources;                 ///< tabs for all sources
   TRootEmbeddedCanvas*     fCalibrationCanvas{nullptr};
   TPad*                    fResidualPad{nullptr};
   TPad*                    fCalibrationPad{nullptr};
   TGStatusBar*             fChannelStatusBar{nullptr};
   TLegend*                 fLegend{nullptr};
   TPaveText*               fChi2Label{nullptr};
   TGHProgressBar*          fProgressBar{nullptr};

   // storage elements
   TSourceCalibration*                                                  fParent;               ///< the parent of this tab
   std::vector<TNucleus*>                                               fNuclei;               ///< the source nucleus
   std::vector<TH1D*>                                                   fProjections;          ///< vector of all projections
   std::string                                                          fName;                 ///< name of this tab
   double                                                               fSigma{2.};            ///< the sigma used in the peak finder
   double                                                               fThreshold{0.05};      ///< the threshold (relative to the largest peak) used in the peak finder
   int                                                                  fDegree{1};            ///< degree of polynomial function used to calibrate
   std::vector<std::vector<std::tuple<double, double, double, double>>> fSourceEnergies;       ///< vector with source energies and uncertainties
   TCalibrationGraphSet*                                                fData{nullptr};        ///< combined data from all sources
	int                                                                  fActiveSourceTab{0};   ///< id of the currently active source tab
};

class TSourceCalibration : public TGMainFrame {
public:
   enum EEntry : int {
      kStartButton,
      kSourceBox      = 100,
      kSigmaEntry     = 200,
      kThresholdEntry = 300,
      kDegreeEntry    = 400
   };

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
   void Calibrate();
   void FindPeaks();
   void FindPeaksFast();
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
   std::vector<std::tuple<double, double, double, double>> SourceEnergy(const size_t& i) { return fSourceEnergy.at(i); }
   void                                                    CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   using TGWindow::HandleTimer;
   void HandleTimer();
   void SecondWindow();
   void FinalWindow();

   static void VerboseLevel(int val)
   {
      fVerboseLevel = val;
      if(val > 4) { TCalibrationGraphSet::VerboseLevel(val - 4); }
   }
   static int VerboseLevel() { return fVerboseLevel; }

   static void ZoomX();

   void PrintLayout() const;

private:
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
   TGTextButton*   fCalibrateButton{nullptr};
   TGTextButton*   fFindPeaksButton{nullptr};
   TGTextButton*   fFindPeaksFastButton{nullptr};
   TGTextButton*   fDiscardButton{nullptr};
   TGTextButton*   fAcceptButton{nullptr};
   TGTextButton*   fAcceptAllButton{nullptr};
   TGTextButton*   fNextButton{nullptr};
   TGGroupFrame*   fParameterFrame{nullptr};
   TGLabel*        fSigmaLabel{nullptr};
   TGNumberEntry*  fSigmaEntry{nullptr};
   TGLabel*        fThresholdLabel{nullptr};
   TGNumberEntry*  fThresholdEntry{nullptr};
   TGLabel*        fDegreeLabel{nullptr};
   TGNumberEntry*  fDegreeEntry{nullptr};
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

   unsigned int fLineHeight{20};   ///< Height of text boxes and progress bar

   int fOldErrorLevel;   ///< Used to store old value of gErrorIgnoreLevel (set to kError for the scope of the class)

   double fDefaultSigma{2.};         ///< The default sigma used for the peak finding algorithm, can be changed later.
   double fDefaultThreshold{0.05};   ///< The default threshold used for the peak finding algorithm, can be changed later. Co-56 source needs a much lower threshold, 0.01 or 0.02, but that makes it much slower too.
   int    fDefaultDegree{1};         ///< The default degree of the polynomial used for calibrating, can be changed later.

   TFile* fOutput{nullptr};

   static int fVerboseLevel;   ///< Changes verbosity from 0 (quiet) to 4 (very verbose)

   /// \cond CLASSIMP
   ClassDefOverride(TSourceCalibration, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

#endif
