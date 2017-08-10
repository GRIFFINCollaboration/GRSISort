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
     fProjection(nullptr), fGateHist(nullptr), fBGHist(nullptr), fSubtractedHist(nullptr), fGateSlider(nullptr),
     fBGSlider(nullptr), fPeakSlider(nullptr), fBGParamEntry(nullptr), fBGCheckButton(nullptr), fPeakSkewCheckButton(nullptr), fBly(nullptr), 
     fBly1(nullptr), fGateFrame(nullptr), fProjectionFrame(nullptr), fAxisCombo(nullptr), fLowGateMarker(nullptr),
     fHighGateMarker(nullptr), fLowBGMarker(nullptr),  fHighBGMarker(nullptr), fLowPeakMarker(nullptr), fHighPeakMarker(nullptr),
     fPeakMarker(nullptr), fGateAxis(0), fForceUpdate(true), fPeakFit(nullptr)
{

   // fProjection = (TH1*)(fMatrix->ProjectionX()->Clone());

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
   BuildInterface();
   MakeConnections();
   InitializeInterface();
   //DoDraw();
   // SetTreeName(treeName);
   fCurrentFile = TFile::CurrentFile();
}

void TBGSubtraction::MakeConnections(){

   //Connect Canvases to recognize that they have been zoomed
   fProjectionCanvas->GetCanvas()->Connect("RangeAxisChanged()", "TBGSubtraction", this, "DoProjectionCanvasZoomed()");

   fGateCanvas->GetCanvas()->Connect("RangeAxisChanged()", "TBGSubtraction", this, "DoGateCanvasZoomed()");
   fGateCanvas->GetCanvas()->Connect("RangeAxisChanged()", "TBGSubtraction", this, "UpdatePeakSliders()");
   fGateCanvas->GetCanvas()->Connect("RangeAxisChanged()", "TBGSubtraction", this, "DrawPeakMarkers()");
   fGateCanvas->GetCanvas()->Connect("RangeAxisChanged()", "TBGSubtraction", this, "DrawPeak()");


   //Connect Status Info to canvases
 //  fProjectionCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBGSubtraction", this,
 //                                          "ProjectionStatusInfo(Int_t,Int_t,Int_t,TObject*)");
   fGateCanvas->GetCanvas()->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "TBGSubtraction", this,
                                     "GateStatusInfo(Int_t,Int_t,Int_t,TObject*)");

   //Connect the sliding of sliders
   //I'm storing the "true" information in the entry boxes. This is what we refer back to for
   //the actual value of the sliders, etc.
   fGateSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fGateSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DrawGateMarkers()");
   fGateSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoGating()");
   fBGSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fBGSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DrawBGMarkers()");
   fBGSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoGating()");

   fPeakSlider->Connect("PointerPositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fPeakSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fPeakSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DrawPeakMarkers()");
   fPeakSlider->Connect("PointerPositionChanged()", "TBGSubtraction", this, "DrawPeakMarkers()");

   //Connect the clicking of buttons
   fPeakFitButton->Connect("Clicked()", "TBGSubtraction", this, "DoPeakFit()");

   fBGCheckButton->Connect("Clicked()","TBGSubtraction",this,"UpdateBackground()");
   fBGCheckButton->Connect("Clicked()","TBGSubtraction",this,"DoGating()");

   fWrite2FileButton->Connect("Clicked()", "TBGSubtraction", this, "WriteHistograms()");

   //Connect the axis combo box
   fAxisCombo->Connect("Selected(Int_t,Int_t)", "TBGSubtraction", this, "AxisComboSelected()");

   //Connect the Gate entries so that if you type a value in they do the proper things
   //Everyhting that is updated sets the value of these entries, which trickles down and fixes everything else
   fGateEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fGateEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "UpdateProjectionSliders()");
   fGateEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DrawGateMarkers()");
   fGateEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fGateEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "UpdateProjectionSliders()");
   fGateEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DrawGateMarkers()");
   fBGEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "UpdateProjectionSliders()");
   fBGEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DrawBGMarkers()");
   fBGEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "UpdateProjectionSliders()");
   fBGEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DrawBGMarkers()");

   //Connect the bg paramater entry to do the proper thing
   fBGParamEntry->Connect("ValueSet(Long_t)","TBGSubtraction",this,"UpdateBackground()");
   fBGParamEntry->Connect("ValueSet(Long_t)","TBGSubtraction",this,"DoGating()");

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

   //Set Up entry and sliders for doing gating.
   Double_t xmin, ymin, xmax, ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
   Double_t x_width = xmax - xmin;

   fGateEntryLow->SetNumber(xmin+def_gate_low*x_width);
   fGateEntryHigh->SetNumber(xmax-(1.0-def_gate_high)*x_width);
   fGateEntryLow->SetLimitValues(xmin,xmax);
   fGateEntryHigh->SetLimitValues(xmin,xmax);
   
   fBGEntryLow->SetNumber(xmin+def_bg_low*x_width);
   fBGEntryHigh->SetNumber(xmax-(1.0-def_bg_high)*x_width);
   fBGEntryLow->SetLimitValues(xmin,xmax);
   fBGEntryHigh->SetLimitValues(xmin,xmax);
   
   UpdateProjectionSliders();
   DoGating();

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

   ResetInterface();
}

