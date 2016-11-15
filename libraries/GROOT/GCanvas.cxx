
#include "Globals.h"
#include "GCanvas.h"


#include <TClass.h>
#include <TPaveStats.h>
#include <TList.h>
#include <TText.h>
#include <TLatex.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraphErrors.h>
#include <Buttons.h>
#include <KeySymbols.h>
#include <TVirtualX.h>
#include <TROOT.h>
#include <TFrame.h>
#include <TF1.h>
#include <TGraph.h>
#include <TPolyMarker.h>
#include <TSpectrum.h>
#include <TPython.h>
#include <TCutG.h>

#include <TApplication.h>
#include <TContextMenu.h>
#include <TGButton.h>

//#include <TGFileDialog.h>
#include <GPopup.h>

//#include "GROOTGuiFactory.h"
#include "GRootCommands.h"
#include "GH2I.h"
#include "GH2D.h"
#include "GH1D.h"

//#include "TRuntimeObjects.h"

#include <iostream>
#include <fstream>
#include <string>

#include <TMath.h>

#include "TGRSIint.h"

#ifndef kArrowKeyPress
#define kArrowKeyPress 25
#define kArrowKeyRelease 26
#endif

enum MyArrowPress{
  kMyArrowLeft  = 0x1012,
  kMyArrowUp    = 0x1013,
  kMyArrowRight = 0x1014,
  kMyArrowDown  = 0x1015
};

ClassImp(GMarker)

  void GMarker::Copy(TObject &object) const {
    TObject::Copy(object);
    ((GMarker&)object).x      = x;
    ((GMarker&)object).y      = y;
    ((GMarker&)object).localx = localx;
    ((GMarker&)object).localy = localy;
    ((GMarker&)object).linex  = 0;
    ((GMarker&)object).liney  = 0;
    ((GMarker&)object).binx  = binx;
    ((GMarker&)object).biny  = biny;
  }
/*
   ClassImp(GPopup)

   GPopup::GPopup(const TGWindow *p,const TGWindow *m)
   : TGTransientFrame(p,m,200,200,kVerticalFrame) {
   SetCleanup(kDeepCleanup);
   Connect("CloseWindow()","GPopup",this,"CloseWindow()");
   DontCallClose();
   if(!p&&!m) {
   MakeZombie();
   return;
   }
   TGHorizontalFrame *fHtop = new TGHorizontalFrame(this,200,200);

   fButton1 = new TGTextButton(fHtop,"&ok",1);
   fButton1->SetCommand("printf(\"you pressed ok.\\n\")");
   fButton1 = new TGTextButton(fHtop,"&cancel",2);
   fButton1->SetCommand("printf(\"you pressed cancel.\\n\")");

   fHtop->AddFrame(fButton1,0);//new TGLayoutHints);//(kLHintsCenterX|kLHintsCenterY));
   fHtop->AddFrame(fButton2,0);//new TGLayoutHints);//(kLHintsCenterX|kLHintsCenterY));
   AddFrame(fHtop,0);//new TGLayoutHints);//(kLHintsExpandX|kLHintsExpandY));

   SetEditDisabled(kEditDisable);
   MapSubwindows();
   TGDimension size = GetDefaultSize();
   Resize(size);
   CenterOnParent();
   MapWindow();
   }

   GPopup::~GPopup() {
   if(IsZombie()) return;
//cleanup?
}

void GPopup::CloseWindow() {
DeleteWindow();
}
*/










int GCanvas::lastx = 0;
int GCanvas::lasty = 0;

GCanvas::GCanvas(Bool_t build)
  :TCanvas(build)  {
    GCanvasInit();
  }

GCanvas::GCanvas(const char* name, const char* title, Int_t form)
  :TCanvas(name,title,form) {
    GCanvasInit();

  }

GCanvas::GCanvas(const char* name, const char* title, Int_t ww, Int_t wh)
  :TCanvas(name,title,ww,wh) {
    GCanvasInit();

  }

GCanvas::GCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid)
  :TCanvas(name,ww,wh,winid) {
    // this constructor is used to create an embedded canvas
    // I see no reason for us to support this here.  pcb.
    GCanvasInit();
  }


GCanvas::GCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh,bool gui)
  :TCanvas(name,title,wtopx,wtopy,ww,wh) {
    GCanvasInit();
    fGuiEnabled = gui;
  }


GCanvas::~GCanvas() {
  //TCanvas::~TCanvas();
}

void GCanvas::GCanvasInit() {
  // ok, to interact with the default TGWindow
  // stuff from the root gui we need our own GRootCanvas.
  // We make this using GROOTGuiFactory, which replaces the
  // TRootGuiFactory used in the creation of some of the
  // default gui's (canvas,browser,etc).
  //fStatsDisplayed = true;
  fMarkerMode = true;
  control_key = false;
  fGuiEnabled = false;
  fBackgroundMode = kNoBackground;
  //if(gVirtualX->InheritsFrom("TGX11")) {
  //    printf("\tusing x11-like graphical interface.\n");
  //}
  //this->SetCrosshair(true);
  SetBit(kNotDeleted,false); //root voodoo.
}

void GCanvas::AddMarker(int x,int y,int dim) {
  std::vector<TH1*> hists = FindHists(dim);
  if(hists.size() < 1){
    return;
  }
  TH1* hist = hists[0];

  GMarker *mark = new GMarker();
  mark->x = x;
  mark->y = y;
  if(dim==1) {
    mark->localx = gPad->AbsPixeltoX(x);
    mark->localy = gPad->AbsPixeltoY(y);
    mark->binx = hist->GetXaxis()->FindBin(mark->localx);
    mark->biny = hist->GetYaxis()->FindBin(mark->localy);


    double bin_edge = hist->GetXaxis()->GetBinLowEdge(mark->binx);
    mark->linex = new TLine(bin_edge,hist->GetMinimum(),
        bin_edge,hist->GetMaximum());
    mark->SetColor(kRed);
    mark->Draw();
  } else if (dim==2) {
    mark->localx = gPad->AbsPixeltoX(x);
    mark->localy = gPad->AbsPixeltoY(y);
    mark->binx = hist->GetXaxis()->FindBin(mark->localx);
    mark->biny = hist->GetYaxis()->FindBin(mark->localy);
    double binx_edge = hist->GetXaxis()->GetBinLowEdge(mark->binx);
    double biny_edge = hist->GetYaxis()->GetBinLowEdge(mark->biny);

    mark->linex = new TLine(binx_edge,hist->GetYaxis()->GetXmin(),
        binx_edge,hist->GetYaxis()->GetXmax());
    mark->liney = new TLine(hist->GetXaxis()->GetXmin(),biny_edge,
        hist->GetXaxis()->GetXmax(),biny_edge);

    mark->SetColor(kRed);
    mark->Draw();
  }

  unsigned int max_number_of_markers = (dim==1) ? 4 : 2;

  fMarkers.push_back(mark);

  if(fMarkers.size() > max_number_of_markers) {
    delete fMarkers.at(0);
    fMarkers.erase(fMarkers.begin());
  }
  return;
}

void GCanvas::RemoveMarker(Option_t* opt) {
  TString options(opt);

  if(options.Contains("all")){
    for(auto marker : fMarkers){
      delete marker;
    }
    for(auto marker : fBackgroundMarkers){
      delete marker;
    }
    fMarkers.clear();
    fBackgroundMarkers.clear();
  } else {
    if(fMarkers.size()<1)
      return;
    if(fMarkers.at(fMarkers.size()-1))
      delete fMarkers.at(fMarkers.size()-1);
    //printf("Marker %i Removed\n");
    fMarkers.erase(fMarkers.end()-1);
  }
}

void GCanvas::OrderMarkers() {
  std::sort(fMarkers.begin(),fMarkers.end());
  return;
}

void GCanvas::RedrawMarkers() {
  gPad->Update();
  for(auto marker : fMarkers){
    if(marker->linex){
      marker->linex->SetY1(GetUymin());
      marker->linex->SetY2(GetUymax());
    }
    if(marker->liney){
      marker->liney->SetX1(GetUxmin());
      marker->liney->SetX2(GetUxmax());
    }
    marker->Draw();
  }

  for(auto marker : fBackgroundMarkers){
    if(marker->linex){
      marker->linex->SetY1(GetUymin());
      marker->linex->SetY2(GetUymax());
    }
    if(marker->liney){
      marker->liney->SetX1(GetUxmin());
      marker->liney->SetX2(GetUxmax());
    }
    marker->Draw();
  }
}

