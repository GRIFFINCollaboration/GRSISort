#include "Globals.h"
#include "GCanvas.h"

#include "TClass.h"
#include "TPaveStats.h"
#include "TList.h"
#include "TText.h"
#include "TLatex.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraphErrors.h"
#include "Buttons.h"
#include "KeySymbols.h"
#include "TVirtualX.h"
#include "TROOT.h"
#include "TFrame.h"
#include "TF1.h"
#include "TGraph.h"
#include "TPolyMarker.h"
#include "TSpectrum.h"
//#include "TPython.h"
#include "TCutG.h"
#include "TGInputDialog.h"

#include "TApplication.h"
#include "TContextMenu.h"
#include "TGButton.h"

#include "GPopup.h"

#include "GRootCommands.h"
#include "GH2I.h"
#include "GH2D.h"
#include "GH1D.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "TMath.h"

#include "TGRSIint.h"
#include "TLevelScheme.h"

#ifndef kArrowKeyPress
#define kArrowKeyPress 25
#define kArrowKeyRelease 26
#endif

enum MyArrowPress { kMyArrowLeft  = 0x1012,
                    kMyArrowUp    = 0x1013,
                    kMyArrowRight = 0x1014,
                    kMyArrowDown  = 0x1015 };

GMarker::GMarker(int x, int y, TH1* hist)
   : fHist(hist)
{
   if(fHist->GetDimension() == 1) {
      double localX = gPad->AbsPixeltoX(x);

      fLineX = new TLine(localX, gPad->GetUymin(), localX, gPad->GetUymax());
      fLineY = nullptr;
      SetColor(kRed);
      Draw();
   } else if(fHist->GetDimension() == 2) {
      double localX = gPad->AbsPixeltoX(x);
      double localY = gPad->AbsPixeltoY(y);

      fLineX = new TLine(localX, gPad->GetUymin(), localX, gPad->GetUymax());
      fLineY = new TLine(gPad->GetUxmin(), localY, gPad->GetUxmax(), localY);

      SetColor(kRed);
      Draw();
   }
}

void GMarker::Copy(TObject& object) const
{
   TObject::Copy(object);
   (static_cast<GMarker&>(object)).fLineX = nullptr;
   (static_cast<GMarker&>(object)).fLineY = nullptr;
   (static_cast<GMarker&>(object)).fHist  = fHist;
}

double GCanvas::fLastX = 0;
double GCanvas::fLastY = 0;

GCanvas::GCanvas(Bool_t build) : TCanvas(build)
{
   GCanvasInit();
}

GCanvas::GCanvas(const char* name, const char* title, Int_t form) : TCanvas(name, title, form)
{
   GCanvasInit();
}

GCanvas::GCanvas(const char* name, const char* title, Int_t winw, Int_t winh) : TCanvas(name, title, winw, winh)
{
   GCanvasInit();
}

GCanvas::GCanvas(const char* name, Int_t winw, Int_t winh, Int_t winid) : TCanvas(name, winw, winh, winid)
{
   // this constructor is used to create an embedded canvas
   // I see no reason for us to support this here.  pcb.
   GCanvasInit();
   fGuiEnabled = true;   // NOLINT
}

GCanvas::GCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t winw, Int_t winh, bool gui)
   : TCanvas(name, title, wtopx, wtopy, winw, winh)
{
   GCanvasInit();
   fGuiEnabled = gui;   // NOLINT
}

GCanvas::~GCanvas()
{
   // TCanvas::~TCanvas();
   delete[] fCutName;
}

void GCanvas::GCanvasInit()
{
   // ok, to interact with the default TGWindow
   // stuff from the root gui we need our own GRootCanvas.
   // We make this using GROOTGuiFactory, which replaces the
   // TRootGuiFactory used in the creation of some of the
   // default gui's (canvas,browser,etc).
   // fStatsDisplayed = true;
   fMarkerMode     = true;
   fGuiEnabled     = false;
   fBackgroundMode = EBackgroundSubtraction::kNoBackground;
   fCutName        = new char[256];
   SetBit(kNotDeleted, false);   // root voodoo.
}

void GCanvas::AddMarker(int x, int y, TH1* hist)
{
   auto*        mark                  = new GMarker(x, y, hist);
   unsigned int max_number_of_markers = (hist->GetDimension() == 1) ? 4 : 2;

   fMarkers.push_back(mark);

   if(fMarkers.size() > max_number_of_markers) {
      delete fMarkers.at(0);
      fMarkers.erase(fMarkers.begin());
   }
}

void GCanvas::RemoveMarker(Option_t* opt)
{
   TString options(opt);

   if(options.Contains("all")) {
      for(auto* marker : fMarkers) {
         delete marker;
      }
      for(auto* marker : fBackgroundMarkers) {
         delete marker;
      }
      fMarkers.clear();
      fBackgroundMarkers.clear();
   } else {
      if(fMarkers.empty()) {
         return;
      }
      delete fMarkers.back();
      fMarkers.pop_back();
   }
}

void GCanvas::OrderMarkers()
{
   std::sort(fMarkers.begin(), fMarkers.end());
}

void GCanvas::RedrawMarkers()
{
   gPad->Update();
   for(auto* marker : fMarkers) {
      marker->Update(GetUxmin(), GetUxmax(), GetUymin(), GetUymax());
      marker->Draw();
   }

   for(auto* marker : fBackgroundMarkers) {
      marker->Update(GetUxmin(), GetUxmax(), GetUymin(), GetUymax());
      marker->Draw();
   }
}

bool GCanvas::SetBackgroundMarkers()
{
   if(GetNMarkers() < 2) {
      return false;
   }

   // Delete previous background, if any.
   for(auto* marker : fBackgroundMarkers) {
      delete marker;
   }
   fBackgroundMarkers.clear();

   // Push last two markers into the background.
   fBackgroundMarkers.push_back(fMarkers.back());
   fMarkers.pop_back();
   fBackgroundMarkers.push_back(fMarkers.back());
   fMarkers.pop_back();

   // Change background marker color.
   for(auto* marker : fBackgroundMarkers) {
      marker->SetColor(kBlue);
   }

   fBackgroundMode = EBackgroundSubtraction::kRegionBackground;

   return true;
}

