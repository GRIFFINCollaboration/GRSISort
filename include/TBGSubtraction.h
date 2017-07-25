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
      kBGSlider,
      kPeakSlider
   };
   enum EEntries {
      kBGParamEntry,
      kGateLowEntry,
      kGateHighEntry,
      kPeakLowEntry,
      kPeakHighEntry,
      kPeakEntry,
      kBGLowEntry,
      kBGHighEntry,
      kWrite2FileNameEntry,
      kHistogramDescriptionEntry,
      kComboAxisEntry,
      kBGCheckButton
   };

   //  RQ_OBJECT("TBGSubtraction")
private:
   TGMainFrame*         fMain{nullptr};
   TRootEmbeddedCanvas* fProjectionCanvas;
   TRootEmbeddedCanvas* fGateCanvas;
   TH2*                 fMatrix;
   TH1*                 fProjection;
   TH1*                 fGateHist;
   TH1*                 fBGHist;
   TH1*                 fSubtractedHist;
   TGDoubleHSlider*     fGateSlider;
   TGDoubleHSlider*     fBGSlider;
   TGTripleHSlider*     fPeakSlider;
   TGNumberEntry*       fBGParamEntry;
   TGNumberEntry*       fBGEntryLow{nullptr};
   TGNumberEntry*       fBGEntryHigh{nullptr};
   TGNumberEntry*       fGateEntryLow{nullptr};
   TGNumberEntry*       fGateEntryHigh{nullptr};
   TGLabel*             fBGParamLabel{nullptr};
   TGCheckButton*       fBGCheckButton;

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
   TGHorizontalFrame* fBGEntryFrame{nullptr};
   TGHorizontalFrame* fDescriptionFrame{nullptr};
   TGHorizontalFrame* fButtonFrame{nullptr};

   // Combo box
   TGComboBox* fAxisCombo;

   // Markers
   GMarker* fLowGateMarker;
   GMarker* fHighGateMarker;
   GMarker* fLowBGMarker;
   GMarker* fHighBGMarker;
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
   void ClickedBGButton();
   void DoPeakFit();
   void DoSlider(Int_t pos = 0);
   void DoEntry(Long_t);
   void DoProjectionCanvasZoomed();
   void DoGateCanvasZoomed();
   void UpdateProjectionSliders();
   void UpdatePeakSliders();
   void UpdateBackground();
   void DoProjection();
   void DoGating();
   void DrawOnNewCanvas();
   void DrawGateMarkers();
   void DrawBGMarkers();
   void DrawPeakMarkers();
   void DrawPeak();
   void WriteHistograms();
   void GateStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);
   void ProjectionStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);

private:
   void BuildInterface();
   void ResetInterface();
   void MakeConnections();
   void InitializeInterface();
   void StatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);

   /// \cond CLASSIMP
   ClassDefOverride(TBGSubtraction, 6); // Background subtractor GUI
   /// \endcond
};
/*! @} */

#endif