bool GCanvas::SetBackgroundMarkers() {
  if(GetNMarkers() < 2){
    return false;
  }

  // Delete previous background, if any.
  for(auto marker : fBackgroundMarkers){
    delete marker;
  }
  fBackgroundMarkers.clear();

  // Push last two markers into the background.
  fBackgroundMarkers.push_back(fMarkers.back());
  fMarkers.pop_back();
  fBackgroundMarkers.push_back(fMarkers.back());
  fMarkers.pop_back();

  // Change background marker color.
  for(auto marker : fBackgroundMarkers){
    marker->SetColor(kBlue);
  }

  fBackgroundMode = kRegionBackground;

  return true;
}

bool GCanvas::CycleBackgroundSubtraction() {
  if(fBackgroundMarkers.size() < 2){
    return false;
  }

  Color_t color = 0;

  switch(fBackgroundMode){
    case kNoBackground:
      fBackgroundMode = kRegionBackground;
      printf("hello??\n");
      Prompt();
      color = kBlue;
      break;
    case kRegionBackground:
      fBackgroundMode = kTotalFraction;
      color = kGreen;
      break;
    case kTotalFraction:
      fBackgroundMode = kMatchedLowerMarker;
      color = kOrange;
      break;
    case kMatchedLowerMarker:
      fBackgroundMode = kSplitTwoMarker;
      color = kMagenta;
      break;
    case kSplitTwoMarker:
      fBackgroundMode = kNoBackground;
      color = 0;
      break;
  };

  for(auto marker : fBackgroundMarkers){
    marker->SetColor(color);
  }

  return true;
}

//void GCanvas::AddBGMarker(GMarker *mark) {
//  GMarker *bg_mark = new GMarker(*mark);
//  fBG_Markers.push_back(bg_mark);
//}

GCanvas *GCanvas::MakeDefCanvas() {
  // Static function to build a default canvas.

  const char *defcanvas = gROOT->GetDefCanvasName();
  char *cdef;
  TList *lc = (TList*)gROOT->GetListOfCanvases();
  if (lc->FindObject(defcanvas)) {
    Int_t n = lc->GetSize() + 1;
    cdef = new char[strlen(defcanvas)+15];
    do {
      strlcpy(cdef,Form("%s_n%d", defcanvas, n++),strlen(defcanvas)+15);
    } while (lc->FindObject(cdef));
  } else
    cdef = StrDup(Form("%s",defcanvas));
  GCanvas *c = new GCanvas(cdef, cdef, 1);
  //printf("GCanvas::MakeDefCanvas"," created default GCanvas with name %s",cdef);
  delete [] cdef;
  return c;
}

//void GCanvas::ProcessEvent(Int_t event,Int_t x,Int_t y,TObject *obj) {
//   printf("{GCanvas} ProcessEvent:\n");
//   printf("\tevent: \t0x%08x\n",event);
//   printf("\tobject:\t0x%08x\n",obj);
//   printf("\tx:     \t0x%i\n",x);
//   printf("\ty:     \t0x%i\n",y);
//}

//void GCanvas::ExecuteEvent(Int_t event,Int_t x,Int_t y) {
//  printf("exc event called.\n");
//}


void GCanvas::HandleInput(int event,Int_t x,Int_t y) {
  //If the below switch breaks. You need to upgrade your version of ROOT
  //Version 5.34.24 works. //older version should work now too pcb (8/2015)
  bool used = false;
  //printf("event = 0x%08x\t x = 0x%08x\t y = 0x%08x \n",event,x,y);
  switch(event) {
    case kButton1Down: //single click
    case kButton1Double: //double click
      used = HandleMousePress(event,x,y);
      break;
    case kButton1Shift: //shift-click
      used = HandleMouseShiftPress(event,x,y);
      break;
    case 9:  //control-click
      used = HandleMouseControlPress(event,x,y);
      break;

  };
  if(!used)
    TCanvas::HandleInput(static_cast<EEventType>(event),x,y);
  return;
}


void GCanvas::Draw(Option_t *opt) {
  printf("GCanvas Draw was called.\n");
  TCanvas::Draw(opt);
  if(this->FindObject("TFrame"))
    this->FindObject("TFrame")->SetBit(TBox::kCannotMove);
}


std::vector<TH1*> GCanvas::FindHists(int dim) {
  std::vector<TH1*> tempvec;
  TIter iter(gPad->GetListOfPrimitives());
  while(TObject *obj = iter.Next()) {
    if( obj->InheritsFrom(TH1::Class()) ) {
      TH1* hist = (TH1*)obj;
      if(hist->GetDimension()==dim){
        tempvec.push_back(hist);
      }
    }
  }
  return tempvec;
}

std::vector<TH1*> GCanvas::FindAllHists() {
  std::vector<TH1*> tempvec;
  TIter iter(gPad->GetListOfPrimitives());
  while(TObject *obj = iter.Next()) {
    if( obj->InheritsFrom("TH1"))
      tempvec.push_back((TH1*)obj);
  }
  return tempvec;
}

bool GCanvas::HandleArrowKeyPress(Event_t *event,UInt_t *keysym) {

  bool edited = false;

  std::vector<TH1*> hists = FindHists();
  if(hists.size()>0){
    edited = Process1DArrowKeyPress(event,keysym);
  }

  if(edited){
    gPad->Modified();
    gPad->Update();
  }
  return true;
}


bool GCanvas::HandleKeyboardPress(Event_t *event,UInt_t *keysym) {
  bool edited = false;

  edited = ProcessNonHistKeyboardPress(event, keysym);

  std::vector<TH1*> hists = FindHists(1);
  if(hists.size()>0 && !edited){
    edited = Process1DKeyboardPress(event,keysym);
  }
  hists = FindHists(2);
  if(hists.size()>0 && !edited){
    edited = Process2DKeyboardPress(event,keysym);
  }

  if(edited) {
    gPad->Modified();
    gPad->Update();
  }
  return true;
}

bool GCanvas::HandleMousePress(Int_t event,Int_t x,Int_t y) {
  if(!GetSelected()) {
    return false;
  }


  TH1* hist = 0;
  if(GetSelected()->InheritsFrom(TH1::Class())) {
    hist = (TH1*)GetSelected();
  } else if(GetSelected()->IsA() == TFrame::Class()) {
    std::vector<TH1*> hists = FindAllHists();
    if(hists.size()) {
      hist = hists.front();

      // Let everybody know that the histogram is selected
      SetSelected(hist);
      SetClickSelected(hist);
      Selected(GetSelectedPad(), hist, event);
    }
  }

  if(!hist || hist->GetDimension() > 2) {
    return false;
  }

  bool used = false;

  if(fMarkerMode) {
    AddMarker(x,y,hist->GetDimension());
    used = true;
  }

  if(used) {
    gPad->Modified();
    gPad->Update();
  }

  return used;
}

bool GCanvas::HandleMouseShiftPress(Int_t event,Int_t x,Int_t y) {
  TH1 *hist = 0;
  TIter iter(gPad->GetListOfPrimitives());
  while(TObject *obj = iter.Next()) {
    if(obj->InheritsFrom(TH1::Class()))
      hist = (TH1*)obj;
  }
  if(!hist)
    return false;

  TString options;
  switch(hist->GetDimension()) {
    case 1:
      {
        if(hist->InheritsFrom(GH1D::Class())) {
          new GCanvas();
          ((GH1D*)hist)->GetParent()->Draw("colz");
          return true;
        }
        std::vector<TH1*> hists = FindHists();
        new GCanvas();
        //options.Append("HIST");
        hists.at(0)->DrawCopy(options.Data());
        for(unsigned int j=1;j<hists.size();j++)
          hists.at(j)->DrawCopy("same");
      }
      return true;
    case 2:
      options.Append("colz");
      GH2D *ghist = new GH2D(*((TH2*)hist));
      new GCanvas();
      ghist->Draw();
      return true;
  };
  return false;
}

bool GCanvas::HandleMouseControlPress(Int_t event,Int_t x,Int_t y) {
  //printf("GetSelected() = 0x%08x\n",GetSelected());
  if(!GetSelected())
    return false;
  //printf("GetSelected()->GetName() = %s\n",GetSelected()->GetName());
  if(GetSelected()->InheritsFrom(TCutG::Class())) {
    // TODO: Bring this back, once we have brought over more from GRUTinizer
    // if(TRuntimeObjects::Get())
    //   TRuntimeObjects::Get()->GetGates().Add(GetSelected());
  }
  return true;
}



