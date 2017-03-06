#include "TROOT.h"
#include "TBGSubtraction.h"
#include "TGTripleSlider.h"
#include "TGNumberEntry.h"
#include "TInterpreter.h"

/// \cond CLASSIMP
ClassImp(TBGSubtraction)
/// \endcond


TBGSubtraction::TBGSubtraction(TH2* mat) :
   TGMainFrame(0, 10, 10, kHorizontalFrame), fProjectionCanvas(nullptr), fGateCanvas(nullptr),
   fMatrix(mat),fProjection(nullptr), fGateHist(nullptr), fBGHist(nullptr), fSubtractedHist(nullptr), fGateSlider(nullptr), fBGSlider(nullptr), fBGParamEntry(nullptr),
   fBly(nullptr), fBly1(nullptr),
   fGateFrame(nullptr), fProjectionFrame(nullptr),
   fLowGateMarker(nullptr), fHighGateMarker(nullptr), fLowBGMarker(nullptr), fHighBGMarker(nullptr){

  // fProjection = (TH1*)(fMatrix->ProjectionX()->Clone());
   fProjection = fMatrix->ProjectionY();

  // fTree = 0;
   if (!gClient) return;
   gInterpreter->SaveContext();
   BuildInterface();
   //SetTreeName(treeName);
   fCurrentFile = TFile::CurrentFile();

}


