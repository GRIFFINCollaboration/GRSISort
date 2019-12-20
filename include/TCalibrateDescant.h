#ifndef _TCALIBRATEDESCANT_H
#define _TCALIBRATEDESCANT_H
#include "TGFrame.h"
#include "TRootEmbeddedCanvas.h"
#include "TGStatusBar.h"
#include "TGButton.h"
#include "TGTripleSlider.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"

#include "TH1D.h"
#include "TH2.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TMath.h"

#include "GCanvas.h"

/** \addtogroup GUI
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TCalibrateDescant
///
/// This class implements a GUI to fit and calibrate DESCANT.
///
/////////////////////////////////////////////////////////////////
class TCalibrateDescant;

class TParameterInput {
public:
	TParameterInput() {}
	~TParameterInput() {}
	TGHorizontalFrame* Build(TGVerticalFrame*& frame, const std::string& name, const Int_t& baseId, const Double_t& xmin, const Double_t& xmax);

	double Value()     const { return fEntry->GetNumber(); }
	double LowLimit()  const { return fEntryLow->GetNumber(); }
	double HighLimit() const { return fEntryHigh->GetNumber(); }

	const char* Name() const { return fLabel->GetTitle(); }

	void Set(double val, double min, double max);

	void UpdateSlider();
	void UpdateEntries();

	void Connect(TCalibrateDescant* parent);

private:
	Int_t fBaseId{-1};

	TGHorizontalFrame* fFrame{nullptr};

	TGLabel* fLabel{nullptr};
	TGTripleHSlider* fSlider{nullptr};
	TGNumberEntry* fEntry{nullptr};
	TGNumberEntry* fEntryLow{nullptr};
	TGNumberEntry* fEntryHigh{nullptr};
};

class TCalibrateDescant : public TGMainFrame {
public:
   enum class ESourceType { k22NaLow, k22NaHigh, k24NaLow, k24NaHigh, k60Co, k137Cs, k152Eu121, k152Eu344, k152Eu1408, k241AmEdge, k241Am };
	enum EParameter { kAmplitude = 0, kPosition = 3, kSigma = 6, kDSigma = 9, kPeakAmp = 12, kPeakPos = 15, kPeakSigma = 18, kNoiseAmp = 21, kNoisePos = 24, kNoiseSigma = 27, kThreshold = 30, kThresholdSigma = 33, kBgConst = 36, kBgAmp = 39, kBgDecayConst = 42 };

	TCalibrateDescant(TH2* hist, const ESourceType& source = ESourceType::k137Cs);

	void Previous();
	void Next();
	void Fit();
	void Save();
	void Status(Int_t px, Int_t py, Int_t pz, TObject* selected);
	void FitCanvasZoomed();
	void CalibrationCanvasZoomed();
	void UpdateInitial();

private:
	void BuildInterface();
	void MakeConnections();
	void CreateGraphicMembers();
	void UpdateInterface();
	void AddCalibrationPoint(double value, double uncertainty);
	void InitializeParameters();

	TH2* fMatrix{nullptr};
	int fCurrentProjection{0};
	std::vector<TH1D*> fProjections;
	std::vector<TGraphErrors*> fCalibrations;
	TF1* fInitial{nullptr};
	TF1* fFit{nullptr};
	TF1* fEdge{nullptr};
	TF1* fPeak{nullptr};
	TF1* fNoise{nullptr};
	TF1* fBg{nullptr};

	ESourceType fSource{ESourceType::k137Cs};

   TGVerticalFrame*     fLeftFrame{nullptr};
   TGVerticalFrame*     fRightFrame{nullptr};

   TRootEmbeddedCanvas* fFitCanvas{nullptr};
   TRootEmbeddedCanvas* fCalibrationCanvas{nullptr};
	TGStatusBar*         fStatusBar{nullptr};

	TParameterInput fAmplitude;
	TGHorizontalFrame* fAmplitudeFrame{nullptr};
	TParameterInput fPosition;
	TGHorizontalFrame* fPositionFrame{nullptr};
	TParameterInput fSigma;
	TGHorizontalFrame* fSigmaFrame{nullptr};
	TParameterInput fDSigma;
	TGHorizontalFrame* fDSigmaFrame{nullptr};
	TParameterInput fPeakAmp;
	TGHorizontalFrame* fPeakAmpFrame{nullptr};
	TParameterInput fPeakPos;
	TGHorizontalFrame* fPeakPosFrame{nullptr};
	TParameterInput fPeakSigma;
	TGHorizontalFrame* fPeakSigmaFrame{nullptr};
	TParameterInput fNoiseAmp;
	TGHorizontalFrame* fNoiseAmpFrame{nullptr};
	TParameterInput fNoisePos;
	TGHorizontalFrame* fNoisePosFrame{nullptr};
	TParameterInput fNoiseSigma;
	TGHorizontalFrame* fNoiseSigmaFrame{nullptr};
	TParameterInput fThreshold;
	TGHorizontalFrame* fThresholdFrame{nullptr};
	TParameterInput fThresholdSigma;
	TGHorizontalFrame* fThresholdSigmaFrame{nullptr};
	TParameterInput fBgConst;
	TGHorizontalFrame* fBgConstFrame{nullptr};
	TParameterInput fBgAmp;
	TGHorizontalFrame* fBgAmpFrame{nullptr};
	TParameterInput fBgDecayConst;
	TGHorizontalFrame* fBgDecayConstFrame{nullptr};

	TGHorizontalFrame* fButtonFrame{nullptr};

	TGTextButton* fPreviousButton{nullptr};
	TGTextButton* fFitButton{nullptr};
	TGTextButton* fNextButton{nullptr};
	TGTextButton* fSaveButton{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TCalibrateDescant, 1);
   /// \endcond
};

double MaximumRecoilEnergy(double gammaEnergy);
double SourceEnergy(const TCalibrateDescant::ESourceType& source);
double SourceEnergyUncertainty(const TCalibrateDescant::ESourceType& source);
double FullEdge(double* x, double* par);
/*! @} */
#endif
