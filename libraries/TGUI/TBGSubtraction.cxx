#include "TROOT.h"
#include "TBGSubtraction.h"
#include "TGTripleSlider.h"
#include "TGNumberEntry.h"
#include "TInterpreter.h"
#include "TString.h"

/// \cond CLASSIMP
ClassImp(TBGSubtraction)
   /// \endcond

   TBGSubtraction::TBGSubtraction(TH2* mat, const char* gate_axis)
   : TGMainFrame(nullptr, 10, 10, kHorizontalFrame), fProjectionCanvas(nullptr), fGateCanvas(nullptr), fMatrix(mat),
     fProjection(nullptr), fGateHist(nullptr), fBGHist1(nullptr), fBGHist2(nullptr), fSubtractedHist(nullptr), fGateSlider(nullptr),
     fBGSlider1(nullptr), fBGSlider2(nullptr), fPeakSlider(nullptr), fBGParamEntry(nullptr), fBGCheckButton1(nullptr), fBGCheckButton2(nullptr), 
     fPeakSkewCheckButton(nullptr), fAutoUpdateCheckButton(nullptr), fBly(nullptr), fBly1(nullptr), fGateFrame(nullptr), 
     fProjectionFrame(nullptr), fAxisCombo(nullptr), fLowGateMarker(nullptr), fHighGateMarker(nullptr), fLowBGMarker1(nullptr),  
     fHighBGMarker1(nullptr), fLowBGMarker2(nullptr), fHighBGMarker2(nullptr), fLowPeakMarker(nullptr), fHighPeakMarker(nullptr), 
     fPeakMarker(nullptr), fGateAxis(0), fForceUpdate(true), fPeakFit(nullptr) {

   TString tmp_gate_word(gate_axis);
   tmp_gate_word.ToUpper();
   if(tmp_gate_word.EqualTo("X")) {
      fGateAxis = 0;
   } else {
      fGateAxis = 1;
   }

   if(fGateAxis == 0) {
      fProjection = fMatrix->ProjectionY();
   } else {
      fProjection = fMatrix->ProjectionX();
   }

   // fTree = 0;
   if(!gClient) {
      return;
   }
   gInterpreter->SaveContext();
   //First we build the interface based on how it looks
   BuildInterface();
   //Now we initialize pieces of the interface to create the initial projections etc.
   InitializeInterface();
   MakeConnections();
   fCurrentFile = TFile::CurrentFile();
}

void TBGSubtraction::MakeConnections(){

   //Connect Canvases to recognize that they have been zoomed
   //This function includes setting the slider, and drawing the gate markers.
   fProjectionCanvas->GetCanvas()->Connect("RangeChanged()", "TBGSubtraction", this, "DoProjectionCanvasZoomed()");

   fGateCanvas->GetCanvas()->Connect("RangeChanged()", "TBGSubtraction", this, "DoGateCanvasZoomed()");

   //Connect Status Info to canvases
   fProjectionCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBGSubtraction", this,
                                          "ProjectionStatusInfo(Int_t,Int_t,Int_t,TObject*)");
   fGateCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBGSubtraction", this,
                                     "GateStatusInfo(Int_t,Int_t,Int_t,TObject*)");

   //Connect the sliding of sliders
   //I'm storing the "true" information in the entry boxes. This is what we refer back to for
   //the actual value of the sliders, etc.
   //However, there are two number entries, so we will send out two signals per change. We want to connect the gating to 
   //the slider instead.

   fGateSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");

   fBGSlider1->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fBGSlider2->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");
   
   fPeakSlider->Connect("PointerPositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fPeakSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");

   //Connect the clicking of buttons
   fPeakFitButton->Connect("Clicked()", "TBGSubtraction", this, "DoPeakFit()");

   fBGCheckButton1->Connect("Clicked()","TBGSubtraction",this,"UpdateBackground()");
   fBGCheckButton2->Connect("Clicked()","TBGSubtraction",this,"UpdateBackground()");

   fWrite2FileButton->Connect("Clicked()", "TBGSubtraction", this, "WriteHistograms()");

   //Connect the axis combo box
   fAxisCombo->Connect("Selected(Int_t,Int_t)", "TBGSubtraction", this, "AxisComboSelected()");

   //Connect the Gate entries so that if you type a value in they do the proper things
   //Everyhting that is updated sets the value of these entries, which trickles down and fixes everything else
   fGateEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fGateEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryLow1->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryHigh1->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryLow2->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryHigh2->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryHigh2->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "UpdateProjectionSliders()");

   //Connect the bg paramater entry to do the proper thing
   fBGParamEntry->Connect("ValueSet(Long_t)","TBGSubtraction",this,"UpdateBackground()");

   //We want to connect the fit peak, and write histogram buttons with the update check box.
   //We don't want someone writing a histo, or fitting a peak when the gate isn't as expected.
   fAutoUpdateCheckButton->Connect("Clicked()","TBGSubtraction",this,"SetStatusFromUpdateCheckButton()");
}

void TBGSubtraction::ResetInterface(){
   Double_t def_gate_low  = 0.3;
   Double_t def_gate_high = 0.4; 
   Double_t def_bg_low    = 0.7; 
   Double_t def_bg_high   = 0.8; 
   Double_t def_peak_low  = 0.4; 
   Double_t def_peak_high = 0.5; //ratios of bar width

   if(fPeakFit){ 
      fPeakFit->Delete();
      fPeakFit = nullptr;
   }
   
   //Lets Project out the matrix


   //Set Up entry and sliders for doing gating.
   Double_t xmin, ymin, xmax, ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
   Double_t x_width = xmax - xmin;

   fGateEntryLow->SetNumber(xmin+def_gate_low*x_width);
   fGateEntryHigh->SetNumber(xmax-(1.0-def_gate_high)*x_width);
   fGateEntryLow->SetLimitValues(xmin,xmax);
   fGateEntryHigh->SetLimitValues(xmin,xmax);
   
   fBGEntryLow1->SetNumber(xmin+def_bg_low*x_width);
   fBGEntryHigh1->SetNumber(xmax-(1.0-def_bg_high)*x_width);
   fBGEntryLow1->SetLimitValues(xmin,xmax);
   fBGEntryHigh1->SetLimitValues(xmin,xmax);
 
   fBGEntryLow2->SetNumber(xmin+def_bg_low*x_width);
   fBGEntryHigh2->SetNumber(xmax-(1.0-def_bg_high)*x_width);
   fBGEntryLow2->SetLimitValues(xmin,xmax);
   fBGEntryHigh2->SetLimitValues(xmin,xmax);

   UpdateProjectionSliders();
   DrawAllMarkers();
   DoAllGates();
   DoGateProjection();

   //Set up peak fitting bars
   fSubtractedHist->GetXaxis()->UnZoom();
   fGateCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
   x_width = xmax - xmin;

   fPeakLowValue = xmin+def_peak_low*x_width;
   fPeakHighValue = xmax-(1.0-def_peak_high)*x_width;
   fPeakValue = xmin+(def_peak_low+0.05)*x_width;

   fPeakLowLimit = xmin;
   fPeakHighLimit = xmax;

   UpdatePeakSliders();
   DrawPeakMarkers();
}