void TBGSubtraction::BuildInterface() {
   ULong_t color;
   gClient->GetColorByName("blue",color);
   // Create a main frame

   fProjectionFrame = new TGVerticalFrame(this,200,200); 
   // Create canvas widget
   fProjectionCanvas = new TRootEmbeddedCanvas("ProjectionCanvas",fProjectionFrame,200,200);
   fProjectionCanvas->GetCanvas()->Connect("RangeAxisChanged()", "TBGSubtraction",this,"DoGateCanvasModified()");
   // Create a horizontal frame widget with buttons

   fGateEntryFrame = new TGHorizontalFrame(fProjectionFrame,200,200);

   //We are going to start the limits off at a specific ratio of the frame
   Double_t xmin,ymin,xmax,ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin,ymin,xmax,ymax);
   Double_t x_width = xmax-xmin;

   fGateEntryLow = new TGNumberEntry(fGateEntryFrame,xmin+0.3*x_width,3, kGateLowEntry, TGNumberFormat::kNESReal, 
                                                                        TGNumberFormat::kNEAAnyNumber,
                                                                        TGNumberFormat::kNELLimitMinMax,
                                                                        xmin,xmax);

   fGateEntryHigh = new TGNumberEntry(fGateEntryFrame,xmax-0.6*x_width,3, kGateHighEntry, TGNumberFormat::kNESReal, 
                                                                        TGNumberFormat::kNEAAnyNumber,
                                                                        TGNumberFormat::kNELLimitMinMax,
                                                                        xmin,xmax);
   fGateEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fGateEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");


   fGateSlider = new TGDoubleHSlider(fProjectionFrame,100,kDoubleScaleBoth,kGateSlider,kHorizontalFrame);
   fGateSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fGateSlider->SetRange(xmin, xmax);
   fGateSlider->SetPosition(xmin+0.3*x_width, xmax-0.6*x_width);

   fBGEntryFrame = new TGHorizontalFrame(fProjectionFrame,200,200);
   fBGEntryLow = new TGNumberEntry(fBGEntryFrame,xmin+0.7*x_width,3, kBGLowEntry, TGNumberFormat::kNESReal, 
                                                                        TGNumberFormat::kNEAAnyNumber,
                                                                        TGNumberFormat::kNELLimitMinMax,
                                                                        xmin,xmax);

   fBGEntryHigh = new TGNumberEntry(fBGEntryFrame,xmax-0.2*x_width,3, kBGHighEntry, TGNumberFormat::kNESReal, 
                                                                        TGNumberFormat::kNEAAnyNumber,
                                                                        TGNumberFormat::kNELLimitMinMax,
                                                                        xmin,xmax);
   fBGEntryLow->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");
   fBGEntryHigh->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoEntry(Long_t)");

   fBGSlider = new TGDoubleHSlider(fProjectionFrame,100,kDoubleScaleBoth,kBGSlider,kHorizontalFrame);
   fBGSlider->Connect("PositionChanged()", "TBGSubtraction", this, "DoSlider()");
   fBGSlider->SetRange(xmin, xmax);
   fBGSlider->SetPosition(xmin+0.7*x_width, xmax-0.2*x_width);
   fProjectionFrame->Resize(100,200);

   fGateFrame = new TGVerticalFrame(this,200,200); 
   //fGateCanvas = new TRootEmbeddedCanvas("GateCanvas",this,200,200);
   fGateCanvas = new TRootEmbeddedCanvas("GateCanvas",fGateFrame,200,200);

   fBGParamFrame = new TGHorizontalFrame(fGateFrame,200,200);
   fBGParamLabel = new TGLabel(fBGParamFrame, "Background:" );
   fBGParamEntry = new TGNumberEntry(fBGParamFrame, 20, 4, kBGParamEntry,TGNumberFormat::kNESInteger,    //style
                                                      TGNumberFormat::kNEANonNegative,              //input value filter
                                                      TGNumberFormat::kNELLimitMin,                //specify limits
                                                      1,1.);                                       //limit values
   fBGParamEntry->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoDraw()");
   fBGParamEntry->Connect("ValueSet(Long_t)", "TBGSubtraction", this, "DoProjection()");


   fButtonFrame = new TGHorizontalFrame(fGateFrame,200,200);
   fDrawCanvasButton = new TGTextButton(fButtonFrame,"&Draw Canvas");
   fDrawCanvasButton->Connect("Clicked()", "TBGSubtraction", this, "DrawOnNewCanvas()");
   fWrite2FileButton = new TGTextButton(fButtonFrame,"&Write Histograms");
   fWrite2FileButton->Connect("Clicked()", "TBGSubtraction", this, "WriteHistograms()");


   fGateFrame->Resize(100,200);

   fBly = new TGLayoutHints(kLHintsTop | kLHintsCenterX| kLHintsExpandX,1,1,3,1);
   fLayoutCanvases = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 1,1,3,4);
   fLayoutParam = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX ,1,1,3,20);

   fBly1 = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY ,20,10,15,0);


   fBGParamFrame->AddFrame(fBGParamLabel,fBly);
   fBGParamFrame->AddFrame(fBGParamEntry,fBly);
   
   fGateEntryFrame->AddFrame(fGateEntryLow,fBly);
   fGateEntryFrame->AddFrame(fGateEntryHigh,fBly);

   fBGEntryFrame->AddFrame(fBGEntryLow,fBly);
   fBGEntryFrame->AddFrame(fBGEntryHigh,fBly);

   fButtonFrame->AddFrame(fDrawCanvasButton,fBly);
   fButtonFrame->AddFrame(fWrite2FileButton,fBly);

   fProjectionFrame->AddFrame(fProjectionCanvas, fLayoutCanvases);
   fProjectionFrame->AddFrame(fGateSlider,fBly);
   fProjectionFrame->AddFrame(fBGSlider,fBly);
   fProjectionFrame->AddFrame(fGateEntryFrame,fBly);
   fProjectionFrame->AddFrame(fBGEntryFrame,fBly);
   
   fGateFrame->AddFrame(fGateCanvas, fLayoutCanvases);
   fGateFrame->AddFrame(fBGParamFrame, fLayoutParam);
   fGateFrame->AddFrame(fButtonFrame,fLayoutParam);
   
   AddFrame(fProjectionFrame, fBly1);
   AddFrame(fGateFrame,fBly1);

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
   DoDraw();
}

void TBGSubtraction::DoDraw() {
   // Draws function graphics in randomly chosen interval
   TCanvas *fCanvas = fProjectionCanvas->GetCanvas();
   fCanvas->cd();
   fProjection->Draw();
   fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber())->Draw("same");
   fCanvas->Update();
   fGateCanvas->GetCanvas()->cd();

 /*  if(fSubtractedHist)
      fSubtractedHist->Draw();
   fGateCanvas->GetCanvas()->Update();
*/
   static bool first_draw = true;
   if(first_draw){
      Double_t xmin,ymin,xmax,ymax;
      fProjectionCanvas->GetCanvas()->GetRange(xmin,ymin,xmax,ymax);
      Double_t x_width = xmax-xmin;
      fGateEntryLow->SetLimitValues(xmin,xmax);
      fGateEntryHigh->SetLimitValues(xmin,xmax);
      fBGEntryLow->SetLimitValues(xmin,xmax);
      fBGEntryHigh->SetLimitValues(xmin,xmax);
      //set the new positions
      fGateEntryLow->SetNumber(xmin + 0.3*x_width);
      fGateEntryHigh->SetNumber(xmax-0.6*x_width);
      fBGEntryLow->SetNumber(xmin+0.7*x_width);
      fBGEntryHigh->SetNumber(xmax -0.2*x_width);

      fGateEntryLow->ValueSet(1);
      fGateEntryHigh->ValueSet(1);
      fBGEntryLow->ValueSet(1);
      fBGEntryHigh->ValueSet(1);

      //The sliders need to know about this as well
      DoProjection();
      first_draw = false;
   }

}