bool GCanvas::CycleBackgroundSubtraction()
{
   if(fBackgroundMarkers.size() < 2) {
      return false;
   }

   Color_t color = 0;

   switch(fBackgroundMode) {
   case EBackgroundSubtraction::kNoBackground:
      fBackgroundMode = EBackgroundSubtraction::kRegionBackground;
      Prompt();
      color = kBlue;
      break;
   case EBackgroundSubtraction::kRegionBackground:
      fBackgroundMode = EBackgroundSubtraction::kTotalFraction;
      color           = kGreen;
      break;
   case EBackgroundSubtraction::kTotalFraction:
      fBackgroundMode = EBackgroundSubtraction::kMatchedLowerMarker;
      color           = kOrange;
      break;
   case EBackgroundSubtraction::kMatchedLowerMarker:
      fBackgroundMode = EBackgroundSubtraction::kSplitTwoMarker;
      color           = kMagenta;
      break;
   case EBackgroundSubtraction::kSplitTwoMarker:
      fBackgroundMode = EBackgroundSubtraction::kNoBackground;
      color           = 0;
      break;
   };

   for(auto* marker : fBackgroundMarkers) {
      marker->SetColor(color);
   }

   return true;
}

GCanvas* GCanvas::MakeDefCanvas()
{
   // Static function to build a default canvas.

   const char* defcanvas = gROOT->GetDefCanvasName();
   char*       cdef      = nullptr;
   auto*       lc        = static_cast<TList*>(gROOT->GetListOfCanvases());
   if(lc->FindObject(defcanvas) != nullptr) {
      Int_t n = lc->GetSize() + 1;
      cdef    = new char[strlen(defcanvas) + 15];
      do {
         strlcpy(cdef, Form("%s_n%d", defcanvas, n++), strlen(defcanvas) + 15);
      } while(lc->FindObject(cdef) != nullptr);
   } else {
      cdef = StrDup(Form("%s", defcanvas));
   }
   auto* c = new GCanvas(cdef, cdef, 1);
   delete[] cdef;
   return c;
}

void GCanvas::HandleInput(int event, Int_t x, Int_t y)
{
   // If the below switch breaks. You need to upgrade your version of ROOT
   // Version 5.34.24 works. //older version should work now too pcb (8/2015)
   bool used = false;
   switch(event) {
   case kButton1Down:   // single click
      used = StorePosition(event, x, y);
      if(used) { break; }
      // next comment prevents warning about falling through for gcc, with c++17 we can also use "[[fallthrough]];"
      // fall through
   case kButton1Double:   // double click
      used = HandleMousePress(event, x, y);
      break;
   case kButton1Shift:   // shift-click
      used = HandleMouseShiftPress(event, x, y);
      break;
   case 9:   // control-click
      used = HandleMouseControlPress(event, x, y);
      break;
   case kButton1Up:   // button released
      used = Zoom(event, x, y);
      break;
   case kWheelUp:
   case kWheelDown:
      used = HandleWheel(event, x, y);
      break;
   };
   if(!used) {
      TCanvas::HandleInput(static_cast<EEventType>(event), x, y);
   }
}

void GCanvas::Draw(Option_t* opt)
{
   std::cout << "GCanvas Draw was called." << std::endl;
   TCanvas::Draw(opt);
   if(FindObject("TFrame") != nullptr) {
      FindObject("TFrame")->SetBit(TBox::kCannotMove);
   }
}

std::vector<TH1*> GCanvas::FindHists(int dim)
{
   std::vector<TH1*> tempvec;
   TIter             iter(gPad->GetListOfPrimitives());
   while(TObject* obj = iter.Next()) {
      if(obj->InheritsFrom(TH1::Class())) {
         TH1* hist = static_cast<TH1*>(obj);
         if(hist->GetDimension() == dim) {
            tempvec.push_back(hist);
         }
      }
   }
   return tempvec;
}

std::vector<TH1*> GCanvas::FindAllHists()
{
   std::vector<TH1*> tempvec;
   TIter             iter(gPad->GetListOfPrimitives());
   while(TObject* obj = iter.Next()) {
      if(obj->InheritsFrom("TH1")) {
         tempvec.push_back(static_cast<TH1*>(obj));
      }
   }
   return tempvec;
}

bool GCanvas::HandleArrowKeyPress(Event_t* event, const UInt_t* keysym)
{

   bool edited = Process1DArrowKeyPress(event, keysym);
   if(!edited) {
      edited = Process2DArrowKeyPress(event, keysym);
   }

   if(edited) {
      gPad->Modified();
      gPad->Update();
   }
   return true;
}

bool GCanvas::HandleKeyboardPress(Event_t* event, const UInt_t* keysym)
{
   bool edited = false;

   edited = ProcessNonHistKeyboardPress(event, keysym);

   if(!edited) {
      edited = Process1DKeyboardPress(event, keysym);
   }
   if(!edited) {
      edited = Process2DKeyboardPress(event, keysym);
   }

   if(edited) {
      gPad->Modified();
      gPad->Update();
   }
   return true;
}

bool GCanvas::HandleMousePress(Int_t event, Int_t x, Int_t y)
{
   if(GetSelected() == nullptr) {
      return false;
   }

   TH1* hist = nullptr;
   if(GetSelected()->InheritsFrom(TH1::Class())) {
      hist = static_cast<TH1*>(GetSelected());
   } else if(GetSelected()->IsA() == TFrame::Class()) {
      std::vector<TH1*> hists = FindAllHists();
      if(!hists.empty()) {
         hist = hists.front();

         // Let everybody know that the histogram is selected
         SetSelected(hist);
         SetClickSelected(hist);
         Selected(GetSelectedPad(), hist, event);
      }
   }

   if((hist == nullptr) || hist->GetDimension() > 2) {
      return false;
   }

   bool used = false;

   if(fMarkerMode) {
      AddMarker(x, y, hist);
      used = true;
   }

   if(used) {
      gPad->Modified();
      gPad->Update();
   }

   return used;
}

