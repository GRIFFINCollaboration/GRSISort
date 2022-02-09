#ifndef _TBGSUBTRACTION_H
#define _TBGSUBTRACTION_H
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
		kGauss = 0,
		kRWPeak = 1,
		kABPeak = 2,
		kAB3Peak = 3
	};

   //  RQ_OBJECT("TBGSubtraction")
private:
   TGMainFrame*         fMain{nullptr};
   TRootEmbeddedCanvas* fProjectionCanvas;
   TRootEmbeddedCanvas* fGateCanvas;
   TH2*                 fMatrix;
   TH1*                 fProjection;
   TH1*                 fGateHist;
   TH1*                 fBGHist1;
   TH1*                 fBGHist2;
   TH1*                 fSubtractedHist;
   TH1*                 fSubtractedBinHist;
   TGDoubleHSlider*     fGateSlider;
   TGDoubleHSlider*     fBGSlider1;
   TGDoubleHSlider*     fBGSlider2;
   TGTripleHSlider*     fPeakSlider;
	TGHSlider*				fBinningSlider;
   TGNumberEntry*       fBGParamEntry;
   TGNumberEntry*       fBGEntryLow1{nullptr};
   TGNumberEntry*       fBGEntryHigh1{nullptr};
   TGNumberEntry*       fBGEntryLow2{nullptr};
   TGNumberEntry*       fBGEntryHigh2{nullptr};
   TGNumberEntry*       fGateEntryLow{nullptr};
   TGNumberEntry*       fGateEntryHigh{nullptr};
   TGLabel*             fBGParamLabel{nullptr};
   TGLabel*             fBinningLabel{nullptr};
   TGCheckButton*       fBGCheckButton1;
   TGCheckButton*       fBGCheckButton2;
   TGCheckButton*       fAutoUpdateCheckButton;

   TGLayoutHints* fBly;
   TGLayoutHints* fBly1;
   TGLayoutHints* fLayoutCanvases{nullptr};
   TGLayoutHints* fLayoutParam{nullptr};

   TGTextEntry* fWrite2FileName{nullptr};
   TGTextEntry* fHistogramDescription{nullptr};
   //      TGTextButton         *fDrawCanvasButton;
   TGTextButton* fWrite2FileButton{nullptr};
   TGTextButton* fPeakFitButton{nullptr};

   // Status Bars
   TGStatusBar* fProjectionStatus{nullptr};

   // Frames
   TGVerticalFrame*   fGateFrame;
   TGVerticalFrame*   fProjectionFrame;
   TGHorizontalFrame* fPeakFitFrame{nullptr};
   TGHorizontalFrame* fBinningFrame{nullptr};
   TGHorizontalFrame* fBGParamFrame{nullptr};
   TGHorizontalFrame* fGateEntryFrame{nullptr};
   TGHorizontalFrame* fBGEntryFrame1{nullptr};
   TGHorizontalFrame* fBGEntryFrame2{nullptr};
   TGHorizontalFrame* fDescriptionFrame{nullptr};
   TGHorizontalFrame* fButtonFrame{nullptr};

   // Combo box
   TGComboBox* fAxisCombo;
   TGComboBox* fPeakCombo;

   // Markers
   GMarker* fLowGateMarker;
   GMarker* fHighGateMarker;
   GMarker* fLowBGMarker1;
   GMarker* fHighBGMarker1;
   GMarker* fLowBGMarker2;
   GMarker* fHighBGMarker2;
   GMarker* fLowPeakMarker;
   GMarker* fHighPeakMarker;
   GMarker* fPeakMarker;

   TFile* fCurrentFile;

   Int_t fGateAxis;

   Bool_t fForceUpdate;
   Double_t fPeakLowLimit; ///< lower limit for peak slider range
   Double_t fPeakHighLimit; ///< upper limit for peak slider range
   Double_t fPeakLowValue; ///< low range for fit
   Double_t fPeakHighValue; ///< high range for fit
   Double_t fPeakValue; ///< centroid for fit

   TSinglePeak* fPeak; ///< the peak to be fit (will be a class that inherits from TSinglePeak)
	TPeakFitter* fPeakFitter; ///< the peak fitter that fPeak is added to
	Int_t fPeakId; ///< the current ID of the peak

	Int_t fMaxBinning{10}; ///< maximum binning possible with binning slider (hard-coded, for now?)

public:
   TBGSubtraction(TH2* mat, const char* gate_axis = "x");
   ~TBGSubtraction() override;
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
   //void DoProjection();
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
   void DrawBGMarkers(TGCheckButton *&check_button, GMarker *&low_marker, GMarker *&high_marker, TGNumberEntry *&low_entry, TGNumberEntry *&high_entry, Int_t color = kBlue);
   void UpdateGateSlider();
   void UpdateBGSlider1();
   void UpdateBGSlider2();
	void UpdateBinningSlider();
	void RebinProjection();

   /// \cond CLASSIMP
   ClassDefOverride(TBGSubtraction, 7); // Background subtractor GUI
   /// \endcond
};
/*! @} */

#endif