void TBGSubtraction::DoFit() {
  // TPeak* peak = new TPeak(
}

TBGSubtraction::~TBGSubtraction() {
   // Clean up used widgets: frames, buttons, layout hints
   Cleanup();
   if(fLowBGMarker)     delete fLowBGMarker;
   if(fHighBGMarker)    delete fHighBGMarker;
   if(fLowGateMarker)   delete fLowGateMarker;
   if(fHighGateMarker)  delete fHighGateMarker;
}

void TBGSubtraction::DoSlider(Int_t pos){
// Handle slider widgets.

   Int_t id;
   TGFrame *frm = (TGFrame *) gTQSender;
   if (frm->IsA()->InheritsFrom(TGSlider::Class())) {
      TGSlider *sl = (TGSlider*) frm;
      id = sl->WidgetId();
   } else {
      TGDoubleSlider *sd = (TGDoubleSlider *) frm;
      id = sd->WidgetId();
   }
   char buf[32];
   sprintf(buf, "%d", pos);

   switch (id) {
      case kGateSlider:
      {
            fGateEntryLow->SetNumber(fGateSlider->GetMinPosition());
            fGateEntryHigh->SetNumber(fGateSlider->GetMaxPosition());
            break;
      }
      case kBGSlider:
            fBGEntryLow->SetNumber(fBGSlider->GetMinPosition());
            fBGEntryHigh->SetNumber(fBGSlider->GetMaxPosition());
         };

   DoProjection();
   DrawMarkers();

}

void TBGSubtraction::DrawMarkers(){

   if(!fLowGateMarker) fLowGateMarker = new GMarker();
   if(!fHighGateMarker) fHighGateMarker = new GMarker();
   if(!fLowBGMarker) fLowBGMarker = new GMarker();
   if(!fHighBGMarker) fHighBGMarker = new GMarker();
   
   fLowGateMarker->localx  = fGateSlider->GetMinPosition();
   fHighGateMarker->localx = fGateSlider->GetMaxPosition();
   fLowBGMarker->localx    = fBGSlider->GetMinPosition();
   fHighBGMarker->localx   = fBGSlider->GetMaxPosition();
   fLowGateMarker->binx    = fProjection->GetXaxis()->FindBin(fLowGateMarker->localx);
   fHighGateMarker->binx   = fProjection->GetXaxis()->FindBin(fHighGateMarker->localx);
   fLowBGMarker->binx      = fProjection->GetXaxis()->FindBin(fLowBGMarker->localx);
   fHighBGMarker->binx     = fProjection->GetXaxis()->FindBin(fHighBGMarker->localx);
   
   double low_gate_bin_edge   = fProjection->GetXaxis()->GetBinLowEdge(fLowGateMarker->binx);
   double high_gate_bin_edge  = fProjection->GetXaxis()->GetBinLowEdge(fHighGateMarker->binx);
   double low_bg_bin_edge     = fProjection->GetXaxis()->GetBinLowEdge(fLowBGMarker->binx);
   double high_bg_bin_edge    = fProjection->GetXaxis()->GetBinLowEdge(fHighBGMarker->binx);
   if(!(fLowGateMarker->linex))
      fLowGateMarker->linex = new TLine(low_gate_bin_edge,fProjection->GetMinimum(),low_gate_bin_edge,fProjection->GetMaximum());
 
   if(!(fHighGateMarker->linex))
      fHighGateMarker->linex = new TLine(high_gate_bin_edge,fProjection->GetMinimum(),high_gate_bin_edge,fProjection->GetMaximum());
   
   if(!(fLowBGMarker->linex))
      fLowBGMarker->linex = new TLine(low_bg_bin_edge,fProjection->GetMinimum(),low_bg_bin_edge,fProjection->GetMaximum());
   
   if(!(fHighBGMarker->linex))
      fHighBGMarker->linex = new TLine(high_bg_bin_edge,fProjection->GetMinimum(),high_bg_bin_edge,fProjection->GetMaximum());

   fLowGateMarker->linex->SetX1(low_gate_bin_edge);
   fLowGateMarker->linex->SetX2(low_gate_bin_edge);
   fLowGateMarker->linex->SetY1(fProjection->GetMinimum());
   fLowGateMarker->linex->SetY2(fProjection->GetMaximum());

   fHighGateMarker->linex->SetX1(high_gate_bin_edge);
   fHighGateMarker->linex->SetX2(high_gate_bin_edge);
   fHighGateMarker->linex->SetY1(fProjection->GetMinimum());
   fHighGateMarker->linex->SetY2(fProjection->GetMaximum());
   
   fLowBGMarker->linex->SetX1(low_bg_bin_edge);
   fLowBGMarker->linex->SetX2(low_bg_bin_edge);
   fLowBGMarker->linex->SetY1(fProjection->GetMinimum());
   fLowBGMarker->linex->SetY2(fProjection->GetMaximum());

   fHighBGMarker->linex->SetX1(high_bg_bin_edge);
   fHighBGMarker->linex->SetX2(high_bg_bin_edge);
   fHighBGMarker->linex->SetY1(fProjection->GetMinimum());
   fHighBGMarker->linex->SetY2(fProjection->GetMaximum());
   
   fLowGateMarker->SetColor(kGreen);
   fHighGateMarker->SetColor(kGreen);
   fLowBGMarker->SetColor(kBlue);
   fHighBGMarker->SetColor(kBlue);
   fProjectionCanvas->GetCanvas()->cd();
   fLowGateMarker->linex->Draw();
   fHighGateMarker->linex->Draw();
   fLowBGMarker->linex->Draw();
   fHighBGMarker->linex->Draw();
   fProjectionCanvas->GetCanvas()->Update();

}