bool GCanvas::HandleMouseShiftPress(Int_t, Int_t, Int_t)
{
   TH1*  hist = nullptr;
   TIter iter(gPad->GetListOfPrimitives());
   while(TObject* obj = iter.Next()) {
      if(obj->InheritsFrom(TH1::Class())) {
         hist = static_cast<TH1*>(obj);
      }
   }
   if(hist == nullptr) {
      return false;
   }

   TString options;
   switch(hist->GetDimension()) {
   case 1: {
      if(hist->InheritsFrom(GH1D::Class())) {
         new GCanvas();
         (static_cast<GH1D*>(hist))->GetParent()->Draw("colz");
         return true;
      }
      std::vector<TH1*> hists = FindHists(1);
      new GCanvas();
      // options.Append("HIST");
      hists.at(0)->DrawCopy(options.Data());
      for(unsigned int j = 1; j < hists.size(); j++) {
         hists.at(j)->DrawCopy("same");
      }
   }
      return true;
   case 2:
      options.Append("colz");
      auto* ghist = new GH2D(*(static_cast<TH2*>(hist)));
      new GCanvas();
      ghist->Draw();
      return true;
   };
   return false;
}

bool GCanvas::HandleMouseControlPress(Int_t, Int_t, Int_t)
{
   if(GetSelected() == nullptr) {
      return false;
   }
   if(GetSelected()->InheritsFrom(TCutG::Class())) {
      // TODO: Bring this back, once we have brought over more from GRUTinizer
      // if(TRuntimeObjects::Get())
      //   TRuntimeObjects::Get()->GetGates().Add(GetSelected());
   }
   return true;
}

bool GCanvas::StorePosition(Int_t, Int_t px, Int_t py)
{
   /// Store the position the mouse button was pressed at.
   if(std::strcmp(GetName(), "LevelScheme") != 0) { return false; }

   fLastX = PixeltoX(px);
   fLastY = PixeltoY(py - GetWh());   // see https:://root.cern.ch/root/htmldoc/guides/users-guide/Graphics.html 11.3.3 "Converting between Coordinate Systems"

   return true;
}

bool GCanvas::Zoom(Int_t, Int_t px, Int_t py)
{
   /// Mouse button was released at this point, set the new range.

   if(std::strcmp(GetName(), "LevelScheme") != 0) { return false; }

   double x = PixeltoX(px);
   double y = PixeltoY(py - GetWh());   // see https:://root.cern.ch/root/htmldoc/guides/users-guide/Graphics.html 11.3.3 "Converting between Coordinate Systems"
   // ensure x,y is the second point of the range
   if(fLastX > x) { std::swap(fLastX, x); }
   if(fLastY > y) { std::swap(fLastY, y); }
   Range(fLastX, fLastY, x, y);
   Modified();
   Update();

   return true;
}

bool GCanvas::HandleWheel(Int_t event, Int_t px, Int_t py)
{
   /// Zoom in (wheel up) and out (wheel down) of level scheme, focused around x, y.
   /// Does nothing if the canvas doesn't have the name "LevelScheme".

   if(std::strcmp(GetName(), "LevelScheme") != 0) { return false; }

   // convert from pixel coordinates to user coordinates
   double x = PixeltoX(px);
   double y = PixeltoY(py - GetWh());   // see https:://root.cern.ch/root/htmldoc/guides/users-guide/Graphics.html 11.3.3 "Converting between Coordinate Systems"

   // get the current range
   double x1 = 0.;
   double y1 = 0.;
   double x2 = 0.;
   double y2 = 0.;
   GetRange(x1, y1, x2, y2);

   // calculate the new range
   double width  = (x2 - x1);
   double height = (y2 - y1);

   if(event == kWheelUp) {
      width /= 1.1;
      height /= 1.1;
   } else if(event == kWheelDown) {
      width *= 1.1;
      height *= 1.1;
   } else {
      std::cout << "Don't know what to do, got event " << event << " which isn't kWheelUp (" << kWheelUp << ") or kWheelDown (" << kWheelDown << ")" << std::endl;
      return false;
   }

   x1 = x - width / 2.;
   y1 = y - height / 2.;
   x2 = x + width / 2.;
   y2 = y + height / 2.;

   Range(x1, y1, x2, y2);
   Modified();
   Update();

   return true;
}

TF1* GCanvas::GetLastFit()
{
   TH1*  hist = nullptr;
   TIter iter(gPad->GetListOfPrimitives());
   while(TObject* obj = iter.Next()) {
      if(obj->InheritsFrom("TH1") && !obj->InheritsFrom("TH2") && !obj->InheritsFrom("TH3")) {
         hist = static_cast<TH1*>(obj);
      }
   }
   if(hist == nullptr) {
      return nullptr;
   }
   if(hist->GetListOfFunctions()->GetSize() > 0) {
      TF1* tmpfit = static_cast<TF1*>(hist->GetListOfFunctions()->Last());
      return tmpfit;
   }
   return nullptr;
}

