#ifndef _TBGSUBTRACTION_H
#define _TBGSUBTRACTION_H
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include "TGDoubleSlider.h"
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

#include "TH1.h"
#include "TH2.h"
#include "GCanvas.h"
#include "TFile.h"

class TBGSubtraction : public TGMainFrame {
   enum ESliders {
      kGateSlider,
      kBGSlider,

   };
   enum EEntries {
      kBGParamEntry,
      kGateLowEntry,
      kGateHighEntry,
      kBGLowEntry,
      kBGHighEntry,
      kWrite2FileNameEntry,
      kHistogramDescriptionEntry,
		kComboAxisEntry,
		kBGCheckButton
   };

   //  RQ_OBJECT("TBGSubtraction")
private:
   TGMainFrame*         fMain;
   TRootEmbeddedCanvas* fProjectionCanvas;
   TRootEmbeddedCanvas* fGateCanvas;
   TH2*                 fMatrix;
   TH1*                 fProjection;
   TH1*                 fGateHist;
   TH1*                 fBGHist;
   TH1*                 fSubtractedHist;
   TGDoubleHSlider*     fGateSlider;
   TGDoubleHSlider*     fBGSlider;
   TGNumberEntry*       fBGParamEntry;
   TGNumberEntry*       fBGEntryLow;
   TGNumberEntry*       fBGEntryHigh;
   TGNumberEntry*       fGateEntryLow;
   TGNumberEntry*       fGateEntryHigh;
   TGLabel*             fBGParamLabel;
	TGCheckButton*			fBGCheckButton;

   TGLayoutHints* fBly;
   TGLayoutHints* fBly1;
   TGLayoutHints* fLayoutCanvases;
   TGLayoutHints* fLayoutParam;

   TGTextEntry* fWrite2FileName;
   TGTextEntry* fHistogramDescription;
   //      TGTextButton         *fDrawCanvasButton;
   TGTextButton* fWrite2FileButton;

   // Status Bars
   TGStatusBar* fProjectionStatus;

   // Frames
   TGVerticalFrame*   fGateFrame;
   TGVerticalFrame*   fProjectionFrame;
   TGHorizontalFrame* fBGParamFrame;
   TGHorizontalFrame* fGateEntryFrame;
   TGHorizontalFrame* fBGEntryFrame;
   TGHorizontalFrame* fDescriptionFrame;
   TGHorizontalFrame* fButtonFrame;

	//Combo box
	TGComboBox * fAxisCombo;

   // Markers
   GMarker* fLowGateMarker;
   GMarker* fHighGateMarker;
   GMarker* fLowBGMarker;
   GMarker* fHighBGMarker;

   TFile* fCurrentFile;

	Int_t fGateAxis;

	Bool_t fForceUpdate;

public:
   TBGSubtraction(TH2* mat, const char * gate_axis = "x");
   virtual ~TBGSubtraction();
   void AxisComboSelected();
   void ClickedBGButton();
	void DoDraw();
   void DoFit();
   void DoSlider(Int_t pos = 0);
   void DoEntry(Long_t);
   void DoGateCanvasModified();
   void DoProjection();
   void DrawOnNewCanvas();
   void DrawMarkers();
   void WriteHistograms();
   void GateStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);
   void ProjectionStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);

private:
   void BuildInterface();
   void StatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected);

   /// \cond CLASSIMP
   ClassDef(TBGSubtraction, 6); // Background subtractor GUI
   /// \endcond
};
/*! @} */

#endif
