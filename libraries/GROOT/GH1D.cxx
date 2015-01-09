
#include <TPad.h>
#include <TString.h>
#include <TList.h>
#include <TBox.h>

#include "GH1D.h"
#include "GCanvas.h"

#include "Globals.h"

int GH1D::fUniqueId = 0;
std::map <GH1D*,int> GH1D::fCurrentHistMap;


GH1D::GH1D() 
      :TH1D() {  InitGH1D();  }

GH1D::~GH1D() { 
   RemoveFromMap(this);   
}


GH1D::GH1D(const TVectorD& v) 
      :TH1D(v) {  InitGH1D();  }

GH1D::GH1D(const char *name, const char *title,Int_t nbinsx,const Float_t *xbins)
      :TH1D(name,title,nbinsx,xbins)         {  InitGH1D();  }

GH1D::GH1D(const char *name, const char *title,Int_t nbinsx,const Double_t *xbins)
      :TH1D(name,title,nbinsx,xbins)        {  InitGH1D();  }

GH1D::GH1D(const char *name, const char *title,Int_t nbinsx,Double_t xlow, Double_t xup)
      :TH1D(name,title,nbinsx,xlow,xup)  {  InitGH1D();  }


//void GH1D::ExecuteEvent(Int_t event, Int_t px, Int_t py) {
//   printf("Execute event  (%i,%i)   0x%08x\n",px,py,event);
//}

void GH1D::InitGH1D() {
   AddToMap(this); 
}

void GH1D::Draw(Option_t *option) {

   TString opt1 = option; opt1.ToLower();
   TString opt2 = option;
   Int_t index  = opt1.Index("same");

   if (index>=0) {
      Int_t indb = opt1.Index("[");
      if (indb>=0) {
         Int_t indk = opt1.Index("]");
         if (index>indb && index<indk) index = -1;
      }
   }
 
   GCanvas  *tgc_ptr = 0;

   if (gPad) {
      if (!gPad->IsEditable()) tgc_ptr = new GCanvas();  //gROOT->MakeDefCanvas();
      if (index>=0) {
         if (gPad->GetX1() == 0   && gPad->GetX2() == 1 &&
             gPad->GetY1() == 0   && gPad->GetY2() == 1 &&
             gPad->GetListOfPrimitives()->GetSize()==0) opt2.Remove(index,4);
      } else {
         //the following statement is necessary in case one attempts to draw
         //a temporary histogram already in the current pad
         if (TestBit(kCanDelete)) gPad->GetListOfPrimitives()->Remove(this);
         gPad->Clear();
      }
   } else {
      if (index>=0) opt2.Remove(index,4);
      tgc_ptr = new GCanvas;
   }

   TH1D::Draw(opt2.Data());
   gPad->FindObject("TFrame");//->SetBit(TBox::kCannotMove);
   gPad->GetListOfPrimitives()->Print();
   printf("Draw called\t\t%p\t%p\n",gPad,gPad->FindObject("TFrame"));
}


void GH1D::AddToMap(GH1D *hist) {
   fCurrentHistMap.insert(std::make_pair(hist,fUniqueId++));
}

void GH1D::RemoveFromMap(GH1D *hist) {
   if(fCurrentHistMap.count(hist)==1) 
      fCurrentHistMap.erase(fCurrentHistMap.find(hist));
}

void GH1D::CheckMapStats() {
   printf("Number of GH1Ds in memory[%i]:\n",fCurrentHistMap.size());
   std::map<GH1D*,int>::iterator iter; int x=0;
   for (iter = fCurrentHistMap.begin(); iter != fCurrentHistMap.end(); iter++) {
      printf("\thist[%i]: fUnique = %i\t%s\n",x++,iter->second,iter->first->GetName());
   }
   printf("-----------------------\n");
}


void GH1D::ExecuteEvent(Int_t evnet,Int_t x,Int_t y) {
   printf(DYELLOW);
   printf("GH1D Execute event called.\n");
   printf(RESET_COLOR);
}
