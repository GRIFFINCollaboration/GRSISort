#ifndef _TBGSUBTRACTION_H
#define _TBGSUBTRACTION_H
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGSlider.h>
#include "TGDoubleSlider.h"
#include "TGSlider.h"
#include "TClass.h"
#include "TGNumberEntry.h"
#include "TGLabel.h"
#include <TGFrame.h>
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
      kBGHighEntry
   };

    //  RQ_OBJECT("TBGSubtraction")
   private:
      TGMainFrame          *fMain;
      TRootEmbeddedCanvas  *fProjectionCanvas;
      TRootEmbeddedCanvas  *fGateCanvas;
      TH2                  *fMatrix;      
      TH1                  *fProjection;
      TH1                  *fGateHist;
      TH1                  *fBGHist;
      TH1                  *fSubtractedHist;
      TGDoubleHSlider      *fGateSlider;
      TGDoubleHSlider      *fBGSlider;
      TGNumberEntry        *fBGParamEntry;
      TGNumberEntry        *fBGEntryLow;
      TGNumberEntry        *fBGEntryHigh;
      TGNumberEntry        *fGateEntryLow;
      TGNumberEntry        *fGateEntryHigh;
      TGLabel              *fBGParamLabel;

      TGLayoutHints        *fBly;
      TGLayoutHints        *fBly1;
      TGLayoutHints        *fLayoutCanvases;
      TGLayoutHints        *fLayoutParam;

      TGTextButton         *fDrawCanvasButton;
      TGTextButton         *fWrite2FileButton;


   //Frames
      TGVerticalFrame      *fGateFrame;
      TGVerticalFrame      *fProjectionFrame;
      TGHorizontalFrame    *fBGParamFrame;
      TGHorizontalFrame    *fGateEntryFrame;
      TGHorizontalFrame    *fBGEntryFrame;
      TGHorizontalFrame    *fButtonFrame;

   //Markers
      GMarker              *fLowGateMarker;
      GMarker              *fHighGateMarker;
      GMarker              *fLowBGMarker;
      GMarker              *fHighBGMarker;

      TFile                *fCurrentFile;
   
   public:
      TBGSubtraction(TH2* mat);
      virtual ~TBGSubtraction();
      void DoDraw();
      void DoFit();
      void DoSlider(Int_t pos = 0);
      void DoEntry(Long_t);
      void DoGateCanvasModified();
      void DoProjection();
      void DrawOnNewCanvas();
      void DrawMarkers();
      void WriteHistograms();

   private:
      void BuildInterface();

   /// \cond CLASSIMP
   ClassDef(TBGSubtraction,6);  // Event Fragments
   /// \endcond
};
/*! @} */

#endif