bool GCanvas::Process1DArrowKeyPress(Event_t*, const UInt_t* keysym)
{
   /// Moves displayed 1D histograms by 50% of the visible range left, right, or selects the next (up) or previous (down) GH1D histogram.
   bool              edited = false;
   std::vector<TH1*> hists  = FindHists(1);
   if(hists.empty()) {
      return edited;
   }

   // get first and last bin in current range
   int first = hists.at(0)->GetXaxis()->GetFirst();
   int last  = hists.at(0)->GetXaxis()->GetLast();

   // first is 1 if no range is defined but can be 0, last is fNbins if no range is defined but can be 0
   // so min will always be 0, and max will always be fNbins+1
   int min = std::min(first, 0);
   int max = std::max(last, hists.at(0)->GetXaxis()->GetNbins() + 1);

   int xdiff = last - first;
   int mdiff = max - min - 2;   // this will always be fNbins-1

   switch(*keysym) {
   case kMyArrowLeft: {
      if(mdiff > xdiff) {
         // try and move left by half the current range
         if(first == (min + 1)) {
            // if first is 1 we can't go any further left
         } else if((first - (xdiff / 2)) < min) {
            first = min + 1;
            last  = min + (xdiff) + 1;
         } else {
            first = first - (xdiff / 2);
            last  = last - (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(first, last);
      }

      edited = true;
   } break;
   case kMyArrowRight: {
      if(mdiff > xdiff) {
         // try and move right by half the current range
         if(last == (max - 1)) {
            // last is fNbins so we can't move further right
         } else if((last + (xdiff / 2)) > max) {
            first = max - 1 - (xdiff);
            last  = max - 1;
         } else {
            last  = last + (xdiff / 2);
            first = first + (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(first, last);
      }

      edited = true;
   } break;

   case kMyArrowUp: {
      GH1D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH1D::Class())) {
            ghist = static_cast<GH1D*>(hist);
            break;
         }
      }

      if(ghist != nullptr) {
         TH1* next = ghist->GetNext();
         if(next != nullptr) {
            next->GetXaxis()->SetRange(first, last);
            next->Draw("");
            RedrawMarkers();
            edited = true;
         }
      }
   } break;

   case kMyArrowDown: {
      GH1D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH1D::Class())) {
            ghist = static_cast<GH1D*>(hist);
            break;
         }
      }

      if(ghist != nullptr) {
         TH1* prev = ghist->GetPrevious();
         if(prev != nullptr) {
            prev->GetXaxis()->SetRange(first, last);
            prev->Draw("");
            RedrawMarkers();
            edited = true;
         }
      }
   } break;
   default: std::cout << "keysym = " << *keysym << std::endl; break;
   }
   return edited;
}

bool GCanvas::ProcessNonHistKeyboardPress(Event_t*, const UInt_t* keysym)
{
   bool edited = false;

   switch(*keysym) {
   case kKey_F2:
      GetCanvasImp()->ShowEditor(!GetCanvasImp()->HasEditor());
      edited = true;
      break;
   case kKey_F9:
      SetCrosshair(static_cast<Int_t>(!HasCrosshair()));
      edited = true;
      break;
#if __cplusplus >= 201703L
   case kKey_u:
      if(GetListOfPrimitives()->FindObject("TLevelScheme") != nullptr) {
         static_cast<TLevelScheme*>(GetListOfPrimitives()->FindObject("TLevelScheme"))->UnZoom();
         edited = true;
      }
      break;
#endif
   }

   return edited;
}