TF1 *GCanvas::GetLastFit() {
  TH1 *hist = 0;
  TIter iter(gPad->GetListOfPrimitives());
  while(TObject *obj = iter.Next()) {
    if( obj->InheritsFrom("TH1") &&
        !obj->InheritsFrom("TH2") &&
        !obj->InheritsFrom("TH3") ) {
      hist = (TH1*)obj;
    }
  }
  if(!hist)
    return 0;
  if(hist->GetListOfFunctions()->GetSize()>0){
    TF1* tmpfit = (TF1*)(hist->GetListOfFunctions()->Last());
    //std::string tmpname = tmpfit->GetName();
    //while(tmpname.find("background") != std::string::npos ){
    //    tmpfit = (TF1*)(hist->GetListOfFunctions()->Before(tmpfit));
    //    tmpname = tmpfit->GetName();
    //}
    return tmpfit;
  }
  return 0;
}


bool GCanvas::Process1DArrowKeyPress(Event_t *event,UInt_t *keysym) {
  bool edited = false;
  std::vector<TH1*> hists = FindHists();

  int first = hists.at(0)->GetXaxis()->GetFirst();
  int last = hists.at(0)->GetXaxis()->GetLast();
  //TAxis* axis = hists.at(0)->GetXaxis();
  //int first = axis->GetFirst();
  //int last = axis->GetLast();

  int min = std::min(first,0);
  int max = std::max(last,hists.at(0)->GetXaxis()->GetNbins()+1);
  //int max = std::max(last,axis->GetNbins()+1);

  int xdiff = last-first;
  int mdiff = max-min-2;

  switch (*keysym) {
    case kMyArrowLeft:
      {
        if(mdiff>xdiff) {
          if(first==(min+1)) {
            //
          }
          else if((first-(xdiff/2))<min) {
            first = min+1;
            last  = min + (xdiff) + 1;
          } else {
            first = first-(xdiff/2);
            last  = last -(xdiff/2);
          }
        }
        for(unsigned int i=0;i<hists.size();i++)
          hists.at(i)->GetXaxis()->SetRange(first,last);
        //double begin = axis->GetBinLowEdge(first);
        //double end = axis->GetBinUpEdge(last);
        //for(unsigned int i=0;i<hists.size();i++) {
        //  hists.at(i)->GetXaxis()->SetRangeUser(begin,end);
        //}

        edited = true;
      }
      break;
    case kMyArrowRight:
      {
        if(mdiff>xdiff) {
          if(last== (max-1)) {
            //
          }else if((last+(xdiff/2))>max) {
            first = max - 1 - (xdiff);
            last  = max - 1;
          } else {
            last  = last +(xdiff/2);
            first = first+(xdiff/2);
          }
        }
        for(unsigned int i=0;i<hists.size();i++)
          hists.at(i)->GetXaxis()->SetRange(first,last);
        //double begin = axis->GetBinLowEdge(first);
        //double end = axis->GetBinUpEdge(last);
        //for(unsigned int i=0;i<hists.size();i++) {
        //  hists.at(i)->GetXaxis()->SetRangeUser(begin,end);
        //}

        edited = true;
      }
      break;

    case kMyArrowUp: {
                       GH1D* ghist = NULL;
                       for(auto hist : hists){
                         if(hist->InheritsFrom(GH1D::Class())){
                           ghist = (GH1D*)hist;
                           break;
                         }
                       }

                       if(ghist) {
                         TH1* prev = ghist->GetNext();
                         if(prev) {
                           prev->GetXaxis()->SetRange(first,last);
                           //prev->GetXaxis()->SetRange(axis->GetBinLowEdge(first),
                           //                           axis->GetBinUpEdge(last));
                           prev->Draw("");
                           RedrawMarkers();
                           edited = true;
                         }
                       }
                     }
                     break;

    case kMyArrowDown: {
                         GH1D* ghist = NULL;
                         for(auto hist : hists){
                           if(hist->InheritsFrom(GH1D::Class())){
                             ghist = (GH1D*)hist;
                             break;
                           }
                         }

                         if(ghist) {
                           TH1* prev = ghist->GetPrevious();
                           if(prev) {
                             prev->GetXaxis()->SetRange(first,last);
                             //prev->GetXaxis()->SetRange(axis->GetBinLowEdge(first),
                             //                           axis->GetBinUpEdge(last));
                             prev->Draw("");
                             RedrawMarkers();
                             edited = true;
                           }
                         }
                       }
                       break;
    default:
                       printf("keysym = %i\n",*keysym);
                       break;
  }
  return edited;
}

bool GCanvas::ProcessNonHistKeyboardPress(Event_t* event, UInt_t* keysym) {
  bool edited = false;

  switch(*keysym) {
    case kKey_F2:
      GetCanvasImp()->ShowEditor(!GetCanvasImp()->HasEditor());
      edited = true;
      break;
    case kKey_F9:
      this->SetCrosshair(!this->HasCrosshair());
      edited = true;
  }

  return edited;
}