void TBGSubtraction::InitializeInterface(){
   //Set up axis combo box for selecting x or y axis
   //This has to happen at the start to create the projection
   fAxisCombo->Resize(150, 20);
   fAxisCombo->AddEntry("x", 0);
   fAxisCombo->AddEntry("y", 1);
   fAxisCombo->SetEnabled();
   fAxisCombo->Select(fGateAxis);
   AxisComboSelected();

   ResetInterface();
   DoGateCanvasZoomed();
   DoProjectionCanvasZoomed();

   SetStatusFromUpdateCheckButton();
   
}

void TBGSubtraction::BuildInterface(){
   // Create a main frame
   fProjectionFrame = new TGVerticalFrame(this, 400, 400);
   // Create canvas widget
   fProjectionCanvas = new TRootEmbeddedCanvas("ProjectionCanvas", fProjectionFrame, 200, 200);
   // Create a horizontal frame widget with buttons

   fGateEntryFrame = new TGHorizontalFrame(fProjectionFrame, 400, 400);

   Double_t xmin, ymin, xmax, ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);

   // We are going to start the limits off at a specific ratio of the frame
   fGateEntryLow = new TGNumberEntry(fGateEntryFrame, 0, 3, kGateLowEntry, TGNumberFormat::kNESReal,
                                     TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);

   fGateEntryHigh =
      new TGNumberEntry(fGateEntryFrame, 0, 3, kGateHighEntry, TGNumberFormat::kNESReal,
                        TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);

   fGateSlider = new TGDoubleHSlider(fProjectionFrame, 100, kDoubleScaleBoth, kGateSlider, kHorizontalFrame);

   fBGEntryFrame1 = new TGHorizontalFrame(fProjectionFrame, 200, 200);
   fBGEntryLow1   = new TGNumberEntry(fBGEntryFrame1, 0, 3, kBGLowEntry1, TGNumberFormat::kNESReal,
                                   TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);

   fBGEntryHigh1 = new TGNumberEntry(fBGEntryFrame1, 0, 3, kBGHighEntry1, TGNumberFormat::kNESReal,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);


   fBGEntryFrame2 = new TGHorizontalFrame(fProjectionFrame, 200, 200);
   fBGEntryLow2   = new TGNumberEntry(fBGEntryFrame2, 0, 3, kBGLowEntry2, TGNumberFormat::kNESReal,
                                   TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);

   fBGEntryHigh2 = new TGNumberEntry(fBGEntryFrame2, 0, 3, kBGHighEntry2, TGNumberFormat::kNESReal,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);


   fBGSlider1 = new TGDoubleHSlider(fProjectionFrame, 100, kDoubleScaleBoth, kBGSlider1, kHorizontalFrame);
   fBGSlider2 = new TGDoubleHSlider(fProjectionFrame, 100, kDoubleScaleBoth, kBGSlider2, kHorizontalFrame);

   fGateFrame = new TGVerticalFrame(this, 200, 200);
   fGateCanvas = new TRootEmbeddedCanvas("GateCanvas", fGateFrame, 200, 200);
   
   // Status Bars
   Int_t parts[]     = {20, 50};
   fProjectionStatus = new TGStatusBar(fProjectionFrame, 50, 10, kHorizontalFrame);
   fProjectionStatus->SetParts(parts, 2);

   fPeakSlider = new TGTripleHSlider(fGateFrame, 100, kDoubleScaleBoth, kPeakSlider, kHorizontalFrame);


   fPeakFitFrame  = new TGHorizontalFrame(fGateFrame, 200, 200);
   fPeakFitButton = new TGTextButton(fPeakFitFrame, "&Fit Peak");
   fPeakSkewCheckButton = new TGCheckButton(fPeakFitFrame, "Skew", kPeakSkewCheckButton);
   fPeakSkewCheckButton->SetState(kButtonUp);

   fBGParamFrame = new TGHorizontalFrame(fGateFrame, 200, 200);
   fBGParamLabel = new TGLabel(fBGParamFrame, "Background:");
   fBGParamEntry = new TGNumberEntry(fBGParamFrame, 20, 4, kBGParamEntry, TGNumberFormat::kNESInteger, // style
                                     TGNumberFormat::kNEANonNegative, // input value filter
                                     TGNumberFormat::kNELLimitMin,    // specify limits
                                     1, 1.);                          // limit values

   fAxisCombo = new TGComboBox(fBGParamFrame, kComboAxisEntry);

   fBGCheckButton1 = new TGCheckButton(fBGParamFrame, "BG 1", kBGCheckButton1);
   fBGCheckButton1->SetState(kButtonDown);

   fBGCheckButton2 = new TGCheckButton(fBGParamFrame, "BG 2", kBGCheckButton2);
   fBGCheckButton2->SetState(kButtonUp);

   fDescriptionFrame     = new TGHorizontalFrame(fGateFrame, 200, 200);
   fAutoUpdateCheckButton = new TGCheckButton(fDescriptionFrame, "Auto Update", kAutoUpdateCheckButton);
   fAutoUpdateCheckButton->SetState(kButtonDown);
   fHistogramDescription = new TGTextEntry(fDescriptionFrame, "gated #gamma-#gamma", kHistogramDescriptionEntry);

   fButtonFrame    = new TGHorizontalFrame(fGateFrame, 200, 200);
   fWrite2FileName = new TGTextEntry(fButtonFrame, "default.root", kWrite2FileNameEntry);
   fWrite2FileButton = new TGTextButton(fButtonFrame, "&Write Histograms");

   fBly            = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 1, 1, 3, 1);
   fLayoutCanvases = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 1, 1, 3, 4);
   fLayoutParam    = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 1, 1, 3, 20);

   fBly1 = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 20, 10, 15, 0);

   //Build smaller horizontal frames
   fBGParamFrame->AddFrame(fAxisCombo, fBly);
   fBGParamFrame->AddFrame(fBGCheckButton1, fBly);
   fBGParamFrame->AddFrame(fBGCheckButton2, fBly);
   fBGParamFrame->AddFrame(fBGParamLabel, fBly);
   fBGParamFrame->AddFrame(fBGParamEntry, fBly);

   fGateEntryFrame->AddFrame(fGateEntryLow, fBly);
   fGateEntryFrame->AddFrame(fGateEntryHigh, fBly);

   fPeakFitFrame->AddFrame(fPeakSkewCheckButton,fBly);
   fPeakFitFrame->AddFrame(fPeakFitButton,fBly);

   fBGEntryFrame1->AddFrame(fBGEntryLow1, fBly);
   fBGEntryFrame1->AddFrame(fBGEntryHigh1, fBly);

   fBGEntryFrame2->AddFrame(fBGEntryLow2, fBly);
   fBGEntryFrame2->AddFrame(fBGEntryHigh2, fBly);

   fDescriptionFrame->AddFrame(fAutoUpdateCheckButton, fBly);
   fDescriptionFrame->AddFrame(fHistogramDescription, fBly);

   fButtonFrame->AddFrame(fWrite2FileName, fBly);
   fButtonFrame->AddFrame(fWrite2FileButton, fBly);

   //Build left and right frames
   fProjectionFrame->AddFrame(fProjectionCanvas, fLayoutCanvases);
   fProjectionFrame->AddFrame(fProjectionStatus, fBly);
   fProjectionFrame->AddFrame(fGateSlider, fBly);
   fProjectionFrame->AddFrame(fBGSlider1, fBly);
   fProjectionFrame->AddFrame(fBGSlider2, fBly);
   fProjectionFrame->AddFrame(fGateEntryFrame, fBly);
   fProjectionFrame->AddFrame(fBGEntryFrame1, fBly);
   fProjectionFrame->AddFrame(fBGEntryFrame2, fBly);

   fGateFrame->AddFrame(fGateCanvas, fLayoutCanvases);
   fGateFrame->AddFrame(fPeakSlider,fBly);
   fGateFrame->AddFrame(fPeakFitFrame,fBly);
   fGateFrame->AddFrame(fBGParamFrame, fLayoutParam);
   fGateFrame->AddFrame(fDescriptionFrame, fLayoutParam);
   fGateFrame->AddFrame(fButtonFrame, fLayoutParam);

   AddFrame(fProjectionFrame, fBly1);
   AddFrame(fGateFrame, fBly1);

   // Set a name to the main frame
   SetWindowName("Gater and Subtractor");

   // Map all subwindows of main frame
   MapSubwindows();

   // Initialize the layout algorithm
   Resize(GetDefaultSize());

   // Map main frame
   MapWindow();
}