void TBGSubtraction::BuildInterface()
{
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

   fBGEntryFrame = new TGHorizontalFrame(fProjectionFrame, 200, 200);
   fBGEntryLow   = new TGNumberEntry(fBGEntryFrame, 0, 3, kBGLowEntry, TGNumberFormat::kNESReal,
                                   TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);

   fBGEntryHigh = new TGNumberEntry(fBGEntryFrame, 0, 3, kBGHighEntry, TGNumberFormat::kNESReal,
                                    TGNumberFormat::kNEAAnyNumber, TGNumberFormat::kNELLimitMinMax, xmin, xmax);

   fBGSlider = new TGDoubleHSlider(fProjectionFrame, 100, kDoubleScaleBoth, kBGSlider, kHorizontalFrame);

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

   fBGCheckButton = new TGCheckButton(fBGParamFrame, "BG On", kBGCheckButton);
   fBGCheckButton->SetState(kButtonDown);

   fDescriptionFrame     = new TGHorizontalFrame(fGateFrame, 200, 200);
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
   fBGParamFrame->AddFrame(fBGCheckButton, fBly);
   fBGParamFrame->AddFrame(fBGParamLabel, fBly);
   fBGParamFrame->AddFrame(fBGParamEntry, fBly);

   fGateEntryFrame->AddFrame(fGateEntryLow, fBly);
   fGateEntryFrame->AddFrame(fGateEntryHigh, fBly);

   fPeakFitFrame->AddFrame(fPeakSkewCheckButton,fBly);
   fPeakFitFrame->AddFrame(fPeakFitButton,fBly);

   fBGEntryFrame->AddFrame(fBGEntryLow, fBly);
   fBGEntryFrame->AddFrame(fBGEntryHigh, fBly);

   fDescriptionFrame->AddFrame(fHistogramDescription, fBly);

   fButtonFrame->AddFrame(fWrite2FileName, fBly);
   fButtonFrame->AddFrame(fWrite2FileButton, fBly);

   //Build left and right frames
   fProjectionFrame->AddFrame(fProjectionCanvas, fLayoutCanvases);
   fProjectionFrame->AddFrame(fProjectionStatus, fBly);
   fProjectionFrame->AddFrame(fGateSlider, fBly);
   fProjectionFrame->AddFrame(fBGSlider, fBly);
   fProjectionFrame->AddFrame(fGateEntryFrame, fBly);
   fProjectionFrame->AddFrame(fBGEntryFrame, fBly);

   fGateFrame->AddFrame(fGateCanvas, fLayoutCanvases);
   fGateFrame->AddFrame(fPeakSlider,fBly);
   fGateFrame->AddFrame(fPeakFitFrame,fBly);
   fGateFrame->AddFrame(fBGParamFrame, fLayoutParam);
   fGateFrame->AddFrame(fDescriptionFrame, fLayoutParam);
   fGateFrame->AddFrame(fButtonFrame, fLayoutParam);

   AddFrame(fProjectionFrame, fBly1);
   AddFrame(fGateFrame, fBly1);

   // (fNumber->GetNumberEntry())->Connect("ReturnPressed()", "MyMainFrame", this,
   //                                             "DoSetlabel()");

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
   if(fLowBGMarker != nullptr) {
      delete fLowBGMarker;
   }
   if(fHighBGMarker != nullptr) {
      delete fHighBGMarker;
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
   if((fBGCheckButton != nullptr) && fBGCheckButton->IsDown()) {
      fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber())->Draw("same");
   }
   else if((fBGCheckButton != nullptr) && !fBGCheckButton->IsDown()){
      fProjection->Draw();
   }
   fCanvas->Update();
   fGateCanvas->GetCanvas()->cd();
}

