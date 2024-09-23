#ifndef TCALIBRATEDESCANT_H
#define TCALIBRATEDESCANT_H
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

class TParameterInput : public TGHorizontalFrame {
public:
   explicit TParameterInput(TGVerticalFrame*& frame) : TGHorizontalFrame(frame, 400, 400) {}
   TParameterInput(const TParameterInput&)                = delete;
   TParameterInput(TParameterInput&&) noexcept            = delete;
   TParameterInput& operator=(const TParameterInput&)     = delete;
   TParameterInput& operator=(TParameterInput&&) noexcept = delete;
   ~TParameterInput()                                     = default;
   TGHorizontalFrame* Build(const std::string& name, const Int_t& baseId, const Double_t& xmin, const Double_t& xmax);

   Bool_t ProcessMessage(Long_t msg, Long_t parameter1, Long_t parameter2) override;

   double Value() const { return fEntry->GetNumber(); }
   double LowLimit() const { return fEntryLow->GetNumber(); }
   double HighLimit() const { return fEntryHigh->GetNumber(); }

   const char* Name() const { return fLabel->GetTitle(); }

   void Set(double val);
   void Set(double val, double low, double high);

   void UpdateSlider();
   void UpdateEntries();

   void Connect(TCalibrateDescant* parent);

private:
   void PrintStatus(const char*);

   Int_t fBaseId{-1};

   TGLabel*         fLabel{nullptr};
   TGTripleHSlider* fSlider{nullptr};
   TGNumberEntry*   fEntry{nullptr};
   TGNumberEntry*   fEntryLow{nullptr};
   TGNumberEntry*   fEntryHigh{nullptr};
};

class TCalibrateDescant : public TGMainFrame {
public:
   enum class ESourceType { k22NaLow,
                            k22NaHigh,
                            k24NaLow,
                            k24NaHigh,
                            k60Co,
                            k137Cs,
                            k152Eu121,
                            k152Eu344,
                            k152Eu1408,
                            k241AmEdge,
                            k241Am };
   enum EParameter { kAmplitude      = 0,
                     kPosition       = 3,
                     kSigma          = 6,
                     kDSigma         = 9,
                     kPeakAmp        = 12,
                     kPeakPos        = 15,
                     kPeakSigma      = 18,
                     kNoiseAmp       = 21,
                     kNoisePos       = 24,
                     kNoiseSigma     = 27,
                     kThreshold      = 30,
                     kThresholdSigma = 33,
                     kBgConst        = 36,
                     kBgAmp          = 39,
                     kBgDecayConst   = 42,
                     kCutoff         = 45 };

   explicit TCalibrateDescant(TH2* hist, const ESourceType& source = ESourceType::k137Cs);
   TCalibrateDescant(const TCalibrateDescant&)                = delete;
   TCalibrateDescant(TCalibrateDescant&&) noexcept            = delete;
   TCalibrateDescant& operator=(const TCalibrateDescant&)     = delete;
   TCalibrateDescant& operator=(TCalibrateDescant&&) noexcept = delete;
   ~TCalibrateDescant()                                       = default;

   Bool_t ProcessMessage(Long_t msg, Long_t parameter1, Long_t parameter2) override;

   void Previous();
   void Next();
   void Fit();
   void ResetFit();
   void UpdateInitialParameters();
   void Save();
   void Status(Int_t px, Int_t py, Int_t pz, TObject* selected);
   void FitCanvasZoomed();
   void CalibrationCanvasZoomed();
   void UpdateInitialFunction();

private:
   void BuildInterface();
   void MakeConnections();
   void CreateGraphicMembers();
   void UpdateInterface();
   void AddCalibrationPoint(double value, double uncertainty);
   void InitializeParameters();

   TH2*                       fMatrix{nullptr};
   int                        fCurrentProjection{0};
   std::vector<TH1D*>         fProjections;
   std::vector<TGraphErrors*> fCalibrations;
   TF1*                       fInitial{nullptr};
   TF1*                       fFit{nullptr};
   TF1*                       fEdge{nullptr};
   TF1*                       fPeak{nullptr};
   TF1*                       fNoise{nullptr};
   TF1*                       fBg{nullptr};

   ESourceType fSource{ESourceType::k137Cs};

   TGVerticalFrame* fLeftFrame{nullptr};
   TGVerticalFrame* fRightFrame{nullptr};

   TRootEmbeddedCanvas* fFitCanvas{nullptr};
   TRootEmbeddedCanvas* fCalibrationCanvas{nullptr};
   TGStatusBar*         fStatusBar{nullptr};

   TParameterInput* fAmplitude{nullptr};
   TParameterInput* fPosition{nullptr};
   TParameterInput* fSigma{nullptr};
   TParameterInput* fDSigma{nullptr};
   TParameterInput* fPeakAmp{nullptr};
   TParameterInput* fPeakPos{nullptr};
   TParameterInput* fPeakSigma{nullptr};
   TParameterInput* fNoiseAmp{nullptr};
   TParameterInput* fNoisePos{nullptr};
   TParameterInput* fNoiseSigma{nullptr};
   TParameterInput* fThreshold{nullptr};
   TParameterInput* fThresholdSigma{nullptr};
   TParameterInput* fBgConst{nullptr};
   TParameterInput* fBgAmp{nullptr};
   TParameterInput* fBgDecayConst{nullptr};
   TParameterInput* fCutoff{nullptr};

   TGHorizontalFrame* fTopButtonFrame{nullptr};
   TGHorizontalFrame* fBottomButtonFrame{nullptr};

   TGTextButton* fPreviousButton{nullptr};
   TGTextButton* fFitButton{nullptr};
   TGTextButton* fNextButton{nullptr};
   TGTextButton* fResetFitButton{nullptr};
   TGTextButton* fUpdateInitialButton{nullptr};
   TGTextButton* fSaveButton{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TCalibrateDescant, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

double MaximumRecoilEnergy(double gammaEnergy);
double SourceEnergy(const TCalibrateDescant::ESourceType& source);
double SourceEnergyUncertainty(const TCalibrateDescant::ESourceType& source);
double FullEdge(double* x, double* par);
/*! @} */
#endif