TBGSubtraction::~TBGSubtraction()
{
   // Clean up used widgets: frames, buttons, layout hints
   Cleanup();
   if(fLowBGMarker1 != nullptr) {
      delete fLowBGMarker1;
   }
   if(fHighBGMarker1 != nullptr) {
      delete fHighBGMarker1;
   }
   if(fLowBGMarker2 != nullptr) {
      delete fLowBGMarker2;
   }
   if(fHighBGMarker2 != nullptr) {
      delete fHighBGMarker2;
   }
   if(fLowGateMarker != nullptr) {
      delete fLowGateMarker;
   }
   if(fHighGateMarker != nullptr) {
      delete fHighGateMarker;
   }
   if(fHighPeakMarker != nullptr) {
      delete fHighPeakMarker;
   }
   if(fLowPeakMarker != nullptr) {
      delete fLowPeakMarker;
   }
   if(fPeakMarker != nullptr) {
      delete fPeakMarker;
   }
}

void TBGSubtraction::UpdateBackground(){
   // Draws function graphics in randomly chosen interval
   TCanvas* fCanvas = fProjectionCanvas->GetCanvas();
   fCanvas->cd();
   fProjection->Draw();
   DrawAllMarkers();
   if((fAutoUpdateCheckButton != nullptr) && fAutoUpdateCheckButton->IsDown()){ 
      DoGateProjection();
   }
   if(((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()) || ((fBGCheckButton2 != nullptr) && fBGCheckButton2->IsDown())) {
      fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber());
   }
   fCanvas->Update();
   fGateCanvas->GetCanvas()->cd();
}

void TBGSubtraction::DoPeakFit(){

   if(fPeakFit) fPeakFit->Delete(); fPeakFit = nullptr;
   
   fPeakFit = new TPeak(fPeakValue, fPeakLowValue, fPeakHighValue);
   fGateCanvas->GetCanvas()->cd();
   if((fPeakSkewCheckButton != nullptr) && fPeakSkewCheckButton->IsDown()) {
      fPeakFit->Fit(fSubtractedHist,"Q");
      fPeakFit->ReleaseParameter(3);
      fPeakFit->ReleaseParameter(4);
   }
   fPeakFit->Fit(fSubtractedHist);
   DrawPeak();
   fGateCanvas->GetCanvas()->Update();
}

void TBGSubtraction::DrawPeak(){
   if(fPeakFit)
      fPeakFit->Draw("same");
}