bool GCanvas::Process1DKeyboardPress(Event_t*, const UInt_t* keysym)
{
   bool              edited = false;
   std::vector<TH1*> hists  = FindHists(1);
   if(hists.empty()) {
      return edited;
   }

   switch(*keysym) {
   case kKey_b: edited = SetBackgroundMarkers(); break;

   case kKey_B: edited = CycleBackgroundSubtraction(); break;

   case kKey_d: {
      new GPopup(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), 500, 200);
   } break;

   case kKey_e:
      if(GetNMarkers() < 2) {
         break;
      }
      {
         if(fMarkers.at(fMarkers.size() - 1)->GetLocalX() < fMarkers.at(fMarkers.size() - 2)->GetLocalX()) {
            for(auto* hist : hists) {
               hist->GetXaxis()->SetRangeUser(fMarkers.at(fMarkers.size() - 1)->GetLocalX(),
                                              fMarkers.at(fMarkers.size() - 2)->GetLocalX());
            }
         } else {
            for(auto* hist : hists) {
               hist->GetXaxis()->SetRangeUser(fMarkers.at(fMarkers.size() - 2)->GetLocalX(),
                                              fMarkers.at(fMarkers.size() - 1)->GetLocalX());
            }
         }
      }
      edited = true;
      RemoveMarker("all");
      break;
   case kKey_E:
      // GetListOfPrimitives()->Print();
      GetContextMenu()->Action(hists.back()->GetXaxis(),
                               TAxis::Class()->GetMethodAny("SetRangeUser"));
      {
         double x1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetFirst());
         double x2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetLast());
         TIter  iter(GetListOfPrimitives());
         while(TObject* obj = iter.Next()) {
            if(obj->InheritsFrom(TPad::Class())) {
               TPad* pad = static_cast<TPad*>(obj);
               TIter iter2(pad->GetListOfPrimitives());
               while(TObject* obj2 = iter2.Next()) {
                  if(obj2->InheritsFrom(TH1::Class())) {
                     TH1* hist = static_cast<TH1*>(obj2);
                     hist->GetXaxis()->SetRangeUser(x1, x2);
                     pad->Modified();
                     pad->Update();
                  }
               }
            }
         }

         // for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
         //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());
      }
      edited = true;
      break;
   case kKey_f:
      if(!hists.empty() && GetNMarkers() > 1) {
         std::cout << "x low = " << fMarkers.at(fMarkers.size() - 2)->GetLocalX() << "\t\txhigh = " << fMarkers.back()->GetLocalX() << std::endl;
         if(PhotoPeakFit(hists.back(), fMarkers.at(fMarkers.size() - 2)->GetLocalX(), fMarkers.back()->GetLocalX()) != nullptr) {
            edited = true;
         }
      }
      break;

   case kKey_F:
      if(!hists.empty() && GetNMarkers() > 1) {
         std::cout << "x low = " << fMarkers.at(fMarkers.size() - 2)->GetLocalX() << "\t\txhigh = " << fMarkers.back()->GetLocalX() << std::endl;
         if(AltPhotoPeakFit(hists.back(), fMarkers.at(fMarkers.size() - 2)->GetLocalX(), fMarkers.back()->GetLocalX(), "+") !=
            nullptr) {
            edited = true;
         }
      }
      break;

   case kKey_g:
      if(GausFit(hists.back(), fMarkers.at(fMarkers.size() - 2)->GetLocalX(), fMarkers.back()->GetLocalX()) != nullptr) {
         edited = true;
      }
      break;

   case kKey_i:
      if(!hists.empty() && GetNMarkers() > 1) {
         int binlow  = fMarkers.at(fMarkers.size() - 1)->GetBinX();
         int binhigh = fMarkers.at(fMarkers.size() - 2)->GetBinX();
         if(binlow > binhigh) {
            std::swap(binlow, binhigh);
         }
         double xlow  = hists.back()->GetXaxis()->GetBinLowEdge(binlow);
         double xhigh = hists.back()->GetXaxis()->GetBinLowEdge(binhigh);

         {
            double epsilon = 16 * (std::nextafter(xlow, INFINITY) - xlow);
            xlow += epsilon;
         }

         {
            double epsilon = 16 * (xhigh - std::nextafter(xhigh, -INFINITY));
            xhigh -= epsilon;
         }

         double sum =
            hists.back()->Integral(hists.back()->GetXaxis()->FindBin(xlow), hists.back()->GetXaxis()->FindBin(xhigh));
         std::cout << BLUE << std::endl
                   << "\tSum [" << xlow << " : " << xhigh << "] = " << sum << RESET_COLOR << std::endl;
      }
      break;
   case kKey_I:
      if(!hists.empty()) {
      }
      break;
   case kKey_l:
   case kKey_y:
      if(GetLogy() != 0) {
         // Show full y range, not restricted to positive values.
         for(auto* hist : hists) {
            hist->GetYaxis()->UnZoom();
         }
         SetLogy(0);
      } else {
         // Only show plot from 0 up when in log scale.
         for(auto* hist : hists) {
            if(hist->GetYaxis()->GetXmin() < 0) {
               hist->GetYaxis()->SetRangeUser(0, hist->GetYaxis()->GetXmax());
            }
         }
         SetLogy(1);
      }
      // TODO: Make this work, instead of disappearing the markers in log mode.
      // RedrawMarkers();
      edited = true;
      break;

   case kKey_m: SetMarkerMode(true); break;
   case kKey_M:
      SetMarkerMode(false);
      [[fallthrough]];
   case kKey_n:
      RemoveMarker("all");
      for(auto* hist : hists) {
         hist->GetListOfFunctions()->Clear();
      }
      for(auto* hist : hists) {
         hist->Sumw2(false);
      }
      RemovePeaks(hists.data(), hists.size());
      edited = true;
      break;
   case kKey_N:
      RemoveMarker("all");
      for(auto* hist : hists) {
         hist->GetListOfFunctions()->Clear();
      }
      RemovePeaks(hists.data(), hists.size());
      Clear();
      hists.at(0)->Draw("hist");
      for(unsigned int i = 1; i < hists.size(); i++) {
         hists.at(i)->Draw("histsame");
      }
      edited = true;
      break;
   case kKey_o:
      for(auto* hist : hists) {
         hist->GetXaxis()->UnZoom();
         hist->GetYaxis()->UnZoom();
      }
      RemoveMarker("all");
      edited = true;
      break;

   case kKey_p: {
      if(GetNMarkers() < 2) {
         break;
      }
      GH1D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH1D::Class())) {
            ghist = static_cast<GH1D*>(hist);
            break;
         }
      }
      //  ok, i found a bug.  if someone tries to gate on a histogram
      //  that is already zoomed, bad things will happen; namely the bins
      //  in the zoomed histogram will not map correctly to the parent. To get
      //  around this we need the bin value, not the bin!   pcb.
      //
      if(ghist != nullptr) {
         GH1D* proj    = nullptr;
         int   binlow  = fMarkers.at(fMarkers.size() - 1)->GetBinX();
         int   binhigh = fMarkers.at(fMarkers.size() - 2)->GetBinX();
         if(binlow > binhigh) {
            std::swap(binlow, binhigh);
         }
         double value_low  = ghist->GetXaxis()->GetBinLowEdge(binlow);
         double value_high = ghist->GetXaxis()->GetBinLowEdge(binhigh);

         {
            double epsilon = 16 * (std::nextafter(value_low, INFINITY) - value_low);
            value_low += epsilon;
         }

         {
            double epsilon = 16 * (value_high - std::nextafter(value_high, -INFINITY));
            value_high -= epsilon;
         }

         if(fBackgroundMarkers.size() >= 2 && fBackgroundMode != EBackgroundSubtraction::kNoBackground) {
            int bg_binlow  = fBackgroundMarkers.at(0)->GetBinX();
            int bg_binhigh = fBackgroundMarkers.at(1)->GetBinX();
            if(bg_binlow > bg_binhigh) {
               std::swap(bg_binlow, bg_binhigh);
            }
            double bg_value_low  = ghist->GetXaxis()->GetBinCenter(bg_binlow);
            double bg_value_high = ghist->GetXaxis()->GetBinCenter(bg_binhigh);
            // Using binhigh-1 instead of binhigh,
            //  because the ProjectionX/Y functions from ROOT use inclusive bin numbers,
            //  rather than exclusive.
            //
            proj = ghist->Project_Background(value_low, value_high, bg_value_low, bg_value_high, fBackgroundMode);
         } else {
            proj = ghist->Project(value_low, value_high);
         }
         if(proj != nullptr) {
            proj->Draw("");
            edited = true;
         }
      }
   } break;

   case kKey_P: {
      GH1D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH1D::Class())) {
            ghist = static_cast<GH1D*>(hist);
            break;
         }
      }

      if(ghist != nullptr) {
         ghist->GetParent()->Draw();
         edited = true;
      }
   } break;
   case kKey_q: {
      TH1* ghist = hists.at(0);
      if(GetNMarkers() > 1) {
         edited = (PhotoPeakFit(ghist, fMarkers.at(fMarkers.size() - 2)->GetLocalX(), fMarkers.back()->GetLocalX()) != nullptr);
      }
      if(edited) {
         ghist->Draw("hist");

         TIter iter(ghist->GetListOfFunctions());
         while(TObject* o = iter.Next()) {
            if(o->InheritsFrom(TF1::Class())) {
               (static_cast<TF1*>(o))->Draw("same");
            }
         }
      }
   }

   break;

   case kKey_r:
      if(GetNMarkers() < 2) {
         break;
      }
      {
         if(fMarkers.at(fMarkers.size() - 1)->GetLocalY() < fMarkers.at(fMarkers.size() - 2)->GetLocalY()) {
            for(auto* hist : hists) {
               hist->GetYaxis()->SetRangeUser(fMarkers.at(fMarkers.size() - 1)->GetLocalY(),
                                              fMarkers.at(fMarkers.size() - 2)->GetLocalY());
            }
         } else {
            for(auto* hist : hists) {
               hist->GetYaxis()->SetRangeUser(fMarkers.at(fMarkers.size() - 2)->GetLocalY(),
                                              fMarkers.at(fMarkers.size() - 1)->GetLocalY());
            }
         }
      }
      edited = true;
      RemoveMarker("all");
      break;
   case kKey_R:
      // GetListOfPrimitives()->Print();
      GetContextMenu()->Action(hists.back()->GetYaxis(),
                               TAxis::Class()->GetMethodAny("SetRangeUser"));
      {
         double y1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetFirst());
         double y2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetLast());
         TIter  iter(GetListOfPrimitives());
         while(TObject* obj = iter.Next()) {
            if(obj->InheritsFrom(TPad::Class())) {
               TPad* pad = static_cast<TPad*>(obj);
               TIter iter2(pad->GetListOfPrimitives());
               while(TObject* obj2 = iter2.Next()) {
                  if(obj2->InheritsFrom(TH1::Class())) {
                     TH1* hist = static_cast<TH1*>(obj2);
                     hist->GetYaxis()->SetRangeUser(y1, y2);
                     pad->Modified();
                     pad->Update();
                  }
               }
            }
         }

         // for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
         //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());
      }
      edited = true;
      break;

   case kKey_s:

      if(GetNMarkers() < 2) {
         edited = ShowPeaks(hists.data(), hists.size());
         RemoveMarker("all");
      } else {
         double x1 = fMarkers.at(fMarkers.size() - 1)->GetLocalX();
         double x2 = fMarkers.at(fMarkers.size() - 2)->GetLocalX();
         if(x1 > x2) {
            std::swap(x1, x2);
         }
         double y1 = fMarkers.at(fMarkers.size() - 1)->GetLocalY();
         double y2 = fMarkers.at(fMarkers.size() - 2)->GetLocalY();
         if(y1 > y2) {
            std::swap(y1, y2);
         }

         double ymax   = hists.at(0)->GetMaximum();
         double thresh = y1 / ymax;
         double sigma  = x2 - x1;
         if(sigma > 10.0) {
            sigma = 10.0;
         }
         edited = ShowPeaks(hists.data(), hists.size(), sigma, thresh);
         RemoveMarker("all");
      }
      break;
   case kKey_S:

      if(GetNMarkers() < 2) {
         edited = ShowPeaks(hists.data(), hists.size());
         RemoveMarker("all");
      } else {
         double x1 = fMarkers.at(fMarkers.size() - 1)->GetLocalX();
         double x2 = fMarkers.at(fMarkers.size() - 2)->GetLocalX();
         if(x1 > x2) {
            std::swap(x1, x2);
         }
         double y1 = fMarkers.at(fMarkers.size() - 1)->GetLocalY();
         double y2 = fMarkers.at(fMarkers.size() - 2)->GetLocalY();
         if(y1 > y2) {
            std::swap(y1, y2);
         }

         double ymax   = hists.at(0)->GetMaximum();
         double thresh = y1 / ymax;
         double sigma  = 1.0;
         if(sigma > 10.0) {
            sigma = 10.0;
         }
         edited = ShowPeaks(hists.data(), hists.size(), sigma, thresh);
         RemoveMarker("all");
      }
      break;
   case kKey_F9: {
      int color = hists.at(0)->GetLineColor() + 1;
      if(color > 9) {
         color = 1;
      }
      hists.at(0)->SetLineColor(static_cast<Color_t>(color));
      edited = true;
   } break;

   case kKey_F10: {
   } break;
   };
   return edited;
}

