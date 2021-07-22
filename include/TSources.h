#ifndef TSOURCES_H
#define TSOURCES_H

#include <cstdarg>
#include <iostream>
#include <vector>
#include <string>

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

#include "GPeak.h"
#include "TNucleus.h"
#include "TCalibrationGraph.h"

std::map<std::tuple<double, double>, std::tuple<double, double> > Match(std::vector<std::tuple<double, double> > peaks, std::vector<std::tuple<double, double> > source);

bool FilledBin(TH2* matrix, const int& bin);

class TSources;

class TChannelTab {
public:
	TChannelTab(TGTab* parent, TH1* projection, TGCompositeFrame* frame, const double& sigma, const double& threshold, const bool& quadratic, const std::vector<std::tuple<double, double> >& sourceEnergy);
	TChannelTab(const TChannelTab& rhs);
	~TChannelTab();

	void MakeConnections();
	void Disconnect();

	void ProjectionStatus(Int_t event, Int_t px, Int_t py, TObject* selected);
	void CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

	void Add(std::map<std::tuple<double, double>, std::tuple<double, double> > map);
	void Calibrate(const bool& quadratic, const std::vector<std::tuple<double, double> >& sourceEnergy, const bool& force = false);
	void FindPeaks(const double& sigma, const double& threshold, const bool& force = false);

	TGraphErrors* Data() { return fData; }

private:
	void BuildInterface();

	// parent
	TGTab*               fParent{nullptr};

	// graphic elements
	TGCompositeFrame*    fFrame{nullptr};
	TGHorizontalFrame*   fTopFrame{nullptr};
	TGVerticalFrame*     fLeftFrame{nullptr};
	TGVerticalFrame*     fRightFrame{nullptr};
	TRootEmbeddedCanvas* fProjectionCanvas{nullptr};	
	TRootEmbeddedCanvas* fCalibrationCanvas{nullptr};	
	TGStatusBar*         fStatusBar{nullptr};

	// storage elements
	TH1* fProjection{nullptr};
	TGraphErrors* fData{nullptr};
	double fSigma{2.};
	double fThreshold{0.05};
	bool fQuadratic{false};
	std::vector<GPeak*> fPeaks;
};

class TSourceTab {
public:
	TSourceTab(TSources* parent, TNucleus* nucleus, TH2* matrix, TGCompositeFrame* frame, const double& sigma, const double& threshold, const bool& quadratic, const std::vector<std::tuple<double, double> >& sourceEnergy, TGHProgressBar* progressBar);
	~TSourceTab();

	void MakeConnections();
	void Disconnect();

	void Calibrate(const bool& quadratic, const std::vector<std::tuple<double, double> >& sourceEnergy, const bool& force = false) { fChannel[fChannelTab->GetCurrent()]->Calibrate(quadratic, sourceEnergy, force); }
	void FindPeaks(const double& sigma, const double& threshold, const bool& force = false) { fChannel[fChannelTab->GetCurrent()]->FindPeaks(sigma, threshold, force); }
	TGTab* ChannelTab() { return fChannelTab; }
	TGraphErrors* Data(int channelId) { return fChannel[channelId]->Data(); }
	size_t NumberOfChannels() { return fChannel.size(); }

private:
	// graphic elements
	TGCompositeFrame* fFrame{nullptr}; ///< main frame of this tab
	TGTab* fChannelTab{nullptr}; ///< tab for channels
	std::vector<TChannelTab*> fChannel; ///< tabs for all channels
	TGHProgressBar*      fProgressBar{nullptr};

	// storage elements
	std::vector<TH1*> fProjections; ///< vector of all projections
	TNucleus* fNucleus; ///< the source nucleus
	TSources* fParent; ///< the parent of this tab
};

class TSources : public TGMainFrame {
public:
	enum ESources {
		k22Na,
		k56Co,
		k60Co,
		k133Ba,
		k152Eu,
		k241Am
	};
	enum EEntry {
		kStartButton,
		kSourceBox = 100,
		kSigmaEntry = 200,
		kThresholdEntry = 300
	};

public:
	TSources(int n...);
	~TSources();

	void SetSource(Int_t windowId, Int_t entryId);
	void Start();

	void LineHeight(const unsigned int& val) { fLineHeight = val; Resize(GetSize()); }

	void Navigate(Int_t id);
	void Calibrate();
	void FindPeaks();
	void SelectedTab(Int_t id);

	void NavigateFinal(Int_t id);
	void CalibrateFinal(const int& channelId);
	void SelectedFinalTab(Int_t id);
	void UpdateChannel(const int& channelId);
	void WriteCalibration();

	double Sigma() { return fSigmaEntry->GetNumber(); }
	double Threshold() { return fThresholdEntry->GetNumber(); }
	bool Quadratic() { if(fQuadraticButton == nullptr) return false; return fQuadraticButton->IsOn(); }
	std::vector<std::tuple<double, double> > SourceEnergy(const size_t& i) { return fSourceEnergy.at(i); }
	void CalibrationStatus(Int_t event, Int_t px, Int_t py, TObject* selected);

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

	TGHorizontalFrame* fTopFrame{nullptr};
	TGHorizontalFrame* fBottomFrame{nullptr};
	TGVerticalFrame* fLeftFrame{nullptr};
	TGVerticalFrame* fRightFrame{nullptr};
	TGTextButton* fStartButton{nullptr};
	TGTab* fTab{nullptr};
	std::vector<TSourceTab*> fSourceTab;

	TGHButtonGroup*      fNavigationGroup{nullptr};
	TGTextButton*        fPreviousButton{nullptr};
	TGTextButton*        fCalibrateButton{nullptr};
	TGTextButton*        fFindPeaksButton{nullptr};
	TGTextButton*        fDiscardButton{nullptr};
	TGTextButton*        fAcceptButton{nullptr};
	TGTextButton*        fAcceptAllButton{nullptr};
	TGTextButton*        fNextButton{nullptr};
	TGGroupFrame*        fParameterFrame{nullptr};
	TGLabel*             fSigmaLabel{nullptr};
	TGNumberEntry*       fSigmaEntry{nullptr};
	TGLabel*             fThresholdLabel{nullptr};
	TGNumberEntry*       fThresholdEntry{nullptr};
	TGCheckButton*       fQuadraticButton{nullptr};
	TGHProgressBar*      fProgressBar{nullptr};
	std::vector<TGCompositeFrame*> fFinalTab;
	std::vector<TRootEmbeddedCanvas*> fFinalCanvas;
	std::vector<TPad*> fResidualPad;
	std::vector<TPad*> fCalibrationPad;
	std::vector<TGStatusBar*> fStatusBar;

	std::vector<TGLabel*> fMatrixNames;
	std::vector<TGComboBox*> fSourceBox;
	std::vector<TNucleus*> fSource;
	std::vector<std::vector<std::tuple<double, double> > > fSourceEnergy;
	std::vector<int> fActualSourceId;
	std::vector<std::vector<int> > fActualChannelId;
	std::vector<std::vector<TGraphErrors*> > fData;
	std::vector<TCalibrationGraphSet*> fFinalData;
	std::vector<const char*> fChannelLabel;
	std::map<int, int> fChannelToIndex;
	std::vector<TLegend*> fLegend;
	
	std::vector<TH2*> fMatrices;
	int fNofBins{0};

	unsigned int fLineHeight{20};

	int fOldErrorLevel;

	ClassDef(TSources, 1);
};

#endif