void TBGSubtraction::ClickedBGButton1()
{
   if((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()) {
      fBGParamEntry->SetState(true);
   } else {
      fBGParamEntry->SetState(false);
   }
}

void TBGSubtraction::UpdateGateSlider(){
   fGateSlider->SetPosition(fGateEntryLow->GetNumber(),fGateEntryHigh->GetNumber());
   fGateSlider->SetRange(fGateEntryLow->GetNumMin(), fGateEntryHigh->GetNumMax());
}

void TBGSubtraction::UpdateBGSlider1(){
   fBGSlider1->SetPosition(fBGEntryLow1->GetNumber(), fBGEntryHigh1->GetNumber());
   fBGSlider1->SetRange(fBGEntryLow1->GetNumMin(), fBGEntryHigh1->GetNumMax());
}

void TBGSubtraction::UpdateBGSlider2(){
   fBGSlider2->SetPosition(fBGEntryLow2->GetNumber(), fBGEntryHigh2->GetNumber());
   fBGSlider2->SetRange(fBGEntryLow2->GetNumMin(), fBGEntryHigh2->GetNumMax());
}

void TBGSubtraction::UpdateProjectionSliders(){
   //Something has happened, so we have to let the sliders know about it
   //This is as simple as checking the status of the "master" entry boxes
   UpdateGateSlider();
   UpdateBGSlider1();
   UpdateBGSlider2();
}

void TBGSubtraction::UpdatePeakSliders(){
   //Something has happened, so we have to let the sliders know about it
   //This is as simple as checking the status of the "master" entry boxes
   //The range has to be set first so that the other slider pieces don't get confused.
   fPeakSlider->SetRange(fPeakLowLimit, fPeakHighLimit);
   fPeakSlider->SetPosition(fPeakLowValue,fPeakHighValue);
   fPeakSlider->SetPointerPosition(fPeakValue);

}

void TBGSubtraction::DoSlider(Int_t pos)
{
   // Handle slider widgets.
   Int_t    id;
   TGFrame* frm = reinterpret_cast<TGFrame*>(gTQSender);
   if(frm->IsA()->InheritsFrom(TGSlider::Class())) {
      TGSlider* sl = static_cast<TGSlider*>(frm);
      id           = sl->WidgetId();
   } else {
      TGDoubleSlider* sd = static_cast<TGDoubleSlider*>(frm);
      id                 = sd->WidgetId();
   }
   char buf[32];
   sprintf(buf, "%d", pos);

   switch(id) {
      case kGateSlider: 
         fGateEntryLow->SetNumber(fGateSlider->GetMinPosition());
         fGateEntryHigh->SetNumber(fGateSlider->GetMaxPosition());
         DrawGateMarkers();
         if((fAutoUpdateCheckButton != nullptr) && fAutoUpdateCheckButton->IsDown()){ 
            MakeGateHisto();
            DoGateProjection();
         }
         break;
      case kBGSlider1:
         fBGEntryLow1->SetNumber(fBGSlider1->GetMinPosition());
         fBGEntryHigh1->SetNumber(fBGSlider1->GetMaxPosition());
         DrawBGMarkers1();
         if((fAutoUpdateCheckButton != nullptr) && fAutoUpdateCheckButton->IsDown()){ 
            MakeBGHisto1();
            DoGateProjection();
         }
         break;    
      case kBGSlider2:
         fBGEntryLow2->SetNumber(fBGSlider2->GetMinPosition());
         fBGEntryHigh2->SetNumber(fBGSlider2->GetMaxPosition());
         DrawBGMarkers2();
         if((fAutoUpdateCheckButton != nullptr) && fAutoUpdateCheckButton->IsDown()){ 
            MakeBGHisto2();
            DoGateProjection();
         }
         break;    
      case kPeakSlider:
         fPeakLowValue  = fPeakSlider->GetMinPosition();
         fPeakHighValue = fPeakSlider->GetMaxPosition();
         fPeakValue     = fPeakSlider->GetPointerPosition();
         DrawPeakMarkers();
         return;
         break;
   };
}

void TBGSubtraction::AxisComboSelected() {
   static int old_selection = -1;
   fGateAxis                = fAxisCombo->GetSelected();

   if(old_selection != fGateAxis) {
      old_selection = fGateAxis;
      delete fProjection;
      if(fGateAxis == 0) {
         std::cout << "Selecting the x axis" << std::endl;
         fProjection = fMatrix->ProjectionY();
      } else {
         std::cout << "Selecting the y axis" << std::endl;
         fProjection = fMatrix->ProjectionX();
      }
      
      fProjectionCanvas->GetCanvas()->cd();
      fProjection->Draw();
      ResetInterface();
      fProjectionCanvas->GetCanvas()->Update();
   }
}

void TBGSubtraction::DrawPeakMarkers(){
   if(fLowPeakMarker == nullptr){
      fLowPeakMarker = new GMarker();
   }
   if(fHighPeakMarker == nullptr){
      fHighPeakMarker = new GMarker();
   }
   if(fPeakMarker == nullptr){
      fPeakMarker = new GMarker();
   }
   if(fSubtractedHist){
      Float_t slider_x_max, slider_x_min, slider_x;
      fPeakSlider->GetPosition(slider_x_min,slider_x_max);
      slider_x = fPeakSlider->GetPointerPosition();
      fLowPeakMarker->localx  = fPeakLowValue;
      fHighPeakMarker->localx = fPeakHighValue;
      fPeakMarker->localx     = fPeakValue;
      fLowPeakMarker->binx    = fSubtractedHist->GetXaxis()->FindBin(fLowPeakMarker->localx);
      fHighPeakMarker->binx   = fSubtractedHist->GetXaxis()->FindBin(fHighPeakMarker->localx);
      fPeakMarker->binx       = fSubtractedHist->GetXaxis()->FindBin(fPeakMarker->localx);

   
      double low_peak_bin_edge  = fSubtractedHist->GetXaxis()->GetBinLowEdge(fLowPeakMarker->binx);
      double high_peak_bin_edge = fSubtractedHist->GetXaxis()->GetBinLowEdge(fHighPeakMarker->binx);
      double peak_bin_edge       = fSubtractedHist->GetXaxis()->GetBinLowEdge(fPeakMarker->binx);
  
      if((fLowPeakMarker->linex) == nullptr) {
         fLowPeakMarker->linex =
            new TLine(low_peak_bin_edge, fSubtractedHist->GetMinimum(), low_peak_bin_edge, fSubtractedHist->GetMaximum());
         fLowPeakMarker->SetColor(kMagenta);
      }

      if((fHighPeakMarker->linex) == nullptr) {
         fHighPeakMarker->linex =
            new TLine(high_peak_bin_edge, fSubtractedHist->GetMinimum(), high_peak_bin_edge, fSubtractedHist->GetMaximum());
         fHighPeakMarker->SetColor(kMagenta);
      }
      if((fPeakMarker->linex) == nullptr) {
         fPeakMarker->linex =
            new TLine(peak_bin_edge, fSubtractedHist->GetMinimum(), peak_bin_edge, fSubtractedHist->GetMaximum());
         fPeakMarker->SetColor(kMagenta);
         fPeakMarker->SetStyle(kDashed);
      }

      fLowPeakMarker->linex->SetX1(low_peak_bin_edge);
      fLowPeakMarker->linex->SetX2(low_peak_bin_edge);
      fLowPeakMarker->linex->SetY1(fSubtractedHist->GetMinimum());
      fLowPeakMarker->linex->SetY2(fSubtractedHist->GetMaximum());
      
      fHighPeakMarker->linex->SetX1(high_peak_bin_edge);
      fHighPeakMarker->linex->SetX2(high_peak_bin_edge);
      fHighPeakMarker->linex->SetY1(fSubtractedHist->GetMinimum());
      fHighPeakMarker->linex->SetY2(fSubtractedHist->GetMaximum());

      fPeakMarker->linex->SetX1(peak_bin_edge);
      fPeakMarker->linex->SetX2(peak_bin_edge);
      fPeakMarker->linex->SetY1(fSubtractedHist->GetMinimum());
      fPeakMarker->linex->SetY2(fSubtractedHist->GetMaximum());

      fGateCanvas->GetCanvas()->cd();
      fLowPeakMarker->linex->Draw();
      fHighPeakMarker->linex->Draw();
      fPeakMarker->linex->Draw();
      fGateCanvas->GetCanvas()->Update();

   }
}

void TBGSubtraction::DrawBGMarkers(TGCheckButton *&check_button, GMarker *&low_marker, GMarker *&high_marker, TGNumberEntry *&low_entry, TGNumberEntry *&high_entry, Int_t color){

      //Only Draw the BG Markers if BG is applied.
   if((check_button != nullptr) && check_button->IsDown()) {
      if(low_marker == nullptr) {
         low_marker = new GMarker();
      }
      if(high_marker == nullptr) {
         high_marker = new GMarker();
      }
      low_marker->localx         = low_entry->GetNumber();
      high_marker->localx        = high_entry->GetNumber();
      low_marker->binx           = fProjection->GetXaxis()->FindBin(low_marker->localx);
      high_marker->binx          = fProjection->GetXaxis()->FindBin(high_marker->localx);
      double low_bg_bin_edge     = fProjection->GetXaxis()->GetBinLowEdge(low_marker->binx);
      double high_bg_bin_edge    = fProjection->GetXaxis()->GetBinLowEdge(high_marker->binx);
   
      if((low_marker->linex) == nullptr) {
         low_marker->linex =
            new TLine(low_bg_bin_edge, fProjection->GetMinimum(), low_bg_bin_edge, fProjection->GetMaximum());
         low_marker->SetColor(color);
      }

      if((high_marker->linex) == nullptr) {
         high_marker->linex =
            new TLine(high_bg_bin_edge, fProjection->GetMinimum(), high_bg_bin_edge, fProjection->GetMaximum());
         high_marker->SetColor(color);
      }
      low_marker->linex->SetX1(low_bg_bin_edge);
      low_marker->linex->SetX2(low_bg_bin_edge);
      low_marker->linex->SetY1(fProjection->GetMinimum());
      low_marker->linex->SetY2(fProjection->GetMaximum());

      high_marker->linex->SetX1(high_bg_bin_edge);
      high_marker->linex->SetX2(high_bg_bin_edge);
      high_marker->linex->SetY1(fProjection->GetMinimum());
      high_marker->linex->SetY2(fProjection->GetMaximum());

      fProjectionCanvas->GetCanvas()->cd();
      low_marker->linex->Draw();
      high_marker->linex->Draw();
      fProjectionCanvas->GetCanvas()->Update();
   }
   
}

void TBGSubtraction::DrawBGMarkers1(){
   DrawBGMarkers(fBGCheckButton1, fLowBGMarker1, fHighBGMarker1, fBGEntryLow1, fBGEntryHigh1);
}

void TBGSubtraction::DrawBGMarkers2(){
   DrawBGMarkers(fBGCheckButton2, fLowBGMarker2, fHighBGMarker2, fBGEntryLow2, fBGEntryHigh2,kMagenta);
}

void TBGSubtraction::DrawAllMarkers(){
   DrawGateMarkers();
   DrawBGMarkers1();
   DrawBGMarkers2();
}

void TBGSubtraction::DrawGateMarkers(){
   if(fLowGateMarker == nullptr) {
      fLowGateMarker = new GMarker();
   }
   if(fHighGateMarker == nullptr) {
      fHighGateMarker = new GMarker();
   }

   fLowGateMarker->localx  = fGateEntryLow->GetNumber();
   fHighGateMarker->localx = fGateEntryHigh->GetNumber();
   fLowGateMarker->binx    = fProjection->GetXaxis()->FindBin(fLowGateMarker->localx);
   fHighGateMarker->binx   = fProjection->GetXaxis()->FindBin(fHighGateMarker->localx);

   double low_gate_bin_edge  = fProjection->GetXaxis()->GetBinLowEdge(fLowGateMarker->binx);
   double high_gate_bin_edge = fProjection->GetXaxis()->GetBinLowEdge(fHighGateMarker->binx);
   if((fLowGateMarker->linex) == nullptr) {
      fLowGateMarker->linex =
         new TLine(low_gate_bin_edge, fProjection->GetMinimum(), low_gate_bin_edge, fProjection->GetMaximum());
      fLowGateMarker->SetColor(kGreen);
   }

   if((fHighGateMarker->linex) == nullptr) {
      fHighGateMarker->linex =
         new TLine(high_gate_bin_edge, fProjection->GetMinimum(), high_gate_bin_edge, fProjection->GetMaximum());
      fHighGateMarker->SetColor(kGreen);
   }

   fLowGateMarker->linex->SetX1(low_gate_bin_edge);
   fLowGateMarker->linex->SetX2(low_gate_bin_edge);
   fLowGateMarker->linex->SetY1(fProjection->GetMinimum());
   fLowGateMarker->linex->SetY2(fProjection->GetMaximum());

   fHighGateMarker->linex->SetX1(high_gate_bin_edge);
   fHighGateMarker->linex->SetX2(high_gate_bin_edge);
   fHighGateMarker->linex->SetY1(fProjection->GetMinimum());
   fHighGateMarker->linex->SetY2(fProjection->GetMaximum());

   fProjectionCanvas->GetCanvas()->cd();
   fLowGateMarker->linex->Draw();
   fHighGateMarker->linex->Draw();

   fProjectionCanvas->GetCanvas()->Update();

}

void TBGSubtraction::DoEntry(Long_t){

   TGNumberEntry* te = reinterpret_cast<TGNumberEntry*>(gTQSender);
   int            id = te->WidgetId();

   //When we do an entry, update the corresponding slider...this then performs the gating.
   switch(id) {
   case kGateLowEntry:
      if(fGateEntryLow->GetNumber() > fGateEntryHigh->GetNumber())
         fGateEntryLow->SetNumber(fGateEntryHigh->GetNumber());
         UpdateGateSlider();
         fGateSlider->PositionChanged();
      break;
   case kGateHighEntry:
      if(fGateEntryHigh->GetNumber() < fGateEntryLow->GetNumber())
         fGateEntryHigh->SetNumber(fGateEntryLow->GetNumber());
         UpdateGateSlider();
      break;
   case kBGLowEntry1:
      if(fBGEntryLow1->GetNumber() > fBGEntryHigh1->GetNumber())
         fBGEntryLow1->SetNumber(fBGEntryHigh1->GetNumber());
         UpdateBGSlider1();
      break;
   case kBGHighEntry1:
      if(fBGEntryHigh1->GetNumber() < fBGEntryLow1->GetNumber())
         fBGEntryHigh1->SetNumber(fBGEntryLow1->GetNumber());
         UpdateBGSlider1();
      break;
   case kBGLowEntry2:
      if(fBGEntryLow2->GetNumber() > fBGEntryHigh2->GetNumber())
         fBGEntryLow2->SetNumber(fBGEntryHigh2->GetNumber());
         UpdateBGSlider2();
      break;
   case kBGHighEntry2:
      if(fBGEntryHigh2->GetNumber() < fBGEntryLow2->GetNumber())
         fBGEntryHigh2->SetNumber(fBGEntryLow2->GetNumber());
         UpdateBGSlider2();
      break;
   };
}

void TBGSubtraction::DoGateCanvasZoomed(){
   //If we zoom in on this canvas, we need to update the sliders and Number boxes appropriately.
   Double_t xmin, ymin, xmax, ymax;
   fGateCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
   // The first thing we need to do is set new limits on the number entries
   // Start by getting the old limits, this should be the same as the old histo range
   Double_t old_limit_min  = fPeakLowLimit;
   Double_t old_limit_max  = fPeakHighLimit;

   // Get the old value
   Double_t old_lower_peak_val = fPeakLowValue;
   Double_t old_upper_peak_val = fPeakHighValue;
   Double_t old_peak_val       = fPeakValue;
   // Now maintain the ratio of the position
   Double_t rel_lower_peak_val = (old_lower_peak_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_upper_peak_val = (old_upper_peak_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_peak_val       = (old_peak_val - old_limit_min) / (old_limit_max - old_limit_min);
   // set the new positions
   fPeakLowValue = xmin + rel_lower_peak_val * (xmax - xmin);
   fPeakHighValue = xmin + rel_upper_peak_val * (xmax - xmin);
   fPeakValue = xmin + rel_peak_val * (xmax - xmin);
   // Set the new limits
   fPeakLowLimit = xmin;
   fPeakHighLimit = xmax;
   
   UpdatePeakSliders();
   DrawPeakMarkers();
}

void TBGSubtraction::DoProjectionCanvasZoomed(){
   //If we zoom in on this canvas, we need to update the sliders and Number boxes appropriately.
   Double_t xmin, ymin, xmax, ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
   // The first thing we need to do is set new limits on the number entries
   // Start by getting the old limits, this should be the same as the old histo range
   Double_t old_limit_min = fGateEntryLow->GetNumMin();
   Double_t old_limit_max = fGateEntryLow->GetNumMax();

   // Get the old value
   Double_t old_lower_gate_val = fGateEntryLow->GetNumber();
   Double_t old_upper_gate_val = fGateEntryHigh->GetNumber();
   Double_t old_lower_bg1_val   = fBGEntryLow1->GetNumber();
   Double_t old_upper_bg1_val   = fBGEntryHigh1->GetNumber();
   Double_t old_lower_bg2_val   = fBGEntryLow2->GetNumber();
   Double_t old_upper_bg2_val   = fBGEntryHigh2->GetNumber();
   // Now maintain the ratio of the position
   Double_t rel_lower_gate_val = (old_lower_gate_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_upper_gate_val = (old_upper_gate_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_lower_bg1_val   = (old_lower_bg1_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_upper_bg1_val   = (old_upper_bg1_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_lower_bg2_val   = (old_lower_bg2_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_upper_bg2_val   = (old_upper_bg2_val - old_limit_min) / (old_limit_max - old_limit_min);
   // set the new positions
   fGateEntryLow->SetNumber(xmin + rel_lower_gate_val * (xmax - xmin));
   fGateEntryHigh->SetNumber(xmin + rel_upper_gate_val * (xmax - xmin));
   fBGEntryLow1->SetNumber(xmin + rel_lower_bg1_val * (xmax - xmin));
   fBGEntryHigh1->SetNumber(xmin + rel_upper_bg1_val * (xmax - xmin));
   fBGEntryLow2->SetNumber(xmin + rel_lower_bg2_val * (xmax - xmin));
   fBGEntryHigh2->SetNumber(xmin + rel_upper_bg2_val * (xmax - xmin));
   // Set the new limits
   fGateEntryLow->SetLimitValues(xmin, xmax);
   fGateEntryHigh->SetLimitValues(xmin, xmax);
   fBGEntryLow1->SetLimitValues(xmin, xmax);
   fBGEntryHigh1->SetLimitValues(xmin, xmax);
   fBGEntryLow2->SetLimitValues(xmin, xmax);
   fBGEntryHigh2->SetLimitValues(xmin, xmax);
   
   UpdateProjectionSliders();
   DrawAllMarkers();


   if((fAutoUpdateCheckButton != nullptr) && fAutoUpdateCheckButton->IsDown()){ 
      DoAllGates(); //Figure out how to make this happen on histo zooms and not scaling canvas
   }
   //DoGateProjection();
}

void TBGSubtraction::DoAllGates(){ 
   MakeGateHisto();
   MakeBGHisto1();
   MakeBGHisto2();
}


void TBGSubtraction::MakeGateHisto(){
   if(fGateHist != nullptr) {
      delete fGateHist;
   }
   
   const char* proj_name = Form("gate_%d_%d", static_cast<Int_t>(fGateEntryLow->GetNumber()),
                                static_cast<Int_t>(fGateEntryHigh->GetNumber()));

   fGateAxis = fAxisCombo->GetSelected();

   if(fGateAxis == 0) {
      fGateHist = fMatrix->ProjectionX(proj_name, fMatrix->GetYaxis()->FindBin(fGateSlider->GetMinPosition()),
                                       fMatrix->GetYaxis()->FindBin(fGateSlider->GetMaxPosition()));
   } else {
      fGateHist = fMatrix->ProjectionY(proj_name, fMatrix->GetXaxis()->FindBin(fGateSlider->GetMinPosition()),
                                       fMatrix->GetXaxis()->FindBin(fGateSlider->GetMaxPosition()));
   }
   fGateHist->Sumw2();
}

void TBGSubtraction::MakeBGHisto1(){
   Double_t xmin, ymin, xmax, ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
   
   if(fBGHist1 != nullptr) {
      delete fBGHist1;
   }

   const char* bg_name1 =
      Form("bg1_%d_%d", static_cast<Int_t>(fBGEntryLow1->GetNumber()), static_cast<Int_t>(fBGEntryHigh1->GetNumber()));
   
   if(fGateAxis == 0) {
      fBGHist1 = fMatrix->ProjectionX(bg_name1, fMatrix->GetYaxis()->FindBin(fBGSlider1->GetMinPosition()),
                                     fMatrix->GetYaxis()->FindBin(fBGSlider1->GetMaxPosition()));
   } else {
      fBGHist1 = fMatrix->ProjectionY(bg_name1, fMatrix->GetXaxis()->FindBin(fBGSlider1->GetMinPosition()),
                                     fMatrix->GetXaxis()->FindBin(fBGSlider1->GetMaxPosition()));
   }

   fBGHist1->Sumw2();
}

void TBGSubtraction::MakeBGHisto2(){
   if(fBGHist2 != nullptr) {
      delete fBGHist2;
   }

   const char* bg_name2 =
      Form("bg2_%d_%d", static_cast<Int_t>(fBGEntryLow2->GetNumber()), static_cast<Int_t>(fBGEntryHigh2->GetNumber()));
   
   if(fGateAxis == 0) {
      fBGHist2 = fMatrix->ProjectionX(bg_name2, fMatrix->GetYaxis()->FindBin(fBGSlider2->GetMinPosition()),
                                     fMatrix->GetYaxis()->FindBin(fBGSlider2->GetMaxPosition()));
   } else {
      fBGHist2 = fMatrix->ProjectionY(bg_name2, fMatrix->GetXaxis()->FindBin(fBGSlider2->GetMinPosition()),
                                     fMatrix->GetXaxis()->FindBin(fBGSlider2->GetMaxPosition()));
   }

   fBGHist2->Sumw2();
}

void TBGSubtraction::DoGateProjection(){
   TH1*     bg_hist       = nullptr;
   Double_t under_peak_bg = 0.0;
   //Make sure one of the two buttons are checked
   if(((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()) || ((fBGCheckButton2 != nullptr) && fBGCheckButton2->IsDown())) {
      bg_hist       = fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber());
      under_peak_bg = bg_hist->Integral(bg_hist->FindBin(fGateSlider->GetMinPosition()),
                                        bg_hist->FindBin(fGateSlider->GetMaxPosition()));
   }

   //Add the two background regions together;
   Double_t bg_region = 0.0;
   if((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()){
      bg_region += fProjection->Integral(fProjection->FindBin(fBGSlider1->GetMinPosition()),
                                              fProjection->FindBin(fBGSlider1->GetMaxPosition()));
   }
   if((fBGCheckButton2 != nullptr) && fBGCheckButton2->IsDown()){
      bg_region += fProjection->Integral(fProjection->FindBin(fBGSlider2->GetMinPosition()),
                                              fProjection->FindBin(fBGSlider2->GetMaxPosition()));
   }
   Double_t ratio = 0;

   if(bg_region != 0) {
      ratio = under_peak_bg / bg_region;
   }

   Int_t first_bin = -1, last_bin = -1;
   if(fSubtractedHist != nullptr) {
      // Get old axis range
      first_bin = fSubtractedHist->GetXaxis()->GetFirst();
      last_bin  = fSubtractedHist->GetXaxis()->GetLast();
      delete fSubtractedHist;
   }
   const char* sub_name = Form("%s_%s", fGateHist->GetName(), fBGHist1->GetName()); //TO DO: Come up with better naming
   fSubtractedHist      = static_cast<TH1*>(fGateHist->Clone(sub_name));
   if((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()) {
      fSubtractedHist->Add(fBGHist1, -ratio);
   }
   if((fBGCheckButton2 != nullptr) && fBGCheckButton2->IsDown()) {
      fSubtractedHist->Add(fBGHist2, -ratio);
   }
   fGateCanvas->GetCanvas()->cd();
   if(fSubtractedHist != nullptr) {
      fSubtractedHist->GetXaxis()->SetRange(first_bin, last_bin);
      fSubtractedHist->Draw("hist");
   }
 //  DrawPeakMarkers();
   fGateCanvas->GetCanvas()->Update();


}

void TBGSubtraction::DoProjection(){
   if(fGateHist != nullptr) {
      delete fGateHist;
   }
   
   const char* proj_name = Form("gate_%d_%d", static_cast<Int_t>(fGateEntryLow->GetNumber()),
                                static_cast<Int_t>(fGateEntryHigh->GetNumber()));

   fGateAxis = fAxisCombo->GetSelected();

   if(fGateAxis == 0) {
      fGateHist = fMatrix->ProjectionX(proj_name, fMatrix->GetYaxis()->FindBin(fGateSlider->GetMinPosition()),
                                       fMatrix->GetYaxis()->FindBin(fGateSlider->GetMaxPosition()));
   } else {
      fGateHist = fMatrix->ProjectionY(proj_name, fMatrix->GetXaxis()->FindBin(fGateSlider->GetMinPosition()),
                                       fMatrix->GetXaxis()->FindBin(fGateSlider->GetMaxPosition()));
   }
   fGateHist->Sumw2();

   if(fBGHist1 != nullptr) {
      delete fBGHist1;
   }
   if(fBGHist2 != nullptr) {
      delete fBGHist2;
   }
   const char* bg_name1 =
      Form("bg1_%d_%d", static_cast<Int_t>(fBGEntryLow1->GetNumber()), static_cast<Int_t>(fBGEntryHigh1->GetNumber()));
   const char* bg_name2 =
      Form("bg2_%d_%d", static_cast<Int_t>(fBGEntryLow2->GetNumber()), static_cast<Int_t>(fBGEntryHigh2->GetNumber()));

   if(fGateAxis == 0) {
      fBGHist1 = fMatrix->ProjectionX(bg_name1, fMatrix->GetYaxis()->FindBin(fBGSlider1->GetMinPosition()),
                                     fMatrix->GetYaxis()->FindBin(fBGSlider1->GetMaxPosition()));
      fBGHist2 = fMatrix->ProjectionX(bg_name1, fMatrix->GetYaxis()->FindBin(fBGSlider2->GetMinPosition()),
                                     fMatrix->GetYaxis()->FindBin(fBGSlider2->GetMaxPosition()));
   } else {
      fBGHist1 = fMatrix->ProjectionY(bg_name1, fMatrix->GetXaxis()->FindBin(fBGSlider1->GetMinPosition()),
                                     fMatrix->GetXaxis()->FindBin(fBGSlider1->GetMaxPosition()));
      fBGHist2 = fMatrix->ProjectionY(bg_name2, fMatrix->GetXaxis()->FindBin(fBGSlider2->GetMinPosition()),
                                     fMatrix->GetXaxis()->FindBin(fBGSlider2->GetMaxPosition()));
   }

   fBGHist1->Sumw2();
   fBGHist2->Sumw2();
   TH1*     bg_hist       = nullptr;
   Double_t under_peak_bg = 0.0;
   //Make sure one of the two buttons are checked
   if(((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()) || ((fBGCheckButton2 != nullptr) && fBGCheckButton2->IsDown())) {
      bg_hist       = fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber());
      under_peak_bg = bg_hist->Integral(bg_hist->FindBin(fGateSlider->GetMinPosition()),
                                        bg_hist->FindBin(fGateSlider->GetMaxPosition()));
   }

   //Add the two background regions together;
   Double_t bg_region = 0.0;
   if((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()){
      bg_region += fProjection->Integral(fProjection->FindBin(fBGSlider1->GetMinPosition()),
                                              fProjection->FindBin(fBGSlider1->GetMaxPosition()));
   }
   if((fBGCheckButton2 != nullptr) && fBGCheckButton2->IsDown()){
      bg_region += fProjection->Integral(fProjection->FindBin(fBGSlider2->GetMinPosition()),
                                              fProjection->FindBin(fBGSlider2->GetMaxPosition()));
   }
   Double_t ratio = 0;

   if(bg_region != 0) {
      ratio = under_peak_bg / bg_region;
   }

   Int_t first_bin = -1, last_bin = -1;
   if(fSubtractedHist != nullptr) {
      // Get old axis range
      first_bin = fSubtractedHist->GetXaxis()->GetFirst();
      last_bin  = fSubtractedHist->GetXaxis()->GetLast();
      delete fSubtractedHist;
   }
   const char* sub_name = Form("%s_%s", fGateHist->GetName(), fBGHist1->GetName()); //TO DO: Come up with better naming
   
   fSubtractedHist      = static_cast<TH1*>(fGateHist->Clone(sub_name));
   if((fBGCheckButton1 != nullptr) && fBGCheckButton1->IsDown()) {
      fSubtractedHist->Add(fBGHist1, -ratio);
   }
   if((fBGCheckButton2 != nullptr) && fBGCheckButton2->IsDown()) {
      fSubtractedHist->Add(fBGHist2, -ratio);
   }
   
   fGateCanvas->GetCanvas()->cd();
   if(fSubtractedHist != nullptr) {
      fSubtractedHist->GetXaxis()->SetRange(first_bin, last_bin);
      fSubtractedHist->Draw("hist");
   }
   DrawPeakMarkers();
   fGateCanvas->GetCanvas()->Update();
}

void TBGSubtraction::DrawOnNewCanvas()
{
   if(fSubtractedHist != nullptr) {
      auto* g = new TCanvas;
      g->cd();
      fSubtractedHist->DrawCopy();
      g->Update();
   }
}

void TBGSubtraction::WriteHistograms()
{
   // Find if there is a file name
   const char* file_name = fWrite2FileName->GetText();

   if(file_name == nullptr) {
      std::cout<<"Please enter a file name"<<std::endl;
      return;
   }

   TFile f(file_name, "Update");
   std::cout<<"Writing "<<fHistogramDescription->GetText()<<" histograms to "<<f.GetName()<<std::endl;
   if(fSubtractedHist != nullptr) {
      if(fHistogramDescription->GetText() != nullptr) {
         fSubtractedHist->SetTitle(fHistogramDescription->GetText());
      }
      fSubtractedHist->Write();
   }

   if(fBGHist1 != nullptr) {
      if(fHistogramDescription->GetText() != nullptr) {
         fBGHist1->SetTitle(Form("%s Background 1", fHistogramDescription->GetText()));
      }

      fBGHist1->Write();
   }

   if(fBGHist2 != nullptr) {
      if(fHistogramDescription->GetText() != nullptr) {
         fBGHist2->SetTitle(Form("%s Background 2", fHistogramDescription->GetText()));
      }

      fBGHist2->Write();
   }

   if(fGateHist != nullptr) {
      if(fHistogramDescription->GetText() != nullptr) {
         fGateHist->SetTitle(Form("%s Gate only", fHistogramDescription->GetText()));
      }

      fGateHist->Write();
   }
}

void TBGSubtraction::GateStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected)
{
   fGateCanvas->GetCanvas()->cd();
   StatusInfo(event, px, py, selected);
}

void TBGSubtraction::ProjectionStatusInfo(Int_t event, Int_t px, Int_t py, TObject* selected)
{
   fProjectionCanvas->GetCanvas()->cd();
   StatusInfo(event, px, py, selected);
}

void TBGSubtraction::StatusInfo(Int_t, Int_t px, Int_t py, TObject* selected)
{
   fProjectionStatus->SetText(selected->GetName(), 0);
   fProjectionStatus->SetText(selected->GetObjectInfo(px, py), 1);
}

void TBGSubtraction::SetStatusFromUpdateCheckButton(){
   //Here we want to check the status of the update checkbox, and then do the following:
   if((fAutoUpdateCheckButton == nullptr) || !fAutoUpdateCheckButton->IsDown()){
      //1. the checkbox has been turned off, this means we want to disable anything that can
      //mess with the user.
      fPeakFitButton->SetEnabled(false);
      fWrite2FileButton->SetEnabled(false);
   }
   else{
      //2. the checkbox has been turned on, this means we want to enable features as well as take all
      //of the current gates
      fPeakFitButton->SetEnabled(true);
      fWrite2FileButton->SetEnabled(true);
      DoAllGates();
      DoGateProjection();
   }

}