void TBGSubtraction::DoPeakFit()
{
   if(fPeakFit) fPeakFit->Delete(); fPeakFit = nullptr;
   
   fPeakFit = new TPeak(fPeakValue, fPeakLowValue, fPeakHighValue);
   fGateCanvas->GetCanvas()->cd();
   if((fPeakSkewCheckButton != nullptr) && fPeakSkewCheckButton->IsDown()) {
      fPeakFit->Fit(fSubtractedHist,"Q");
      fPeakFit->ReleaseParameter(3);
      fPeakFit->ReleaseParameter(4);
   }
   fPeakFit->Fit(fSubtractedHist);
   fGateCanvas->GetCanvas()->Update();
}

void TBGSubtraction::DrawPeak(){
   if(fPeakFit)
      fPeakFit->Draw("same");
}

void TBGSubtraction::ClickedBGButton()
{
   if((fBGCheckButton != nullptr) && fBGCheckButton->IsDown()) {
      fBGParamEntry->SetState(true);
   } else {
      fBGParamEntry->SetState(false);
   }
}

void TBGSubtraction::UpdateProjectionSliders(){
   //Something has happened, so we have to let the sliders know about it
   //This is as simple as checking the status of the "master" entry boxes
   fGateSlider->SetPosition(fGateEntryLow->GetNumber(),fGateEntryHigh->GetNumber());
   fGateSlider->SetRange(fGateEntryLow->GetNumMin(), fGateEntryHigh->GetNumMax());

   fBGSlider->SetPosition(fBGEntryLow->GetNumber(), fBGEntryHigh->GetNumber());
   fBGSlider->SetRange(fBGEntryLow->GetNumMin(), fBGEntryHigh->GetNumMax());

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
         break;
      case kBGSlider:
         fBGEntryLow->SetNumber(fBGSlider->GetMinPosition());
         fBGEntryHigh->SetNumber(fBGSlider->GetMaxPosition());
         break;    
      case kPeakSlider:
         fPeakLowValue  = fPeakSlider->GetMinPosition();
         fPeakHighValue = fPeakSlider->GetMaxPosition();
         fPeakValue     = fPeakSlider->GetPointerPosition();
         return;
         break;
   };
}

