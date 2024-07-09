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

std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> Match(std::vector<std::tuple<double, double, double, double>> peaks, std::vector<std::tuple<double, double, double, double>> source, int verboseLevel);
std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> SmartMatch(std::vector<std::tuple<double, double, double, double>> peaks, std::vector<std::tuple<double, double, double, double>> source, int verboseLevel);

double Polynomial(double* x, double* par);
double Efficiency(double* x, double* par);

bool FilledBin(TH2* matrix, const int& bin);

class TSourceCalibration;

class TChannelTab {
public:
   TChannelTab(TGTab* parent, TH1* projection, TGCompositeFrame* frame, const double& sigma, const double& threshold, const int& degree, const std::vector<std::tuple<double, double, double, double>>& sourceEnergy);
   TChannelTab(const TChannelTab& rhs);
   ~TChannelTab();

   void MakeConnections();
   void Disconnect();

   void ProjectionStatus(Int_t event, Int_t px, Int_t py, TObject* selected);
   void CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   void Add(std::map<std::tuple<double, double, double, double>, std::tuple<double, double, double, double>> map);
   void Calibrate(const int& degree, const bool& force = false);
   void FindPeaks(const double& sigma, const double& threshold, const std::vector<std::tuple<double, double, double, double>>& sourceEnergy, const bool& force = false, const bool& fast = true);

   TGraphErrors* Data() { return fData; }
   TGraphErrors* Efficiency() { return fEfficiency; }

   void VerboseLevel(int val) { fVerboseLevel = val; }

private:
   void BuildInterface();

   // parent
   TGTab* fParent{nullptr};

   // graphic elements
   TGCompositeFrame*    fFrame{nullptr};
   TGHorizontalFrame*   fTopFrame{nullptr};
   TGVerticalFrame*     fLeftFrame{nullptr};
   TGVerticalFrame*     fRightFrame{nullptr};
   TRootEmbeddedCanvas* fProjectionCanvas{nullptr};
   TRootEmbeddedCanvas* fCalibrationCanvas{nullptr};
   TGStatusBar*         fStatusBar{nullptr};

   // storage elements
   TH1*                fProjection{nullptr};
   TGraphErrors*       fData{nullptr};
   TGraphErrors*       fEfficiency{nullptr};
   double              fSigma{2.};
   double              fThreshold{0.05};
   int                 fDegree{1};
   std::vector<GPeak*> fPeaks;
   int                 fVerboseLevel{0};   ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
};

class TSourceTab {
public:
	TSourceTab(TSourceCalibration* parent, TNucleus* nucleus, TH2* matrix, TGCompositeFrame* frame, double sigma, double threshold, int degree, std::vector<std::tuple<double, double, double, double>> sourceEnergy, TGHProgressBar* progressBar);
   ~TSourceTab();

   void CreateChannelTab(int bin);

   void MakeConnections();
   void Disconnect();

   void          Calibrate(const int& degree, const bool& force = false) { fChannel[fChannelTab->GetCurrent()]->Calibrate(degree, force); }
   void          FindPeaks(const double& sigma, const double& threshold, const bool& force = false, const bool& fast = true) { fChannel[fChannelTab->GetCurrent()]->FindPeaks(sigma, threshold, fSourceEnergy, force, fast); }
   TGTab*        ChannelTab() { return fChannelTab; }
   TGraphErrors* Data(int channelId) { return fChannel[channelId]->Data(); }
   TGraphErrors* Efficiency(int channelId) { return fChannel[channelId]->Efficiency(); }
   size_t        NumberOfChannels() { return fChannel.size(); }

   void VerboseLevel(int val)
   {
      fVerboseLevel = val;
      for(auto* channel : fChannel) { channel->VerboseLevel(val); }
   }

private:
   // graphic elements
   TGCompositeFrame*         fFrame{nullptr};        ///< main frame of this tab
   TGTab*                    fChannelTab{nullptr};   ///< tab for channels
   std::vector<TChannelTab*> fChannel;               ///< tabs for all channels
   TGHProgressBar*           fProgressBar{nullptr};

   // storage elements
   std::vector<TH1*>                                       fProjections;       ///< vector of all projections
   TNucleus*                                               fNucleus;           ///< the source nucleus
   TSourceCalibration*                                     fParent;            ///< the parent of this tab
   TH2*                                                    fMatrix{nullptr};   ///< the matrix we're using
   double                                                  fSigma{2.};         ///< the sigma used in the peak finder
   double                                                  fThreshold{0.05};   ///< the threshold (relative to the largest peak) used in the peak finder
   int                                                     fDegree{1};         ///< degree of polynomial function used to calibrate
   std::vector<std::tuple<double, double, double, double>> fSourceEnergy;      ///< vector with source energies and uncertainties
   int                                                     fVerboseLevel{0};   ///< Changes verbosity from 0 (quiet) to 4 (very verbose)
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
   ~TSourceCalibration() override;

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
   void FitEfficiency(const int& channelId);
   void SelectedFinalTab(Int_t id);
   void SelectedFinalMainTab(Int_t id);
   void UpdateChannel(const int& channelId);
   void WriteCalibration();

