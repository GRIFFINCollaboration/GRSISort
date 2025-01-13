#ifndef TBGSUBTRACTION_H
#define TBGSUBTRACTION_H
#include "TGClient.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TRandom.h"
#include "TGButton.h"
#include "TGDoubleSlider.h"
#include "TGTripleSlider.h"
#include "TGSlider.h"
#include "TGToolTip.h"
#include "TClass.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TGStatusBar.h"
#include "TGFrame.h"
#include "TGComboBox.h"
#include "TRootEmbeddedCanvas.h"
#include "RQ_OBJECT.h"

#include "TPeakFitter.h"
#include "TSinglePeak.h"

#include "TH1.h"
#include "TH2.h"
#include "GCanvas.h"
#include "TFile.h"

/** \addtogroup GUI
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TBGSubtraction
///
/// This class implements a GUI to perform projections of a 2D-Matrix
/// with proper background subtraction.
///
/// There is a known issue that clicking on the close button of the
/// window will cause a segmentation violation. If the class was
/// created on the heap (using `new`), this will make it impossible
/// to kill grsisort from within the terminal or sending it to the
/// background. If the class was however created on the stack, this
/// will simply crash grsisort. Because of this the recommended way
/// to start it is using a line like `TBGSubtraction bg(<matrix-name>)`
/// where bg is the variable name (can be any other valid c++ variable
/// name), and <matrix-name> is the name of the 2D histogram (without
/// any quotation marks).
///
/////////////////////////////////////////////////////////////////

class TBGSubtraction : public TGMainFrame {
   // these enums are used to communicate with ROOT classes
   // since those classes take ints as arguments there isn't
   // much gained by changing them to enum classes
   enum ESliders {
      kGateSlider,
      kBGSlider1,
      kBGSlider2,
      kPeakSlider,
      kBinningSlider
   };
   enum EEntries {
      kBGParamEntry,
      kGateLowEntry,
      kGateHighEntry,
      kPeakLowEntry,
      kPeakHighEntry,
      kPeakEntry,
      kBGLowEntry1,
      kBGHighEntry1,
      kBGLowEntry2,
      kBGHighEntry2,
      kWrite2FileNameEntry,
      kHistogramDescriptionEntry,
      kComboAxisEntry,
      kComboPeakEntry,
      kBGCheckButton1,
      kBGCheckButton2,
      kAutoUpdateCheckButton
   };
   enum EPeaks {
      kGauss   = 0,
      kRWPeak  = 1,
      kABPeak  = 2,
      kAB3Peak = 3
   };

   //  RQ_OBJECT("TBGSubtraction")
private:
   TGMainFrame*         fMain{nullptr};
   TRootEmbeddedCanvas* fProjectionCanvas{nullptr};
   TRootEmbeddedCanvas* fGateCanvas{nullptr};
   TH2*                 fMatrix{nullptr};
   TH1*                 fProjection{nullptr};
   TH1*                 fGateHist{nullptr};
   TH1*                 fBGHist1{nullptr};
   TH1*                 fBGHist2{nullptr};
   TH1*                 fSubtractedHist{nullptr};
   TH1*                 fSubtractedBinHist{nullptr};
   TGDoubleHSlider*     fGateSlider{nullptr};
   TGDoubleHSlider*     fBGSlider1{nullptr};
   TGDoubleHSlider*     fBGSlider2{nullptr};
   TGTripleHSlider*     fPeakSlider{nullptr};
   TGHSlider*           fBinningSlider{nullptr};
   TGNumberEntry*       fBGParamEntry{nullptr};
   TGNumberEntry*       fBGEntryLow1{nullptr};
   TGNumberEntry*       fBGEntryHigh1{nullptr};
   TGNumberEntry*       fBGEntryLow2{nullptr};
   TGNumberEntry*       fBGEntryHigh2{nullptr};
   TGNumberEntry*       fGateEntryLow{nullptr};
   TGNumberEntry*       fGateEntryHigh{nullptr};
   TGLabel*             fBGParamLabel{nullptr};
   TGLabel*             fBinningLabel{nullptr};
   TGCheckButton*       fBGCheckButton1{nullptr};
   TGCheckButton*       fBGCheckButton2{nullptr};
   TGCheckButton*       fAutoUpdateCheckButton{nullptr};

   TGLayoutHints* fBly{nullptr};
   TGLayoutHints* fBly1{nullptr};
   TGLayoutHints* fLayoutCanvases{nullptr};
   TGLayoutHints* fLayoutParam{nullptr};

   TGTextEntry*  fWrite2FileName{nullptr};
   TGTextEntry*  fHistogramDescription{nullptr};
   TGTextButton* fWrite2FileButton{nullptr};
   TGTextButton* fPeakFitButton{nullptr};

   // Status Bars
   TGStatusBar* fProjectionStatus{nullptr};

   // Frames
   TGVerticalFrame*   fGateFrame{nullptr};
   TGVerticalFrame*   fProjectionFrame{nullptr};
   TGHorizontalFrame* fPeakFitFrame{nullptr};
   TGHorizontalFrame* fBinningFrame{nullptr};
   TGHorizontalFrame* fBGParamFrame{nullptr};
   TGHorizontalFrame* fGateEntryFrame{nullptr};
   TGHorizontalFrame* fBGEntryFrame1{nullptr};
   TGHorizontalFrame* fBGEntryFrame2{nullptr};
   TGHorizontalFrame* fDescriptionFrame{nullptr};
   TGHorizontalFrame* fButtonFrame{nullptr};

   // Combo box
   TGComboBox* fAxisCombo{nullptr};
   TGComboBox* fPeakCombo{nullptr};

   // Markers
   GMarker* fLowGateMarker{nullptr};
   GMarker* fHighGateMarker{nullptr};
   GMarker* fLowBGMarker1{nullptr};
   GMarker* fHighBGMarker1{nullptr};
   GMarker* fLowBGMarker2{nullptr};
   GMarker* fHighBGMarker2{nullptr};
   GMarker* fLowPeakMarker{nullptr};
   GMarker* fHighPeakMarker{nullptr};
   GMarker* fPeakMarker{nullptr};

   TFile* fCurrentFile{nullptr};

   Int_t fGateAxis{0};

   Bool_t   fForceUpdate{true};
   Double_t fPeakLowLimit{0.};    ///< lower limit for peak slider range
   Double_t fPeakHighLimit{0.};   ///< upper limit for peak slider range
   Double_t fPeakLowValue{0.};    ///< low range for fit
   Double_t fPeakHighValue{0.};   ///< high range for fit
   Double_t fPeakValue{0.};       ///< centroid for fit

   TSinglePeak* fPeak{nullptr};         ///< the peak to be fit (will be a class that inherits from TSinglePeak)
   TPeakFitter* fPeakFitter{nullptr};   ///< the peak fitter that fPeak is added to

   Int_t fMaxBinning{20};   ///< maximum binning possible with binning slider (hard-coded, for now?)

public:
   explicit TBGSubtraction(TH2* mat, const char* gate_axis = "x", int maxBinning = 20);
   TBGSubtraction(const TBGSubtraction&) = delete;
   TBGSubtraction(TBGSubtraction&&)      = delete;
   ~TBGSubtraction();

   TBGSubtraction& operator=(const TBGSubtraction&) = delete;
   TBGSubtraction& operator=(TBGSubtraction&&)      = delete;

   void AxisComboSelected();
   void PeakComboSelected();
   void ClickedBGButton1();
   void ClickedBGButton2();
   //  void ClickedBG2Button();
   void DoPeakFit();
   void DoSlider(Int_t pos = 0);
   void DoEntry(Long_t);
   void DoProjectionCanvasZoomed();
   void DoGateCanvasZoomed();
   void UpdateProjectionSliders();
   void UpdateBackground();
   void UpdatePeakSliders();
   // void DoProjection();
   void DrawOnNewCanvas();
   void DrawAllMarkers();
   void DrawGateMarkers();
   void DrawBGMarkers1();
   void DrawBGMarkers2();
   void DrawPeakMarkers();
   void DrawPeak();
   void WriteHistograms();
   void GateStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);
   void ProjectionStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);

   void MakeGateHisto();
   void MakeBGHisto1();
   void MakeBGHisto2();
   void DoAllGates();

   void DoGateProjection();
   void SetStatusFromUpdateCheckButton();

private:
   void BuildInterface();
   void ResetInterface();
   void MakeConnections();
   void Disconnect();
   void InitializeInterface();
   void StatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);
   void DrawBGMarkers(TGCheckButton*& check_button, GMarker*& low_marker, GMarker*& high_marker, TGNumberEntry*& low_entry, TGNumberEntry*& high_entry, Color_t color = kBlue);
   void UpdateGateSlider();
   void UpdateBGSlider1();
   void UpdateBGSlider2();
   void UpdateBinningSlider();
   void RebinProjection();

   /// \cond CLASSIMP
   ClassDefOverride(TBGSubtraction, 7)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */

#endif