void TBGSubtraction::DoEntry(Long_t val /*text*/){


   TGNumberEntry *te = (TGNumberEntry *) gTQSender;
   int id = te->WidgetId();

   switch(id){
      case kGateLowEntry:
         fGateSlider->SetPosition(fGateEntryLow->GetNumber(),fGateSlider->GetMaxPosition());
         fGateSlider->SetRange(fGateEntryLow->GetNumMin(), fGateEntryLow->GetNumMax());
         break;
      case kGateHighEntry:
         fGateSlider->SetPosition(fGateSlider->GetMinPosition(),fGateEntryHigh->GetNumber());
         fGateSlider->SetRange(fGateEntryHigh->GetNumMin(), fGateEntryHigh->GetNumMax());
         break;
      case kBGLowEntry:
         fBGSlider->SetPosition(fBGEntryLow->GetNumber(),fBGSlider->GetMaxPosition());
         fBGSlider->SetRange(fBGEntryLow->GetNumMin(), fBGEntryLow->GetNumMax());
         break;
      case kBGHighEntry:
         fBGSlider->SetPosition(fBGSlider->GetMinPosition(),fBGEntryHigh->GetNumber());
         fBGSlider->SetRange(fBGEntryHigh->GetNumMin(), fBGEntryHigh->GetNumMax());
         break;
   };

   DrawMarkers();

}