bool GCanvas::Process1DKeyboardPress(Event_t *event,UInt_t *keysym) {

  //printf("keysym:   0x%08x\n",*keysym);

  bool edited = false;
  std::vector<TH1*> hists = FindHists();
  if(hists.size()<1)
    return edited;

  switch(*keysym) {
    case kKey_Control:
      toggle_control();
      break;

    case kKey_b:
      edited = SetBackgroundMarkers();
      break;

    case kKey_B:
      edited = CycleBackgroundSubtraction();
      break;

    case kKey_d:
      {
        printf("i am here.\n");
        new GPopup(gClient->GetDefaultRoot(),gClient->GetDefaultRoot(),500,200);
        //new GPopup(0,0);
        //this);
        //TGFileInfo fi;
        //new TGFileDialog(gClient->GetDefaultRoot(),gClient->GetDefaultRoot(),
        //                 kFDOpen, &fi);


      }
      break;

    case kKey_e:
      if(GetNMarkers()<2)
        break;
      {
        if(fMarkers.at(fMarkers.size()-1)->localx < fMarkers.at(fMarkers.size()-2)->localx)
          for(unsigned int i=0;i<hists.size();i++)
            hists.at(i)->GetXaxis()->SetRangeUser(fMarkers.at(fMarkers.size()-1)->localx,fMarkers.at(fMarkers.size()-2)->localx);
        else
          for(unsigned int i=0;i<hists.size();i++)
            hists.at(i)->GetXaxis()->SetRangeUser(fMarkers.at(fMarkers.size()-2)->localx,fMarkers.at(fMarkers.size()-1)->localx);
      }
      edited = true;
      RemoveMarker("all");
      break;
    case kKey_E:
      //this->GetListOfPrimitives()->Print();
      GetContextMenu()->Action(hists.back()->GetXaxis(),hists.back()->GetXaxis()->Class()->GetMethodAny("SetRangeUser"));
      {
        double x1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetFirst());
        double x2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetLast());
        TIter iter(this->GetListOfPrimitives());
        while(TObject *obj = iter.Next()) {
          if(obj->InheritsFrom(TPad::Class())) {
            TPad *pad = (TPad*)obj;
            TIter iter2(pad->GetListOfPrimitives());
            while(TObject *obj2=iter2.Next()) {
              if(obj2->InheritsFrom(TH1::Class())) {
                TH1* hist = (TH1*)obj2;
                hist->GetXaxis()->SetRangeUser(x1,x2);
                pad->Modified();
                pad->Update();
              }
            }
          }
        }

        //for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
        //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());

      }
      edited = true;
      break;
    case kKey_f:
      if(!hists.empty() && GetNMarkers()>1) {
        //printf("x low = %.1f\t\txhigh = %.1f\n",fMarkers.at(fMarkers.size()-2)->localx,fMarkers.back()->localx);
        if(PhotoPeakFit(hists.back(),fMarkers.at(fMarkers.size()-2)->localx,fMarkers.back()->localx))
          edited = true;
      }
      break;

    case kKey_g:
      if(GausFit(hists.back(),fMarkers.at(fMarkers.size()-2)->localx,fMarkers.back()->localx))
        edited = true;
      break;

      //case kKey_G:
      //   edited = GausBGFit();
      //   break;

    case kKey_i:
      if(!hists.empty() && GetNMarkers()>1) {
        int binlow = fMarkers.at(fMarkers.size()-1)->binx;
        int binhigh = fMarkers.at(fMarkers.size()-2)->binx;
        if(binlow > binhigh){
          std::swap(binlow, binhigh);
        }
        double xlow  = hists.back()->GetXaxis()->GetBinLowEdge(binlow);
        double xhigh = hists.back()->GetXaxis()->GetBinLowEdge(binhigh);

        {
          double epsilon = 16*(std::nextafter(xlow, INFINITY) - xlow);
          xlow += epsilon;
        }

        {
          double epsilon = 16*(xhigh - std::nextafter(xhigh, -INFINITY));
          xhigh -= epsilon;
        }

        double sum = hists.back()->Integral(hists.back()->GetXaxis()->FindBin(xlow),
            hists.back()->GetXaxis()->FindBin(xhigh));
        printf( BLUE "\n\tSum [%.01f : %.01f] = %.01f" RESET_COLOR  "\n",xlow,xhigh,sum);
      }
      break;
    case kKey_I:
      if(!hists.empty()) {
        printf( BLUE );

        printf( RESET_COLOR );
      }
      break;
    case kKey_l:
      if(GetLogy()){
        // Show full y range, not restricted to positive values.
        for(unsigned int i=0;i<hists.size();i++) {
          hists.at(i)->GetYaxis()->UnZoom();
        }
        SetLogy(0);
      } else {
        // Only show plot from 0 up when in log scale.
        for(unsigned int i=0;i<hists.size();i++) {
          if(hists.at(i)->GetYaxis()->GetXmin()<0) {
            hists.at(i)->GetYaxis()->SetRangeUser(0,hists.at(i)->GetYaxis()->GetXmax());
          }
        }
        SetLogy(1);
      }
      // TODO: Make this work, instead of disappearing the markers in log mode.
      //RedrawMarkers();
      edited = true;
      break;

    case kKey_m:
      SetMarkerMode(true);
      break;
    case kKey_M:
      SetMarkerMode(false);
    case kKey_n:
      RemoveMarker("all");
      for(unsigned int i=0;i<hists.size();i++) {
        hists.at(i)->GetListOfFunctions()->Clear();
      }
      for(unsigned int ii=0;ii<hists.size();ii++)
        hists[ii]->Sumw2(false);
      RemovePeaks(hists.data(),hists.size());
      edited = true;
      break;
    case kKey_N:
      RemoveMarker("all");
      for(unsigned int i=0;i<hists.size();i++) {
        hists.at(i)->GetListOfFunctions()->Clear();
      }
      RemovePeaks(hists.data(),hists.size());
      this->Clear();
      hists.at(0)->Draw("hist");
      for(unsigned int i=1;i<hists.size();i++) {
        hists.at(i)->Draw("histsame");
      }
      edited=true;
      break;
    case kKey_o:
      for(unsigned int i=0;i<hists.size();i++) {
        hists.at(i)->GetXaxis()->UnZoom();
        hists.at(i)->GetYaxis()->UnZoom();
      }
      RemoveMarker("all");
      edited = true;
      break;

    case kKey_p: {
        if(GetNMarkers() < 2){
          break;
        }
        GH1D* ghist = NULL;
        for(auto hist : hists){
          if(hist->InheritsFrom(GH1D::Class())){
            ghist = (GH1D*)hist;
            break;
          }
        }
        //  ok, i found a bug.  if someone tries to gate on a histogram
        //  that is already zoomed, bad things will happen; namely the bins
        //  in the zoomed histogram will not map correctly to the parent. To get
        //  around this we need the bin value, not the bin!   pcb.
        //
        if(ghist){
          GH1D* proj = NULL;
          int binlow = fMarkers.at(fMarkers.size()-1)->binx;
          int binhigh = fMarkers.at(fMarkers.size()-2)->binx;
          if(binlow > binhigh){
            std::swap(binlow, binhigh);
          }
          double value_low  = ghist->GetXaxis()->GetBinLowEdge(binlow);
          double value_high = ghist->GetXaxis()->GetBinLowEdge(binhigh);

          {
            double epsilon = 16*(std::nextafter(value_low, INFINITY) - value_low);
            value_low += epsilon;
          }

          {
            double epsilon = 16*(value_high - std::nextafter(value_high, -INFINITY));
            value_high -= epsilon;
          }

          if(fBackgroundMarkers.size()>=2 &&
              fBackgroundMode!=kNoBackground){
            int bg_binlow = fBackgroundMarkers.at(0)->binx;
            int bg_binhigh = fBackgroundMarkers.at(1)->binx;
            if(bg_binlow > bg_binhigh){
              std::swap(bg_binlow, bg_binhigh);
            }
            double bg_value_low  = ghist->GetXaxis()->GetBinCenter(bg_binlow);
            double bg_value_high = ghist->GetXaxis()->GetBinCenter(bg_binhigh);
            // Using binhigh-1 instead of binhigh,
            //  because the ProjectionX/Y functions from ROOT use inclusive bin numbers,
            //  rather than exclusive.
            //
            proj = ghist->Project_Background(value_low, value_high,
                bg_value_low, bg_value_high,
                fBackgroundMode);
          } else {
            proj = ghist->Project(value_low, value_high);
          }
          if(proj){
            proj->Draw("");
            edited=true;
          }
        }
      }
      break;

    case kKey_P: {
                   GH1D* ghist = NULL;
                   for(auto hist : hists){
                     if(hist->InheritsFrom(GH1D::Class())){
                       ghist = (GH1D*)hist;
                       break;
                     }
                   }

                   if(ghist){
                     ghist->GetParent()->Draw();
                     edited=true;
                   }
                 }
                 break;
    case kKey_q:{
                  TH1* ghist = hists.at(0);
                  if(GetNMarkers()>1) {

                    edited = PhotoPeakFit(ghist,fMarkers.at(fMarkers.size()-2)->localx,fMarkers.back()->localx);
                  }
                  if(edited){
                    ghist->Draw("hist");

                    TIter iter(ghist->GetListOfFunctions());
                    while(TObject *o = iter.Next()){
                      if(o->InheritsFrom(TF1::Class())) {
                        ((TF1*)o)->Draw("same");
                      }
                    }
                  }
                }

                break;

    case kKey_r:
                if(GetNMarkers()<2)
                  break;
                {
                  if(fMarkers.at(fMarkers.size()-1)->localy < fMarkers.at(fMarkers.size()-2)->localy)
                    for(unsigned int i=0;i<hists.size();i++)
                      hists.at(i)->GetYaxis()->SetRangeUser(fMarkers.at(fMarkers.size()-1)->localy,fMarkers.at(fMarkers.size()-2)->localy);
                  else
                    for(unsigned int i=0;i<hists.size();i++)
                      hists.at(i)->GetXaxis()->SetRangeUser(fMarkers.at(fMarkers.size()-2)->localy,fMarkers.at(fMarkers.size()-1)->localy);
                }
                edited = true;
                RemoveMarker("all");
                break;
    case kKey_R:
                //this->GetListOfPrimitives()->Print();
                GetContextMenu()->Action(hists.back()->GetYaxis(),hists.back()->GetYaxis()->Class()->GetMethodAny("SetRangeUser"));
                {
                  double y1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetFirst());
                  double y2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetLast());
                  TIter iter(this->GetListOfPrimitives());
                  while(TObject *obj = iter.Next()) {
                    if(obj->InheritsFrom(TPad::Class())) {
                      TPad *pad = (TPad*)obj;
                      TIter iter2(pad->GetListOfPrimitives());
                      while(TObject *obj2=iter2.Next()) {
                        if(obj2->InheritsFrom(TH1::Class())) {
                          TH1* hist = (TH1*)obj2;
                          hist->GetYaxis()->SetRangeUser(y1,y2);
                          pad->Modified();
                          pad->Update();
                        }
                      }
                    }
                  }

                  //for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
                  //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());

                }
                edited = true;
                break;

    case kKey_s:

                if(GetNMarkers()<2){
                  edited = ShowPeaks(hists.data(),hists.size());
                  RemoveMarker("all");
                } else{
                  double x1 = fMarkers.at(fMarkers.size()-1)->localx;
                  double x2 = fMarkers.at(fMarkers.size()-2)->localx;
                  if(x1>x2)
                    std::swap(x1,x2);
                  double y1 = fMarkers.at(fMarkers.size()-1)->localy;
                  double y2 = fMarkers.at(fMarkers.size()-2)->localy;
                  if(y1>y2)
                    std::swap(y1,y2);

                  double ymax = hists.at(0)->GetMaximum();
                  double thresh = y1/ymax;
                  double sigma = x2-x1;
                  if(sigma>10.0)
                    sigma = 10.0;
                  edited = ShowPeaks(hists.data(),hists.size(),sigma,thresh);
                  RemoveMarker("all");
                }
                break;
    case kKey_S:

                if(GetNMarkers()<2){
                  edited = ShowPeaks(hists.data(),hists.size());
                  RemoveMarker("all");
                } else{
                  double x1 = fMarkers.at(fMarkers.size()-1)->localx;
                  double x2 = fMarkers.at(fMarkers.size()-2)->localx;
                  if(x1>x2)
                    std::swap(x1,x2);
                  double y1 = fMarkers.at(fMarkers.size()-1)->localy;
                  double y2 = fMarkers.at(fMarkers.size()-2)->localy;
                  if(y1>y2)
                    std::swap(y1,y2);

                  double ymax = hists.at(0)->GetMaximum();
                  double thresh = y1/ymax;
                  double sigma = 1.0;
                  if(sigma>10.0)
                    sigma = 10.0;
                  edited = ShowPeaks(hists.data(),hists.size(),sigma,thresh);
                  RemoveMarker("all");
                }
                break;
    case kKey_F9:{
                   int color =  hists.at(0)->GetLineColor() + 1;
                   if(color>9)
                     color =1;
                   hists.at(0)->SetLineColor(color);
                   edited=true;
                 }
                 break;

    case kKey_F10:{
                  }
                  break;

  };
  return edited;
}