void TBGSubtraction::AxisComboSelected()
{
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

void TBGSubtraction::DrawBGMarkers(){
   if(fLowBGMarker == nullptr) {
      fLowBGMarker = new GMarker();
   }
   if(fHighBGMarker == nullptr) {
      fHighBGMarker = new GMarker();
   }
   fLowBGMarker->localx       = fBGEntryLow->GetNumber();
   fHighBGMarker->localx      = fBGEntryHigh->GetNumber();
   fLowBGMarker->binx      = fProjection->GetXaxis()->FindBin(fLowBGMarker->localx);
   fHighBGMarker->binx     = fProjection->GetXaxis()->FindBin(fHighBGMarker->localx);
   double low_bg_bin_edge    = fProjection->GetXaxis()->GetBinLowEdge(fLowBGMarker->binx);
   double high_bg_bin_edge   = fProjection->GetXaxis()->GetBinLowEdge(fHighBGMarker->binx);
   
   if((fLowBGMarker->linex) == nullptr) {
      fLowBGMarker->linex =
         new TLine(low_bg_bin_edge, fProjection->GetMinimum(), low_bg_bin_edge, fProjection->GetMaximum());
      fLowBGMarker->SetColor(kBlue);
   }

   if((fHighBGMarker->linex) == nullptr) {
      fHighBGMarker->linex =
         new TLine(high_bg_bin_edge, fProjection->GetMinimum(), high_bg_bin_edge, fProjection->GetMaximum());
      fHighBGMarker->SetColor(kBlue);
   }
   fLowBGMarker->linex->SetX1(low_bg_bin_edge);
   fLowBGMarker->linex->SetX2(low_bg_bin_edge);
   fLowBGMarker->linex->SetY1(fProjection->GetMinimum());
   fLowBGMarker->linex->SetY2(fProjection->GetMaximum());

   fHighBGMarker->linex->SetX1(high_bg_bin_edge);
   fHighBGMarker->linex->SetX2(high_bg_bin_edge);
   fHighBGMarker->linex->SetY1(fProjection->GetMinimum());
   fHighBGMarker->linex->SetY2(fProjection->GetMaximum());

   fProjectionCanvas->GetCanvas()->cd();
   fLowBGMarker->linex->Draw();
   fHighBGMarker->linex->Draw();
   fProjectionCanvas->GetCanvas()->Update();

}

void TBGSubtraction::DrawGateMarkers()
{
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

   switch(id) {
   case kGateLowEntry:
      if(fGateEntryLow->GetNumber() > fGateEntryHigh->GetNumber())
         fGateEntryLow->SetNumber(fGateEntryHigh->GetNumber());
      break;
   case kGateHighEntry:
      if(fGateEntryHigh->GetNumber() < fGateEntryLow->GetNumber())
         fGateEntryHigh->SetNumber(fGateEntryLow->GetNumber());
      break;
   case kBGLowEntry:
      if(fBGEntryLow->GetNumber() > fBGEntryHigh->GetNumber())
         fBGEntryLow->SetNumber(fBGEntryHigh->GetNumber());
      break;
   case kBGHighEntry:
      if(fBGEntryHigh->GetNumber() < fBGEntryLow->GetNumber())
         fBGEntryHigh->SetNumber(fBGEntryLow->GetNumber());
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
   Double_t old_lower_bg_val   = fBGEntryLow->GetNumber();
   Double_t old_upper_bg_val   = fBGEntryHigh->GetNumber();
   // Now maintain the ratio of the position
   Double_t rel_lower_gate_val = (old_lower_gate_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_upper_gate_val = (old_upper_gate_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_lower_bg_val   = (old_lower_bg_val - old_limit_min) / (old_limit_max - old_limit_min);
   Double_t rel_upper_bg_val   = (old_upper_bg_val - old_limit_min) / (old_limit_max - old_limit_min);
   // set the new positions
   fGateEntryLow->SetNumber(xmin + rel_lower_gate_val * (xmax - xmin));
   fGateEntryHigh->SetNumber(xmin + rel_upper_gate_val * (xmax - xmin));
   fBGEntryLow->SetNumber(xmin + rel_lower_bg_val * (xmax - xmin));
   fBGEntryHigh->SetNumber(xmin + rel_upper_bg_val * (xmax - xmin));
   // Set the new limits
   fGateEntryLow->SetLimitValues(xmin, xmax);
   fGateEntryHigh->SetLimitValues(xmin, xmax);
   fBGEntryLow->SetLimitValues(xmin, xmax);
   fBGEntryHigh->SetLimitValues(xmin, xmax);

   fGateEntryLow->ValueSet(1);
   fGateEntryHigh->ValueSet(1);
   fBGEntryLow->ValueSet(1);
   fBGEntryHigh->ValueSet(1);
}

void TBGSubtraction::DoGating(){
   Double_t xmin, ymin, xmax, ymax;
   if(fGateHist != nullptr) {
      delete fGateHist;
   }
   
   fProjectionCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
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

   if(fBGHist != nullptr) {
      delete fBGHist;
   }
   const char* bg_name =
      Form("bg_%d_%d", static_cast<Int_t>(fBGEntryLow->GetNumber()), static_cast<Int_t>(fBGEntryHigh->GetNumber()));

   if(fGateAxis == 0) {
      fBGHist = fMatrix->ProjectionX(bg_name, fMatrix->GetYaxis()->FindBin(fBGSlider->GetMinPosition()),
                                     fMatrix->GetYaxis()->FindBin(fBGSlider->GetMaxPosition()));
   } else {
      fBGHist = fMatrix->ProjectionY(bg_name, fMatrix->GetXaxis()->FindBin(fBGSlider->GetMinPosition()),
                                     fMatrix->GetXaxis()->FindBin(fBGSlider->GetMaxPosition()));
   }

   fBGHist->Sumw2();
   TH1*     bg_hist       = nullptr;
   Double_t under_peak_bg = 0.0;
   if((fBGCheckButton != nullptr) && fBGCheckButton->IsDown()) {
      bg_hist       = fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber());
      under_peak_bg = bg_hist->Integral(bg_hist->FindBin(fGateSlider->GetMinPosition()),
                                        bg_hist->FindBin(fGateSlider->GetMaxPosition()));
   }

   Double_t bg_region = fProjection->Integral(fProjection->FindBin(fBGSlider->GetMinPosition()),
                                              fProjection->FindBin(fBGSlider->GetMaxPosition()));
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
   const char* sub_name = Form("%s_%s", fGateHist->GetName(), fBGHist->GetName());
   fSubtractedHist      = static_cast<TH1*>(fGateHist->Clone(sub_name));
   if((fBGCheckButton != nullptr) && fBGCheckButton->IsDown()) {
      fSubtractedHist->Add(fBGHist, -ratio);
   }
   fGateCanvas->GetCanvas()->cd();
   if(fSubtractedHist != nullptr) {
      fSubtractedHist->GetXaxis()->SetRange(first_bin, last_bin);
      fSubtractedHist->Draw("hist");
   }
 //  DrawPeakMarkers();
   fGateCanvas->GetCanvas()->Update();

}

void TBGSubtraction::DoProjection()
{
   Double_t xmin, ymin, xmax, ymax;
   if(fGateHist != nullptr) {
      delete fGateHist;
   }
   
   fProjectionCanvas->GetCanvas()->GetRange(xmin, ymin, xmax, ymax);
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

   if(fBGHist != nullptr) {
      delete fBGHist;
   }
   const char* bg_name =
      Form("bg_%d_%d", static_cast<Int_t>(fBGEntryLow->GetNumber()), static_cast<Int_t>(fBGEntryHigh->GetNumber()));

   if(fGateAxis == 0) {
      fBGHist = fMatrix->ProjectionX(bg_name, fMatrix->GetYaxis()->FindBin(fBGSlider->GetMinPosition()),
                                     fMatrix->GetYaxis()->FindBin(fBGSlider->GetMaxPosition()));
   } else {
      fBGHist = fMatrix->ProjectionY(bg_name, fMatrix->GetXaxis()->FindBin(fBGSlider->GetMinPosition()),
                                     fMatrix->GetXaxis()->FindBin(fBGSlider->GetMaxPosition()));
   }

   fBGHist->Sumw2();
   TH1*     bg_hist       = nullptr;
   Double_t under_peak_bg = 0.0;
   if((fBGCheckButton != nullptr) && fBGCheckButton->IsDown()) {
      bg_hist       = fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber());
      under_peak_bg = bg_hist->Integral(bg_hist->FindBin(fGateSlider->GetMinPosition()),
                                        bg_hist->FindBin(fGateSlider->GetMaxPosition()));
   }

   Double_t bg_region = fProjection->Integral(fProjection->FindBin(fBGSlider->GetMinPosition()),
                                              fProjection->FindBin(fBGSlider->GetMaxPosition()));
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
   const char* sub_name = Form("%s_%s", fGateHist->GetName(), fBGHist->GetName());
   fSubtractedHist      = static_cast<TH1*>(fGateHist->Clone(sub_name));
   if((fBGCheckButton != nullptr) && fBGCheckButton->IsDown()) {
      fSubtractedHist->Add(fBGHist, -ratio);
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

   if(fBGHist != nullptr) {
      if(fHistogramDescription->GetText() != nullptr) {
         fBGHist->SetTitle(Form("%s Background", fHistogramDescription->GetText()));
      }

      fBGHist->Write();
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
