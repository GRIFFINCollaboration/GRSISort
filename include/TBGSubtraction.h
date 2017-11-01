#ifndef _TBGSUBTRACTION_H
#define _TBGSUBTRACTION_H
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include "TGDoubleSlider.h"
#include "TGTripleSlider.h"
#include "TGSlider.h"
#include "TGToolTip.h"
#include "TClass.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include "TGStatusBar.h"
#include <TGFrame.h>
#include "TGComboBox.h"
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>

#include "TPeak.h"

#include "TH1.h"
#include "TH2.h"
#include "GCanvas.h"
#include "TFile.h"

class TBGSubtraction : public TGMainFrame {
   enum ESliders {
      kGateSlider,
      kBGSlider1,
      kBGSlider2,
      kPeakSlider
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
      kBGCheckButton1,
      kBGCheckButton2,
      kPeakSkewCheckButton
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
   TGDoubleHSlider*     fGateSlider;
   TGDoubleHSlider*     fBGSlider1;
   TGDoubleHSlider*     fBGSlider2;
   TGTripleHSlider*     fPeakSlider;
   TGNumberEntry*       fBGParamEntry;
   TGNumberEntry*       fBGEntryLow1{nullptr};
   TGNumberEntry*       fBGEntryHigh1{nullptr};
   TGNumberEntry*       fBGEntryLow2{nullptr};
   TGNumberEntry*       fBGEntryHigh2{nullptr};
   TGNumberEntry*       fGateEntryLow{nullptr};
   TGNumberEntry*       fGateEntryHigh{nullptr};
   TGLabel*             fBGParamLabel{nullptr};
   TGCheckButton*       fBGCheckButton1;
   TGCheckButton*       fBGCheckButton2;
   TGCheckButton*       fPeakSkewCheckButton;

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
   TGHorizontalFrame* fBGParamFrame{nullptr};
   TGHorizontalFrame* fGateEntryFrame{nullptr};
   TGHorizontalFrame* fBGEntryFrame1{nullptr};
   TGHorizontalFrame* fBGEntryFrame2{nullptr};
   TGHorizontalFrame* fDescriptionFrame{nullptr};
   TGHorizontalFrame* fButtonFrame{nullptr};

   // Combo box
   TGComboBox* fAxisCombo;

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
   Double_t fPeakLowLimit;
   Double_t fPeakHighLimit;
   Double_t fPeakLowValue;
   Double_t fPeakHighValue;
   Double_t fPeakValue;

   TPeak* fPeakFit;

public:
   TBGSubtraction(TH2* mat, const char* gate_axis = "x");
   ~TBGSubtraction() override;
   void AxisComboSelected();
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
   void DoProjection();
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

private:
   void BuildInterface();
   void ResetInterface();
   void MakeConnections();
   void InitializeInterface();
   void StatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);
   void DrawBGMarkers(TGCheckButton *&check_button, GMarker *&low_marker, GMarker *&high_marker, TGNumberEntry *&low_entry, TGNumberEntry *&high_entry, Int_t color = kBlue);
   void UpdateGateSlider();
   void UpdateBGSlider1();
   void UpdateBGSlider2();

   /// \cond CLASSIMP
   ClassDefOverride(TBGSubtraction, 6); // Background subtractor GUI
   /// \endcond
};
/*! @} */

#endif