bool GCanvas::Process1DMousePress(Int_t event,Int_t x,Int_t y)      {
  bool edited = false;
  return edited;
}

bool GCanvas::Process2DArrowKeyPress(Event_t *event,UInt_t *keysym) {
  bool edited = false;
  return edited;
}

bool GCanvas::Process2DKeyboardPress(Event_t *event,UInt_t *keysym) {
  bool edited = false;
  //printf("2d hist key pressed.\n");
  std::vector<TH1*> hists = FindHists(2);
  if(hists.size()<1)
    return edited;
  switch(*keysym) {
    case kKey_e:
      if(GetNMarkers()<2)
        break;
      {
        double x1 = fMarkers.at(fMarkers.size()-1)->localx;
        double y1 = fMarkers.at(fMarkers.size()-1)->localy;
        double x2 = fMarkers.at(fMarkers.size()-2)->localx;
        double y2 = fMarkers.at(fMarkers.size()-2)->localy;
        if(x1>x2)
          std::swap(x1,x2);
        if(y1>y2)
          std::swap(y1,y2);
        for(unsigned int i=0;i<hists.size();i++){
          hists.at(i)->GetXaxis()->SetRangeUser(x1,x2);
          hists.at(i)->GetYaxis()->SetRangeUser(y1,y2);
        }
      }
      edited = true;
      RemoveMarker("all");
      break;

    case kKey_E:
      //this->GetListOfPrimitives()->Print();
      GetContextMenu()->Action(hists.back()->GetXaxis(),hists.back()->GetXaxis()->Class()->GetMethodAny("SetRangeUser"));
      {
        double x1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetFirst());
        double x2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetXaxis()->GetLast());
        TIter iter(this->GetListOfPrimitives());
        while(TObject *obj = iter.Next()) {
          if(obj->InheritsFrom(TPad::Class())) {
            TPad *pad = (TPad*)obj;
            TIter iter2(pad->GetListOfPrimitives());
            while(TObject *obj2=iter2.Next()) {
              if(obj2->InheritsFrom(TH1::Class())) {
                TH1* hist = (TH1*)obj2;
                hist->GetXaxis()->SetRangeUser(x1,x2);
                pad->Modified();
                pad->Update();
              }
            }
          }
        }

        //for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
        //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());

      }
      edited = true;
      break;
    case kKey_g:
      if(GetNMarkers()<2)
        break;
      {
        static int cutcounter=0;
        TCutG *cut = new TCutG(Form("_cut%i",cutcounter++),9);
        //cut->SetVarX("");
        //cut->SetVarY("");
        //
        double x1 = fMarkers.at(fMarkers.size()-1)->localx;
        double y1 = fMarkers.at(fMarkers.size()-1)->localy;
        double x2 = fMarkers.at(fMarkers.size()-2)->localx;
        double y2 = fMarkers.at(fMarkers.size()-2)->localy;
        if(x1>x2)
          std::swap(x1,x2);
        if(y1>y2)
          std::swap(y1,y2);
        double xdist = (x2-x1)/2.0;
        double ydist = (y2-y1)/2.0;
        //
        //
        cut->SetPoint(0,x1,y1);
        cut->SetPoint(1,x1,y1+ydist);
        cut->SetPoint(2,x1,y2);
        cut->SetPoint(3,x1+xdist,y2);
        cut->SetPoint(4,x2,y2);
        cut->SetPoint(5,x2,y2-ydist);
        cut->SetPoint(6,x2,y1);
        cut->SetPoint(7,x2-xdist,y1);
        cut->SetPoint(8,x1,y1);
        cut->SetLineColor(kBlack);
        hists.at(0)->GetListOfFunctions()->Add(cut);

        // TODO: Bring this back once we have pulled in parts of TGRUTint
        //TGRSIint::instance()->LoadTCutG(cut);
      }
      edited = true;
      RemoveMarker("all");
      break;
    case kKey_n:
      RemoveMarker("all");
      //for(unsigned int i=0;i<hists.size();i++)
      //  hists.at(i)->GetListOfFunctions()->Delete();
      RemovePeaks(hists.data(),hists.size());
      for(unsigned int ii=0;ii<hists.size();ii++)
        hists[ii]->Sumw2(false);
      edited = true;
      break;
    case kKey_o:
      for(unsigned int i=0;i<hists.size();i++) {
        TH2* h = (TH2*)hists.at(i);
        h->GetXaxis()->UnZoom();
        h->GetYaxis()->UnZoom();
      }
      RemoveMarker("all");
      edited = true;
      break;
    case kKey_p:
      if(hists.size()<1)
        break;
      printf("you hit the p key.\n");

      break;

    case kKey_P: {
                   GH2D* ghist = NULL;
                   for(auto hist : hists){
                     if(hist->InheritsFrom(GH2Base::Class())){
                       ghist = (GH2D*)hist;
                       break;
                     }
                   }

                   if(ghist && ghist->GetProjections()->GetSize()){
                     ghist->GetProjections()->At(0)->Draw("");
                     edited=true;
                   }
                 }
                 break;
    case kKey_r:
                 if(GetNMarkers()<2)
                   break;
                 {
                   if(fMarkers.at(fMarkers.size()-1)->localy < fMarkers.at(fMarkers.size()-2)->localy)
                     for(unsigned int i=0;i<hists.size();i++)
                       hists.at(i)->GetYaxis()->SetRangeUser(fMarkers.at(fMarkers.size()-1)->localy,fMarkers.at(fMarkers.size()-2)->localy);
                   else
                     for(unsigned int i=0;i<hists.size();i++)
                       hists.at(i)->GetXaxis()->SetRangeUser(fMarkers.at(fMarkers.size()-2)->localy,fMarkers.at(fMarkers.size()-1)->localy);
                 }
                 edited = true;
                 RemoveMarker("all");
                 break;
    case kKey_R:
                 //this->GetListOfPrimitives()->Print();
                 GetContextMenu()->Action(hists.back()->GetYaxis(),hists.back()->GetYaxis()->Class()->GetMethodAny("SetRangeUser"));
                 {
                   double y1 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetFirst());
                   double y2 = hists.back()->GetXaxis()->GetBinCenter(hists.back()->GetYaxis()->GetLast());
                   TIter iter(this->GetListOfPrimitives());
                   while(TObject *obj = iter.Next()) {
                     if(obj->InheritsFrom(TPad::Class())) {
                       TPad *pad = (TPad*)obj;
                       TIter iter2(pad->GetListOfPrimitives());
                       while(TObject *obj2=iter2.Next()) {
                         if(obj2->InheritsFrom(TH1::Class())) {
                           TH1* hist = (TH1*)obj2;
                           hist->GetYaxis()->SetRangeUser(y1,y2);
                           pad->Modified();
                           pad->Update();
                         }
                       }
                     }
                   }

                   //for(int i=0;i<hists.size()-1;i++)   // this doesn't work, set range needs values not bins.   pcb.
                   //   hists.at(i)->GetXaxis()->SetRangeUser(hists.back()->GetXaxis()->GetFirst(),hists.back()->GetXaxis()->GetLast());

                 }
                 edited = true;
                 break;

    case kKey_x: {
                   GH2D* ghist = NULL;
                   for(auto hist : hists) {
                     if(hist->InheritsFrom(GH2Base::Class())){
                       ghist = (GH2D*)hist;
                       break;
                     }
                   }

                   if(ghist){
                     ghist->SetSummary(0);
                     TH1 *phist = ghist->ProjectionX();//->Draw();
                     if(phist) {
                       new GCanvas();
                       phist->Draw("");
                     }
                     edited=true;
                   }
                 }
                 break;

    case kKey_X: {
                   GH2D* ghist = NULL;
                   for(auto hist : hists) {
                     if(hist->InheritsFrom(GH2Base::Class())){
                       ghist = (GH2D*)hist;
                       break;
                     }
                   }

                   if(ghist){
                     ghist->SetSummary(true);
                     ghist->SetSummaryDirection(kYDirection);
                     TH1* phist = ghist->GetNextSummary(0,false);
                     if(phist) {
                       new GCanvas();
                       phist->Draw("");
                     }
                     edited = true;
                   }
                 }
                 break;

    case kKey_y: {
                   GH2D* ghist = NULL;
                   for(auto hist : hists) {
                     if(hist->InheritsFrom(GH2Base::Class())){
                       ghist = (GH2D*)hist;
                       break;
                     }
                   }

                   if(ghist){
                     ghist->SetSummary(0);
                     //printf("ghist = 0x%08x\n",ghist);
                     TH1 *phist = ghist->ProjectionY();//->Draw();
                     //printf("phist = 0x%08x\n",phist);
                     //printf("phist->GetName() = %s\n",phist->GetName());
                     if(phist) {
                       new GCanvas();
                       phist->Draw("");
                     }
                     edited=true;
                   }
                 }
                 break;

    case kKey_Y: {
                   GH2D* ghist = NULL;
                   for(auto hist : hists) {
                     if(hist->InheritsFrom(GH2Base::Class())){
                       ghist = (GH2D*)hist;
                       break;
                     }
                   }

                   if(ghist){
                     ghist->SetSummary(true);
                     ghist->SetSummaryDirection(kXDirection);
                     //TH1* phist = ghist->SummaryProject(1);
                     TH1* phist = ghist->GetNextSummary(0,false);
                     if(phist) {
                       new GCanvas();
                       phist->Draw("");
                     }
                     edited = true;
                   }
                 }
                 break;




    case kKey_l:
    case kKey_z:
                 if(GetLogz()){
                   // Show full y range, not restricted to positive values.
                   for(unsigned int i=0;i<hists.size();i++) {
                     hists.at(i)->GetYaxis()->UnZoom();
                   }
                   TVirtualPad *cpad = gPad;
                   this->cd();
                   gPad->SetLogz(0);
                   cpad->cd();
                 } else {
                   // Only show plot from 0 up when in log scale.
                   for(unsigned int i=0;i<hists.size();i++) {
                     if(hists.at(i)->GetYaxis()->GetXmin()<0) {
                       hists.at(i)->GetYaxis()->SetRangeUser(0,hists.at(i)->GetYaxis()->GetXmax());
                     }
                   }
                   TVirtualPad *cpad = gPad;
                   this->cd();
                   gPad->SetLogz(1);
                   cpad->cd();
                 }
                 edited = true;
                 break;
  };
  return edited;
}