   double Sigma() { return fSigmaEntry->GetNumber(); }
   double Threshold() { return fThresholdEntry->GetNumber(); }
   int    Degree()
   {
      if(fDegreeEntry != nullptr) { fDefaultDegree = fDegreeEntry->GetNumber(); }
      return fDefaultDegree;
   }
   std::vector<std::tuple<double, double, double, double>> SourceEnergy(const size_t& i) { return fSourceEnergy.at(i); }
   void                                                    CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

   using TGWindow::HandleTimer;
   void HandleTimer();
   void SecondWindow();
   void FinalWindow();

   void VerboseLevel(int val)
   {
      fVerboseLevel = val;
      for(auto* source : fSourceTab) { source->VerboseLevel(val); }
   }

   static void ZoomX();
   static void ZoomY();

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
   TGHorizontalFrame*       fBottomFrame{nullptr};
   TGVerticalFrame*         fLeftFrame{nullptr};
   TGVerticalFrame*         fRightFrame{nullptr};
   TGTextButton*            fStartButton{nullptr};
   TGTab*                   fTab{nullptr};
   std::vector<TGTab*>      fFinalTabs;
   std::vector<TSourceTab*> fSourceTab;

   TGTextButton*                     fEmitter{nullptr};
   TGHButtonGroup*                   fNavigationGroup{nullptr};
   TGTextButton*                     fPreviousButton{nullptr};
   TGTextButton*                     fCalibrateButton{nullptr};
   TGTextButton*                     fFindPeaksButton{nullptr};
   TGTextButton*                     fFindPeaksFastButton{nullptr};
   TGTextButton*                     fDiscardButton{nullptr};
   TGTextButton*                     fAcceptButton{nullptr};
   TGTextButton*                     fAcceptAllButton{nullptr};
   TGTextButton*                     fNextButton{nullptr};
   TGGroupFrame*                     fParameterFrame{nullptr};
   TGLabel*                          fSigmaLabel{nullptr};
   TGNumberEntry*                    fSigmaEntry{nullptr};
   TGLabel*                          fThresholdLabel{nullptr};
   TGNumberEntry*                    fThresholdEntry{nullptr};
   TGLabel*                          fDegreeLabel{nullptr};
   TGNumberEntry*                    fDegreeEntry{nullptr};
   TGHProgressBar*                   fProgressBar{nullptr};
   std::vector<TGCompositeFrame*>    fFinalTab;
   std::vector<TRootEmbeddedCanvas*> fFinalCanvas;
   std::vector<TPad*>                fResidualPad;
   std::vector<TPad*>                fCalibrationPad;
   std::vector<TGStatusBar*>         fStatusBar;
   std::vector<TLegend*>             fLegend;
   std::vector<TPaveText*>           fChi2Label;
   std::vector<TGCompositeFrame*>    fEfficiencyTabs;
   std::vector<TRootEmbeddedCanvas*> fEfficiencyCanvas;
   std::vector<TPad*>                fEfficiencyResidualPad;
   std::vector<TPad*>                fEfficiencyPad;
   std::vector<TGStatusBar*>         fEfficiencyStatusBar;
   std::vector<TLegend*>             fEfficiencyLegend;
   std::vector<TPaveText*>           fEfficiencyChi2Label;

   std::vector<TGLabel*>                                                fMatrixNames;
   std::vector<TGComboBox*>                                             fSourceBox;
   std::vector<TNucleus*>                                               fSource;
   std::vector<std::vector<std::tuple<double, double, double, double>>> fSourceEnergy;   ///< vector to hold source energy, energy uncertainty, intensity, and intensity uncertainty
   std::vector<int>                                                     fActualSourceId;
   std::vector<std::vector<int>>                                        fActualChannelId;
   std::vector<std::vector<TGraphErrors*>>                              fData;
   std::vector<TCalibrationGraphSet*>                                   fFinalData;
   std::vector<std::vector<TGraphErrors*>>                              fEfficiency;
   std::vector<TCalibrationGraphSet*>                                   fFinalEfficiency;
   std::vector<const char*>                                             fChannelLabel;
   std::map<int, int>                                                   fChannelToIndex;

   std::vector<TH2*> fMatrices;
   int               fNofBins{0};   ///< Number of filled bins in first matrix

   unsigned int fLineHeight{20};   ///< Height of text boxes and progress bar

   int fOldErrorLevel;   ///< Used to store old value of gErrorIgnoreLevel (set to kError for the scope of the class)

   int fVerboseLevel{0};   ///< Changes verbosity from 0 (quiet) to 4 (very verbose)

   double fDefaultSigma{2.};         ///< The default sigma used for the peak finding algorithm, can be changed later.
   double fDefaultThreshold{0.05};   ///< The default threshold used for the peak finding algorithm, can be changed later. Co-56 source needs a much lower threshold, 0.01 or 0.02, but that makes it much slower too.
   int    fDefaultDegree{1};         ///< The default degree of the polynomial used for calibrating, can be changed later.

   TFile* fOutput{nullptr};

   ClassDefOverride(TSourceCalibration, 1);
};

#endif