bool GCanvas::Process1DMousePress(Int_t, Int_t, Int_t)
{
   bool edited = false;
   return edited;
}

bool GCanvas::Process2DArrowKeyPress(Event_t*, const UInt_t* keysym)
{
   /// Moves displayed 2D histograms by 50% of the visible range left, right, up, or down

   bool              edited = false;
   std::vector<TH1*> hists  = FindHists(2);
   if(hists.empty()) {
      return edited;
   }

   int firstX = hists.at(0)->GetXaxis()->GetFirst();
   int lastX  = hists.at(0)->GetXaxis()->GetLast();
   int firstY = hists.at(0)->GetYaxis()->GetFirst();
   int lastY  = hists.at(0)->GetYaxis()->GetLast();

   int minX = std::min(firstX, 0);
   int maxX = std::max(lastX, hists.at(0)->GetXaxis()->GetNbins() + 1);
   int minY = std::min(firstY, 0);
   int maxY = std::max(lastY, hists.at(0)->GetYaxis()->GetNbins() + 1);

   int xdiff  = lastX - firstX;
   int mxdiff = maxX - minX - 2;
   int ydiff  = lastY - firstY;
   int mydiff = maxY - minY - 2;

   switch(*keysym) {
   case kMyArrowLeft: {
      if(mxdiff > xdiff) {
         if(firstX == (minX + 1)) {
            //
         } else if((firstX - (xdiff / 2)) < minX) {
            firstX = minX + 1;
            lastX  = minX + (xdiff) + 1;
         } else {
            firstX = firstX - (xdiff / 2);
            lastX  = lastX - (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(firstX, lastX);
      }

      edited = true;
   } break;
   case kMyArrowRight: {
      if(mxdiff > xdiff) {
         if(lastX == (maxX - 1)) {
            //
         } else if((lastX + (xdiff / 2)) > maxX) {
            firstX = maxX - 1 - (xdiff);
            lastX  = maxX - 1;
         } else {
            lastX  = lastX + (xdiff / 2);
            firstX = firstX + (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(firstX, lastX);
      }

      edited = true;
   } break;

   case kMyArrowUp: {
      if(mydiff > ydiff) {
         if(lastY == (maxY - 1)) {
            //
         } else if((lastY + (ydiff / 2)) > maxY) {
            firstY = maxY - 1 - ydiff;
            lastY  = maxY - 1;
         } else {
            firstY = firstY + (ydiff / 2);
            lastY  = lastY + (ydiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetYaxis()->SetRange(firstY, lastY);
      }

      edited = true;
   } break;

   case kMyArrowDown: {
      if(mydiff > ydiff) {
         if(firstY == (minY + 1)) {
            //
         } else if((firstY - (ydiff / 2)) < minY) {
            firstY = minY + 1;
            lastY  = minY + (ydiff) + 1;
         } else {
            firstY = firstY - (ydiff / 2);
            lastY  = lastY - (ydiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetYaxis()->SetRange(firstY, lastY);
      }

      edited = true;
   } break;
   default: std::cout << "keysym = " << *keysym << std::endl; break;
   }
   return edited;
}

bool GCanvas::Process2DKeyboardPress(Event_t*, const UInt_t* keysym)
{
   bool              edited = false;
   std::vector<TH1*> hists  = FindHists(2);
   if(hists.empty()) {
      return edited;
   }
   switch(*keysym) {
   case kKey_c: {
      TString defaultName = "cut";
      if(gROOT->FindObject("CUTG") == nullptr) {
         std::cout << "Something went wrong, can't find 'CUTG', did you initialize the cut beforehand? Or maybe you already pressed c?" << std::endl
                   << "Current list of cuts is ";
         if(fCuts.empty()) { std::cout << "empty"; }
         for(auto* cut : fCuts) {
            std::cout << cut->GetName() << " ";
         }
         std::cout << std::endl;
         break;
      }
      fCuts.push_back(static_cast<TCutG*>(gROOT->FindObject("CUTG")));
      fCuts.back()->SetName(fCutName);
      std::cout << "Added cut to list of cuts: ";
      for(auto* cut : fCuts) {
         std::cout << cut->GetName() << " ";
      }
      std::cout << std::endl;
   } break;

   case kKey_e:
      if(GetNMarkers() < 2) {
         break;
      }
      {
         double x1 = fMarkers.at(fMarkers.size() - 1)->GetLocalX();
         double y1 = fMarkers.at(fMarkers.size() - 1)->GetLocalY();
         double x2 = fMarkers.at(fMarkers.size() - 2)->GetLocalX();
         double y2 = fMarkers.at(fMarkers.size() - 2)->GetLocalY();
         if(x1 > x2) {
            std::swap(x1, x2);
         }
         if(y1 > y2) {
            std::swap(y1, y2);
         }
         for(auto* hist : hists) {
            hist->GetXaxis()->SetRangeUser(x1, x2);
            hist->GetYaxis()->SetRangeUser(y1, y2);
         }
      }
      edited = true;
      RemoveMarker("all");
      break;

   case kKey_E:
      // GetListOfPrimitives()->Print();
      GetContextMenu()->Action(hists.back()->GetXaxis(),
                               TAxis::Class()->GetMethodAny("SetRangeUser"));
      {
         double x1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetFirst());
         double x2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetLast());
         TIter  iter(GetListOfPrimitives());
         while(TObject* obj = iter.Next()) {
            if(obj->InheritsFrom(TPad::Class())) {
               TPad* pad = static_cast<TPad*>(obj);
               TIter iter2(pad->GetListOfPrimitives());
               while(TObject* obj2 = iter2.Next()) {
                  if(obj2->InheritsFrom(TH1::Class())) {
                     TH1* hist = static_cast<TH1*>(obj2);
                     hist->GetXaxis()->SetRangeUser(x1, x2);
                     pad->Modified();
                     pad->Update();
                  }
               }
            }
         }

         // for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
         //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());
      }
      edited = true;
      break;

   case kKey_g:
      if(GetNMarkers() < 2) {
         break;
      }
      {
         static int cutcounter = 0;
         auto*      cut        = new TCutG(Form("_cut%i", cutcounter++), 9);
         // cut->SetVarX("");
         // cut->SetVarY("");
         //
         double x1 = fMarkers.at(fMarkers.size() - 1)->GetLocalX();
         double y1 = fMarkers.at(fMarkers.size() - 1)->GetLocalY();
         double x2 = fMarkers.at(fMarkers.size() - 2)->GetLocalX();
         double y2 = fMarkers.at(fMarkers.size() - 2)->GetLocalY();
         if(x1 > x2) {
            std::swap(x1, x2);
         }
         if(y1 > y2) {
            std::swap(y1, y2);
         }
         double xdist = (x2 - x1) / 2.0;
         double ydist = (y2 - y1) / 2.0;
         //
         //
         cut->SetPoint(0, x1, y1);
         cut->SetPoint(1, x1, y1 + ydist);
         cut->SetPoint(2, x1, y2);
         cut->SetPoint(3, x1 + xdist, y2);
         cut->SetPoint(4, x2, y2);
         cut->SetPoint(5, x2, y2 - ydist);
         cut->SetPoint(6, x2, y1);
         cut->SetPoint(7, x2 - xdist, y1);
         cut->SetPoint(8, x1, y1);
         cut->SetLineColor(kBlack);
         hists.at(0)->GetListOfFunctions()->Add(cut);

         // TODO: Bring this back once we have pulled in parts of TGRUTint
         // TGRSIint::instance()->LoadTCutG(cut);
      }
      edited = true;
      RemoveMarker("all");
      break;

   case kKey_i: {
      TString defaultName = "cut";
      new TGInputDialog(nullptr, static_cast<TRootCanvas*>(GetCanvasImp()), "Enter name of cut", defaultName, fCutName);
      if(strlen(fCutName) == 0) {
         break;
      }
      gROOT->SetEditorMode("CutG");
   } break;

   case kKey_n:
      RemoveMarker("all");
      // for(unsigned int i=0;i<hists.size();i++)
      //  hists.at(i)->GetListOfFunctions()->Delete();
      RemovePeaks(hists.data(), hists.size());
      for(auto* hist : hists) {
         hist->Sumw2(false);
      }
      edited = true;
      break;
   case kKey_o:
      for(auto* hist : hists) {
         TH2* h = static_cast<TH2*>(hist);
         h->GetXaxis()->UnZoom();
         h->GetYaxis()->UnZoom();
      }
      RemoveMarker("all");
      edited = true;
      break;
   case kKey_P: {
      GH2D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH2Base::Class())) {
            ghist = static_cast<GH2D*>(hist);
            break;
         }
      }

      if((ghist != nullptr) && (ghist->GetProjections()->GetSize() != 0)) {
         ghist->GetProjections()->At(0)->Draw("");
         edited = true;
      }
   } break;
   case kKey_r:
      if(GetNMarkers() < 2) {
         break;
      }
      {
         if(fMarkers.at(fMarkers.size() - 1)->GetLocalY() < fMarkers.at(fMarkers.size() - 2)->GetLocalY()) {
            for(auto* hist : hists) {
               hist->GetYaxis()->SetRangeUser(fMarkers.at(fMarkers.size() - 1)->GetLocalY(),
                                              fMarkers.at(fMarkers.size() - 2)->GetLocalY());
            }
         } else {
            for(auto* hist : hists) {
               hist->GetYaxis()->SetRangeUser(fMarkers.at(fMarkers.size() - 2)->GetLocalY(),
                                              fMarkers.at(fMarkers.size() - 1)->GetLocalY());
            }
         }
      }
      edited = true;
      RemoveMarker("all");
      break;
   case kKey_R:
      // GetListOfPrimitives()->Print();
      GetContextMenu()->Action(hists.back()->GetYaxis(),
                               TAxis::Class()->GetMethodAny("SetRangeUser"));
      {
         double y1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetFirst());
         double y2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetLast());
         TIter  iter(GetListOfPrimitives());
         while(TObject* obj = iter.Next()) {
            if(obj->InheritsFrom(TPad::Class())) {
               TPad* pad = static_cast<TPad*>(obj);
               TIter iter2(pad->GetListOfPrimitives());
               while(TObject* obj2 = iter2.Next()) {
                  if(obj2->InheritsFrom(TH1::Class())) {
                     TH1* hist = static_cast<TH1*>(obj2);
                     hist->GetYaxis()->SetRangeUser(y1, y2);
                     pad->Modified();
                     pad->Update();
                  }
               }
            }
         }

         // for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
         //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());
      }
      edited = true;
      break;

   case kKey_s: {
      TDirectory* oldDir      = gDirectory;
      TString     defaultName = "CutFile.cuts";
      char*       fileName    = new char[256];
      new TGInputDialog(nullptr, static_cast<TRootCanvas*>(GetCanvasImp()), "Enter file name to save cuts to", defaultName, fileName);
      if(strlen(fileName) == 0) {
         break;
      }
      TFile f(fileName, "update");
      if(!f.IsOpen()) {
         std::cout << RESET_COLOR << "Failed to open file '" << fileName << "', not saving cuts!" << std::endl;
         break;
      }
      std::cout << RESET_COLOR << "Writing the following cuts to '" << fileName << "':" << std::endl;
      for(auto* cut : fCuts) {
         std::cout << cut->GetName() << std::endl;
         cut->Write();
      }
      f.Close();
      delete[] fileName;
      oldDir->cd();
   } break;

   case kKey_x: {
      GH2D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH2Base::Class())) {
            ghist = static_cast<GH2D*>(hist);
            break;
         }
      }

      if(ghist != nullptr) {
         ghist->SetSummary(false);
         TH1* phist = ghist->ProjectionX();   //->Draw();
         if(phist != nullptr) {
            new GCanvas();
            phist->Draw("");
         }
         edited = true;
      }
   } break;

   case kKey_X: {
      GH2D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH2Base::Class())) {
            ghist = static_cast<GH2D*>(hist);
            break;
         }
      }

      if(ghist != nullptr) {
         ghist->SetSummary(true);
         ghist->SetSummaryDirection(EDirection::kYDirection);
         TH1* phist = ghist->GetNextSummary(nullptr, false);
         if(phist != nullptr) {
            new GCanvas();
            phist->Draw("");
         }
         edited = true;
      }
   } break;

   case kKey_y: {
      GH2D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH2Base::Class())) {
            ghist = static_cast<GH2D*>(hist);
            break;
         }
      }

      if(ghist != nullptr) {
         ghist->SetSummary(false);
         TH1* phist = ghist->ProjectionY();   //->Draw();
         if(phist != nullptr) {
            new GCanvas();
            phist->Draw("");
         }
         edited = true;
      }
   } break;

   case kKey_Y: {
      GH2D* ghist = nullptr;
      for(auto* hist : hists) {
         if(hist->InheritsFrom(GH2Base::Class())) {
            ghist = static_cast<GH2D*>(hist);
            break;
         }
      }

      if(ghist != nullptr) {
         ghist->SetSummary(true);
         ghist->SetSummaryDirection(EDirection::kXDirection);
         // TH1* phist = ghist->SummaryProject(1);
         TH1* phist = ghist->GetNextSummary(nullptr, false);
         if(phist != nullptr) {
            new GCanvas();
            phist->Draw("");
         }
         edited = true;
      }
   } break;

   case kKey_l:
   case kKey_z:
      if(GetLogz() != 0) {
         // Show full z range, not restricted to positive values.
         for(auto* hist : hists) {
            hist->GetZaxis()->UnZoom();
         }
         TVirtualPad* cpad = gPad;
         cd();
         gPad->SetLogz(0);
         cpad->cd();
      } else {
         // Only show plot from 0 up when in log scale.
         for(auto* hist : hists) {
            if(hist->GetZaxis()->GetXmin() < 0) {
               hist->GetZaxis()->SetRangeUser(0, hist->GetYaxis()->GetXmax());
            }
         }
         TVirtualPad* cpad = gPad;
         cd();
         gPad->SetLogz(1);
         cpad->cd();
      }
      edited = true;
      break;
   };
   return edited;
}

bool GCanvas::Process2DMousePress(Int_t, Int_t, Int_t)
{
   bool edited = false;
   return edited;
}