bool GCanvas::Process2DMousePress(Int_t event,Int_t x,Int_t y)      {
  bool edited = false;
  return edited;
}








/*

   bool GCanvas::GausFit(GMarker *m1,GMarker *m2) {
   TIter iter(gPad->GetListOfPrimitives());
   TH1 *hist = 0;
   bool edited = false;
   while(TObject *obj = iter.Next()) {
   if( obj->InheritsFrom("TH1") &&
   !obj->InheritsFrom("TH2") &&
   !obj->InheritsFrom("TH3") ) {
   hist = (TH1*)obj;
   }
   }
   if(!hist)
   return false;
   if(!m1 || !m2) {
   if(GetNMarkers()<2) {
   return false;
   } else {
   m1 = fMarkers.at(fMarkers.size()-1);
   m2 = fMarkers.at(fMarkers.size()-2);
   }
   }

   TF1 *gausfit = hist->GetFunction("gausfit");
   if(gausfit)
   gausfit->Delete();
   int binx[2];
   double x[2];
   double y[2];
   if(m1->localx < m2->localx) {
   x[0]=m1->localx; x[1]=m2->localx;
   binx[0]=m1->x;   binx[1]=m2->x;
   y[0]=hist->GetBinContent(m1->x); y[1]=hist->GetBinContent(m2->x);
   } else {
   x[1]=m1->localx; x[0]=m2->localx;
   binx[1]=m1->x;   binx[0]=m2->x;
   y[1]=hist->GetBinContent(m1->x); y[0]=hist->GetBinContent(m2->x);
   }
//printf("x[0] = %.02f   x[1] = %.02f\n",x[0],x[1]);
gausfit = new TF1("gausfit","gaus",x[0],x[1]);
//  TF1 *gfit = new TF1("gaus","gaus",x[0],x[1]);
//  hist->Fit(gfit,"QR+");

///gausfit->SetParameters(y[0],0,gfit->GetParameter(0),gfit->GetParameter(1),gfit->GetParameter(2));

//  gfit->Delete();
//hist->GetFunction("gaus")->Delete();

hist->Fit(gausfit,"QR+");

double param[3];
double error[3];

gausfit->GetParameters(param);
error[0] = gausfit->GetParError(0);
error[1] = gausfit->GetParError(1);
error[2] = gausfit->GetParError(2);

printf("\nIntegral from % 4.01f to % 4.01f: %f\n",x[0],x[1],gausfit->Integral(x[0],x[1])/hist->GetBinWidth(1));
printf("Centroid:  % 4.02f  +/- %.02f\n",param[1],error[1]);
printf("FWHM:      % 4.02f  +/- %.02f\n",param[2]*2.35,error[2]*2.35);
double integral = gausfit->Integral(x[0],x[1])/hist->GetBinWidth(1);
double int_err  = integral*TMath::Sqrt((error[0]/param[0])*(error[0]/param[0]) +
((error[2]/param[2])*(error[2]/param[2])));
printf("Area:      % 4.02f  +/- %.02f\n",
integral,int_err);
return true;

}

bool GCanvas::PeakFit(GMarker *m1,GMarker *m2) {
  TIter iter(gPad->GetListOfPrimitives());
  TH1 *hist = 0;
  bool edited = false;
  while(TObject *obj = iter.Next()) {
    if( obj->InheritsFrom("TH1") &&
        !obj->InheritsFrom("TH2") &&
        !obj->InheritsFrom("TH3") ) {
      hist = (TH1*)obj;
    }
  }
  if(!hist)
    return false;
  if(!m1 || !m2) {
    if(GetNMarkers()<2) {
      return false;
    } else {
      m1 = fMarkers.at(fMarkers.size()-1);
      m2 = fMarkers.at(fMarkers.size()-2);
    }
  }

  // TPeak *mypeak = (TPeak*)(hist->GetFunction("peak"));
  // if(mypeak)
  //   mypeak->Delete();
  int binx[2];
  double x[2];
  double y[2];
  if(m1->localx < m2->localx) {
    x[0]=m1->localx; x[1]=m2->localx;
    binx[0]=m1->x;   binx[1]=m2->x;
    y[0]=hist->GetBinContent(m1->x); y[1]=hist->GetBinContent(m2->x);
  } else {
    x[1]=m1->localx; x[0]=m2->localx;
    binx[1]=m1->x;   binx[0]=m2->x;
    y[1]=hist->GetBinContent(m1->x); y[0]=hist->GetBinContent(m2->x);
  }
  //printf("x[0] = %.02f   x[1] = %.02f\n",x[0],x[1]);
  TPeak* mypeak = new TPeak((x[0]+x[1])/2.0,x[0],x[1]);
  //  TF1 *gfit = new TF1("gaus","gaus",x[0],x[1]);
  //  hist->Fit(gfit,"QR+");

  ///gausfit->SetParameters(y[0],0,gfit->GetParameter(0),gfit->GetParameter(1),gfit->GetParameter(2));

  //  gfit->Delete();
  //hist->GetFunction("gaus")->Delete();

  mypeak->Fit(hist,"+");
  hist->GetListOfFunctions()->Add(mypeak->Background()->Clone());
  // hist->GetListOfFunctions()->Add(mypeak);
  TPeak *peakfit = (TPeak*)(hist->GetListOfFunctions()->Last());
  //  mypeak->Background()->Draw("SAME");

  //
  //  double param[3];
  //  double error[3];
  //
  //  gausfit->GetParameters(param);
  //  error[0] = gausfit->GetParError(0);
  //  error[1] = gausfit->GetParError(1);
  //  error[2] = gausfit->GetParError(2);
  //
  //  printf("\nIntegral from % 4.01f to % 4.01f: %f\n",x[0],x[1],gausfit->Integral(x[0],x[1])/hist->GetBinWidth(1));
  //  printf("Centroid:  % 4.02f  +/- %.02f\n",param[1],error[1]);
  //  printf("FWHM:      % 4.02f  +/- %.02f\n",param[2]*2.35,error[2]*2.35);
  //  double integral = gausfit->Integral(x[0],x[1])/hist->GetBinWidth(1);
  //  double int_err  = integral*TMath::Sqrt((error[0]/param[0])*(error[0]/param[0]) +
  //                                         ((error[2]/param[2])*(error[2]/param[2])));
  //  printf("Area:      % 4.02f  +/- %.02f\n",
  //         integral,int_err);
  return true;

}


bool GCanvas::PeakFitQ(GMarker *m1,GMarker *m2) {
  TIter iter(gPad->GetListOfPrimitives());
  TH1 *hist = 0;
  bool edited = false;
  while(TObject *obj = iter.Next()) {
    if( obj->InheritsFrom("TH1") &&
        !obj->InheritsFrom("TH2") &&
        !obj->InheritsFrom("TH3") ) {
      hist = (TH1*)obj;
    }
  }
  if(!hist)
    return false;
  if(!m1 || !m2) {
    if(GetNMarkers()<2) {
      return false;
    } else {
      m1 = fMarkers.at(fMarkers.size()-1);
      m2 = fMarkers.at(fMarkers.size()-2);
    }
  }

  int binx[2];
  double x[2];
  double y[2];
  if(m1->localx < m2->localx) {
    x[0]=m1->localx; x[1]=m2->localx;
    binx[0]=m1->x;   binx[1]=m2->x;
    y[0]=hist->GetBinContent(m1->x); y[1]=hist->GetBinContent(m2->x);
  } else {
    x[1]=m1->localx; x[0]=m2->localx;
    binx[1]=m1->x;   binx[0]=m2->x;
    y[1]=hist->GetBinContent(m1->x); y[0]=hist->GetBinContent(m2->x);
  }
  //printf("x[0] = %.02f   x[1] = %.02f\n",x[0],x[1]);
  TPeak * mypeak = new TPeak((x[0]+x[1])/2.0,x[0],x[1]);
  //  if(hist->FindObject(mypeak->GetName())){
  //     //delete mypeak;
  //     mypeak = (TPeak*)(hist->FindObject(mypeak->GetName()));
  //  }
  //  TF1 *gfit = new TF1("gaus","gaus",x[0],x[1]);
  //  hist->Fit(gfit,"QR+");

  ///gausfit->SetParameters(y[0],0,gfit->GetParameter(0),gfit->GetParameter(1),gfit->GetParameter(2));

  //  gfit->Delete();
  //hist->GetFunction("gaus")->Delete();

  mypeak->Fit(hist,"Q+");
  hist->GetListOfFunctions()->Add(mypeak->Background()->Clone());
  // hist->GetListOfFunctions()->Add(mypeak);
  TPeak *peakfit = (TPeak*)(hist->GetListOfFunctions()->Last());
  //hist->GetListOfFunctions()->Print();
  if(!peakfit) {
    printf("peakfit not found??\n");
    return false;
  }
  //  mypeak->Background()->Draw("SAME");
  mypeak->Print();
  */

    /*
       double param[10];
       double error[10];
       peakfit->GetParameters(param);
       error[0] = peakfit->GetParError(0);
       error[1] = peakfit->GetParError(1);
       error[2] = peakfit->GetParError(2);
       error[3] = peakfit->GetParError(3);
       error[4] = peakfit->GetParError(4);
       error[4] = peakfit->GetParError(5);
       error[4] = peakfit->GetParError(6);
       error[4] = peakfit->GetParError(7);
       error[4] = peakfit->GetParError(8);
       error[4] = peakfit->GetParError(9);

       printf("\nIntegral from % 4.01f to % 4.01f: %f\n",x[0],x[1],peakfit->Integral(x[0],x[1])/hist->GetBinWidth(1));
       printf("Centroid:  % 4.02f  +/- %.02f\n",param[1],error[1]);
       printf("FWHM:      % 4.02f  +/- %.02f\n",fabs(param[2]*2.35),error[2]*2.35);
    // double integral = gausfit->Integral(x[0],x[1])/hist->GetBinWidth(1);
    // double int_err  = integral*TMath::Sqrt(((error[2]/param[2])*(error[2]/param[2]))+
    //                                        ((error[4]/param[4])*(error[4]/param[4])));
    // printf("Area:      % 4.02f  +/- %.02f\n",
    //        integral - (bg->Integral(x[0],x[1])/hist->GetBinWidth(1)),int_err);
    */
    //  return true;

    //}

  /*
     void GCanvas::SetBackGroundSubtractionType() {
  // used to set the background subtraction type
  // used for the p command. Current configurations
  // are:
  //
  // 0.  No background subtraction.
  // 1.  Fraction of the total projection. setting a bg level estimates the fraction.
  // 2.  From marker #3         -> make a subtract gate the same width as the project gate.
  // 3.  From marker #3 & #4    -> make a suntract gate from maker 3 and 4 the same total widthe as the project gate. Odd numebrs default to marker #4.
  // 4.  Between marker #3 & #4 -> make a subtract gate between marker 3 and 4.
  // 5.  Use marker #1 & #2     -> use the 'b' key to create a subtract projection.  Projection is not drawn but last projection made will be subtracted
  //                               in the next projection.
  //

  fBGSubtraction_type++;
  //if(fBGSubtraction_type >5)
  if(fBGSubtraction_type >4)
  fBGSubtraction_type = 0;
  printf("\n");
  switch(fBGSubtraction_type) {
  case 0:
  printf("BG subtraction off, project will not automatically subtract background.\n");
  break;
  case 1:
  printf("BG subtraction set to fraction of total projection, use b to set fraction.\n");
  break;
  case 2:
  printf("BG subtraction set to ( marker3->| ), use b to confirm subtraction gate.\n");
  break;
  case 3:
  printf("BG subtraction set to ( marker3->| ) & ( marker4->| ), use b to confirm subtraction gates.\n");
  break;
  case 4:
  printf("BG subtraction set to ( marker1->marker2 ), use b to confirm subtraction gates.\n");
  break;
  default:
  printf("Changing BG subtraction type, type is now: %i\n",fBGSubtraction_type);
  };
  Prompt();
  return;
  }

  bool GCanvas::SetBackGround(GMarker *m1,GMarker *m2,GMarker *m3,GMarker *m4) {
  ClearBGMarkers();  //removes all BG markers...
  bool edited = false;
  switch(fBGSubtraction_type) {
  case 0:
  printf(RED "\nBackground Subtraction type not set, no Background subtraction will be performed.\n" RESET_COLOR );
  break;
  case 1:
  //if(!m1) {
  if(fMarkers.size()<1) {
  printf(RED "\nPlace at least one marker to set background level.\n" RESET_COLOR );
  break;
  } else if(fMarkers.size()<2) {
  AddBGMarker(fMarkers.at(fMarkers.size()-1));
  RemoveMarker();
  } else {
  AddBGMarker(fMarkers.at(fMarkers.size()-1));
  AddBGMarker(fMarkers.at(fMarkers.size()-2));
  RemoveMarker();
  RemoveMarker();
  }

  edited = SetConstantBG();
  break;
  case 2:
  //printf(RED "\nWork in progress, check back soon; no Background subtraction will be performed.\n" RESET_COLOR );
  if(!m3) {
  printf(RED "\nThree markers need.  First two peak, three for bg.\n" RESET_COLOR );
  Prompt();
  break;
  }
edited = SetBGGate(m1,m2,m3,0);
break;
case 3:
if(!m3 || !m4) {
  printf(RED "\nFour markers need.  First two peak, three and four for split bg.\n" RESET_COLOR );
  Prompt();
  break;
}
edited = SetBGGate(m1,m2,m3,m4);
break;
case 4:
if(!m3 || !m4) {
  printf(RED "\nTwo markers need.  BG gate between marker1 and marker2.\n" RESET_COLOR );
  Prompt();
  break;
}
edited = SetBGGate(m3,m4);
//printf(RED "\nWork in progress, check back soon; no Background subtraction will be performed.\n" RESET_COLOR );
break;
case 5:
printf(RED "\nWork in progress, check back soon; no Background subtraction will be performed.\n" RESET_COLOR );
break;
};
return edited;
}

bool GCanvas::SetBGGate(GMarker *m1, GMarker *m2, GMarker *m3, GMarker *m4) {
  ClearBGMarkers();
  switch(fBGSubtraction_type) {
    case 2:
      if(!m1 || !m2 || !m3)
        return false;
      else {
        AddBGMarker(m3);

        GMarker *mark = new GMarker(*m3);
        mark->x = m3->x + (abs(m1->x - m2->x)+1);
        mark->localx = gPad->AbsPixeltoX(mark->x);
        AddBGMarker(mark);

        mark = fBG_Markers.at(0);
        mark->linex = new TLine(mark->localx,GetUymin(),mark->localx,GetUymax());
        mark->linex->SetLineColor(kBlue);
        mark->linex->Draw();

        mark = fBG_Markers.at(1);
        mark->linex = new TLine(mark->localx,GetUymin(),mark->localx,GetUymax());
        mark->linex->SetLineColor(kBlue);
        mark->linex->Draw();

        RemoveMarker(); // remove marker #3 so the project will work...
      }
      return true;
    case 3:
      if(!m1 || !m2 || !m3 || !m4)
        return false;
      else {
        AddBGMarker(m3);

        GMarker *mark = new GMarker(*m3);
        if((abs(m1->x - m2->x)%2) != 0)
          mark->x = m3->x + ((abs(m1->x - m2->x)+1)/2 + 1 );
        else
          mark->x = m3->x + (abs(m1->x - m2->x)/2 + 1);
        mark->localx = gPad->AbsPixeltoX(mark->x);
        AddBGMarker(mark);

        AddBGMarker(m4);
        mark = new GMarker(*m3);
        mark->x = m4->x + (abs(m1->x - m2->x)/2 + 1);
        mark->localx = gPad->AbsPixeltoX(mark->x);
        AddBGMarker(mark);

        for(int x=0;x<4;x++) {
          mark = fBG_Markers.at(x);
          mark->linex = new TLine(mark->localx,GetUymin(),mark->localx,GetUymax());
          mark->linex->SetLineColor(kBlue);
          mark->linex->Draw();
        }
        RemoveMarker(); // remove marker #4 so the project will work...
        RemoveMarker(); // remove marker #3 so the project will work...
      }
      return true;
    case 4:
      if(!m1 || !m2 )
        return false;
      else {
        AddBGMarker(m1);
        AddBGMarker(m2);
        for(int x=0;x<2;x++) {
          GMarker *mark = fBG_Markers.at(x);
          mark->linex = new TLine(mark->localx,GetUymin(),mark->localx,GetUymax());
          mark->linex->SetLineColor(kBlue);
          mark->linex->Draw();
        }
        RemoveMarker(); // remove marker #4 so the project will work...
        RemoveMarker(); // remove marker #3 so the project will work...
      }
      return true;
    default:
      return false;
  };
}

bool GCanvas::SetConstantBG() {
  bool edited = false;
  std::vector<TH1*> hists = FindHists();
  if(hists.size()<1)
    return edited;
  if(GetNBG_Markers()<1)
    return edited;
  OrderBGMarkers();
  TF1 *const_bg = hists.at(0)->GetFunction("const_bg");
  if(const_bg)
    const_bg->Delete();
  double x[2];
  if(GetNBG_Markers()==1) {
    x[0]=fBG_Markers.at(0)->localx; x[1]=fBG_Markers.at(0)->localx;
  } else {
    x[0]=fBG_Markers.at(0)->localx; x[1]=fBG_Markers.at(1)->localx;
  }
  const_bg = new TF1("const_bg","pol0",x[0],x[1]);
  hists.at(0)->Fit(const_bg,"QR+");
  TAxis *xaxis = hists.at(0)->GetXaxis();
  const_bg->SetRange(xaxis->GetFirst(),xaxis->GetLast());
  const_bg->Draw("SAME");
  hists.at(0)->GetListOfFunctions()->Add(const_bg);
  edited = true;
  return edited;

}

TH1 *GCanvas::GetBackGroundHist(GMarker *addlow,GMarker *addhigh) {
  std::vector<TH1*> hists = FindHists();
  if(hists.size()<1)
    return 0;
  TH1 *hist = hists.at(0);

  switch(fBGSubtraction_type) {
    case 0:
      //printf(RED "\nBackground Subtraction type not set, no Background subtraction will be performed.\n" RESET_COLOR );
      return 0;
    case 1: {
              // check that bg was been set:
              TF1 *const_bg = hist->GetFunction("const_bg");
              if(!const_bg) // not yet set.
                return 0;
              Double_t pj_total = hist->Integral(0,hist->GetNbinsX(),"width");
              if(pj_total<1)
                return 0;
              Double_t bg_frac  = (addhigh->localx-addlow->localx +1)*const_bg->GetParameter(0)/pj_total;
              //GMemObj = *mobj = GRootObjectManager::Instance()->FindObject(hist->GetName());
              //if(!mobj || !mobj->GetParent() || !mobj->GetParent()->InheritsFrom("TH2"))
              //   return 0;
              TH1 *temp  = (TH1*)hist->Clone(Form("%s_bg",hist->GetName()));
              temp ->SetTitle(Form(" - bg(frac %0.4f)",bg_frac));
              temp->Scale(bg_frac);
              return temp;
            }
    case 2: {
              TH1 *temp_bg =0;
              if(GetNBG_Markers()<2)
                return temp_bg;
              OrderBGMarkers();
              GMemObj *mobj = GRootObjectManager::Instance()->FindMemObject(hist->GetName());
              if(!mobj || !mobj->GetParent() || !mobj->GetParent()->InheritsFrom("TH2"))
                return temp_bg;
              int bin0,bin1;
              if(!strcmp(mobj->GetOption(),"ProjY")) {
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(fBG_Markers.size()-1)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(fBG_Markers.size()-2)->localx);
                temp_bg = ((TH2*)mobj->GetParent())->ProjectionX(Form("%s_bg",hist->GetName()),bin0,bin1);
              } else {
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(fBG_Markers.size()-1)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(fBG_Markers.size()-2)->localx);
                temp_bg = ((TH2*)mobj->GetParent())->ProjectionY(Form("%s_bg",hist->GetName()),bin0,bin1);
              }
              temp_bg->SetTitle(Form(" - bg(%.0f to %.0f)",fBG_Markers.at(0)->localx,fBG_Markers.at(1)->localx));
              return temp_bg;
            }
            //printf(RED "\nWork in progress, check back soon; no Background subtraction will be performed.\n" RESET_COLOR );
    case 3: {
              TH1 *temp_bg  =0;
              TH1 *temp_bg1 =0;
              if(GetNBG_Markers()<4)
                return temp_bg;
              OrderBGMarkers();
              GMemObj *mobj = GRootObjectManager::Instance()->FindMemObject(hist->GetName());
              if(!mobj || !mobj->GetParent() || !mobj->GetParent()->InheritsFrom("TH2"))
                return temp_bg;
              int bin0,bin1;
              if(!strcmp(mobj->GetOption(),"ProjY")) {
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(0)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(1)->localx);
                temp_bg = ((TH2*)mobj->GetParent())->ProjectionX(Form("%s_bg",hist->GetName()),bin0,bin1);
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(2)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(3)->localx);
                temp_bg1 = ((TH2*)mobj->GetParent())->ProjectionX(Form("%s_bg",hist->GetName()),bin0,bin1);
              } else {
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(0)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(1)->localx);
                temp_bg = ((TH2*)mobj->GetParent())->ProjectionY(Form("%s_bg",hist->GetName()),bin0,bin1);
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(2)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(3)->localx);
                temp_bg1 = ((TH2*)mobj->GetParent())->ProjectionY(Form("%s_bg",hist->GetName()),bin0,bin1);
              }
              temp_bg->Add(temp_bg1,1);
              temp_bg->SetTitle(Form(" - bg(%.0f to %.0f and %.0f to %.0f)",fBG_Markers.at(0)->localx,fBG_Markers.at(1)->localx,
                    fBG_Markers.at(2)->localx,fBG_Markers.at(3)->localx));
              return temp_bg;
            }
            //printf(RED "\nWork in progress, check back soon; no Background subtraction will be performed.\n" RESET_COLOR );
    case 4: {
              TH1 *temp_bg  =0;
              if(GetNBG_Markers()<2)
                return temp_bg;
              OrderBGMarkers();
              GMemObj *mobj = GRootObjectManager::Instance()->FindMemObject(hist->GetName());
              if(!mobj || !mobj->GetParent() || !mobj->GetParent()->InheritsFrom("TH2"))
                return temp_bg;
              int bin0,bin1;
              if(!strcmp(mobj->GetOption(),"ProjY")) {
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(0)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(1)->localx);
                temp_bg = ((TH2*)mobj->GetParent())->ProjectionX(Form("%s_bg",hist->GetName()),bin0,bin1);
              } else {
                bin1 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(0)->localx);
                bin0 = ((TH2*)mobj->GetParent())->GetXaxis()->FindBin(fBG_Markers.at(1)->localx);
                temp_bg = ((TH2*)mobj->GetParent())->ProjectionY(Form("%s_bg",hist->GetName()),bin0,bin1);
              }
              temp_bg->SetTitle(Form(" - bg(%.0f to %.0f)",fBG_Markers.at(0)->localx,fBG_Markers.at(1)->localx));
              return temp_bg;
            }
            //printf(RED "\nWork in progress, check back soon; no Background subtraction will be performed.\n" RESET_COLOR );
    case 5:
            printf(RED "\nWork in progress, check back soon; no Background subtraction will be performed.\n" RESET_COLOR );
            break;

  };
  return 0;
}
*/