void TBGSubtraction::DoGateCanvasModified() {
   Double_t xmin,ymin,xmax,ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin,ymin,xmax,ymax);
   //The first thing we need to do is set new limits on the number entries
   //Start by getting the old limits, this should be the same as the old histo range
   Double_t old_limit_min = fGateEntryLow->GetNumMin();
   Double_t old_limit_max = fGateEntryLow->GetNumMax();

   //Get the old value
   Double_t old_lower_gate_val = fGateEntryLow->GetNumber();
   Double_t old_upper_gate_val = fGateEntryHigh->GetNumber();
   Double_t old_lower_bg_val = fBGEntryLow->GetNumber();
   Double_t old_upper_bg_val = fBGEntryHigh->GetNumber();
   //Now maintain the ratio of the position
   Double_t rel_lower_gate_val = (old_lower_gate_val - old_limit_min)/(old_limit_max - old_limit_min);
   Double_t rel_upper_gate_val = (old_upper_gate_val-old_limit_min)/(old_limit_max - old_limit_min);
   Double_t rel_lower_bg_val = (old_lower_bg_val-old_limit_min)/(old_limit_max - old_limit_min);
   Double_t rel_upper_bg_val = (old_upper_bg_val-old_limit_min)/(old_limit_max - old_limit_min);
   //set the new positions
   fGateEntryLow->SetNumber(xmin+rel_lower_gate_val*(xmax - xmin));
   fGateEntryHigh->SetNumber(xmin+rel_upper_gate_val*(xmax - xmin));
   fBGEntryLow->SetNumber(xmin+rel_lower_bg_val*(xmax - xmin));
   fBGEntryHigh->SetNumber(xmin+rel_upper_bg_val*(xmax - xmin));
   //Set the new limits
   fGateEntryLow->SetLimitValues(xmin,xmax);
   fGateEntryHigh->SetLimitValues(xmin,xmax);
   fBGEntryLow->SetLimitValues(xmin,xmax);
   fBGEntryHigh->SetLimitValues(xmin,xmax);

   fGateEntryLow->ValueSet(1);
   fGateEntryHigh->ValueSet(1);
   fBGEntryLow->ValueSet(1);
   fBGEntryHigh->ValueSet(1);

}

void TBGSubtraction::DoProjection() {
   Double_t xmin,ymin,xmax,ymax;
   fProjectionCanvas->GetCanvas()->GetRange(xmin,ymin,xmax,ymax);
   if(fGateHist)
      delete fGateHist;
   const char* proj_name = Form("gate_%d_%d",(Int_t)(fGateEntryLow->GetNumber()),(Int_t)(fGateEntryHigh->GetNumber()));
   fGateHist = fMatrix->ProjectionX(proj_name,fMatrix->GetYaxis()->FindBin(fGateSlider->GetMinPosition()),fMatrix->GetYaxis()->FindBin(fGateSlider->GetMaxPosition()));

   if(fBGHist)
      delete fBGHist;
   const char* bg_name = Form("bg_%d_%d",(Int_t)(fBGEntryLow->GetNumber()),(Int_t)(fBGEntryHigh->GetNumber()));
   fBGHist = fMatrix->ProjectionX(bg_name,fMatrix->GetYaxis()->FindBin(fBGSlider->GetMinPosition()),fMatrix->GetYaxis()->FindBin(fBGSlider->GetMaxPosition()));
   TH1* bg_hist = fProjection->ShowBackground(fBGParamEntry->GetNumberEntry()->GetIntNumber());
   Double_t under_peak_bg = bg_hist->Integral(bg_hist->FindBin(fGateSlider->GetMinPosition()),bg_hist->FindBin(fGateSlider->GetMaxPosition()));
   Double_t bg_region = fProjection->Integral(fProjection->FindBin(fBGSlider->GetMinPosition()),fProjection->FindBin(fBGSlider->GetMaxPosition()));
   Double_t ratio = under_peak_bg/bg_region;
   
   Int_t first_bin = -1, last_bin = -1;
   if(fSubtractedHist){
      //Get old axis range
      first_bin = fSubtractedHist->GetXaxis()->GetFirst();
      last_bin = fSubtractedHist->GetXaxis()->GetLast();
      delete fSubtractedHist;

   }
   const char* sub_name = Form("%s_%s",fGateHist->GetName(),fBGHist->GetName());
   fSubtractedHist = static_cast<TH1*>(fGateHist->Clone(sub_name));
   fSubtractedHist->Add(fBGHist,-ratio);
   fGateCanvas->GetCanvas()->cd();
   if(fSubtractedHist){
      fSubtractedHist->GetXaxis()->SetRange(first_bin,last_bin);
      fSubtractedHist->Draw();
   }
   fGateCanvas->GetCanvas()->Update();

}

void TBGSubtraction::DrawOnNewCanvas(){
   if(fSubtractedHist){
      TCanvas* g =  new TCanvas;
      g->cd();
      fSubtractedHist->DrawCopy();
      g->Update();
   }

}

void TBGSubtraction::WriteHistograms() {
   TFile f("test.root","Update");
   std::cout << "Writing histograms to " << f.GetName() << std::endl;
   if(fSubtractedHist) fSubtractedHist->Write();
   if(fBGHist) fBGHist->Write();
   if(fGateHist) fGateHist->Write();
}

