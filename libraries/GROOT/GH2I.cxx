#include "GH2I.h"

#include <iostream>

#include <TDirectory.h>

#include "GH1D.h"

ClassImp(GH2I)

GH2I::GH2I(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins,Int_t nbinsy, const Double_t *ybins) :
  TH2I(name,title,nbinsx,xbins,nbinsy,ybins), GH2Base() { 
}


GH2I::GH2I(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins,Int_t nbinsy, const Float_t *ybins) :
  TH2I(name,title,nbinsx,xbins,nbinsy,ybins), GH2Base() {
}


GH2I::GH2I(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins,
                                            Int_t nbinsy, Double_t ylow, Double_t yup) :
  TH2I(name,title,nbinsx,xbins,nbinsy,ylow,yup), GH2Base() {
}


GH2I::GH2I(const char *name,const char *title,Int_t nbinsx, Double_t xlow, Double_t xup,
                                            Int_t nbinsy, Double_t *ybins) :
  TH2I(name,title,nbinsx,xlow,xup,nbinsy,ybins), GH2Base() {
}


GH2I::GH2I(const char *name,const char *title,Int_t nbinsx, Double_t xlow, Double_t xup,
                                            Int_t nbinsy, Double_t ylow, Double_t yup) :
  TH2I(name,title,nbinsx,xlow,xup,nbinsy,ylow,yup), GH2Base() {
}

GH2I::GH2I(const TObject &obj) {
  if(obj.InheritsFrom(TH2::Class())){
    obj.Copy(*this);
  }
}

GH2I::~GH2I() {  }


void GH2I::Copy(TObject &obj) const {
  TH2::Copy(obj);
  //fProjections->Copy(*(((GH2I&)obj).fProjections));
  //fSummaryProjections->Copy(*(((GH2I&)obj).fSummaryProjections));
}

TObject *GH2I::Clone(const char *newname) const {
  std::string name = newname;
  if(!name.length())
    name = Form("%s_clone",GetName());
  return TH2::Clone(name.c_str());
}

void GH2I::Clear(Option_t *opt) {
  TString sopt(opt);
  if(!sopt.Contains("projonly")){
    TH2I::Clear(opt);
  }
  GH2Clear();
}

void GH2I::Print(Option_t *opt) const { }

void GH2I::Draw(Option_t *opt) {
  std::string option = opt;
  if(option == ""){
    option = "colz";
  }
  TH2I::Draw(option.c_str());
  if(gPad) {
    gPad->Update();
    gPad->GetFrame()->SetBit(TBox::kCannotMove);
  }
}

TH1 *GH2I::DrawCopy(Option_t *opt) const {
  TH1 *h = TH2I::DrawCopy(opt);
  if(gPad) {
    gPad->Update();
    gPad->GetFrame()->SetBit(TBox::kCannotMove);
  }
  return h;
}

TH1 *GH2I::DrawNormalized(Option_t *opt,Double_t norm) const {
  TH1 *h = TH2I::DrawNormalized(opt,norm);
  if(gPad) {
    gPad->Update();
    gPad->GetFrame()->SetBit(TBox::kCannotMove);
  }
  return h;
}










GH1D* GH2I::ProjectionX(const char* name,
                       int firstbin,
                       int lastbin,
                       Option_t* option) {
  return GH2ProjectionX(name,firstbin,lastbin,option);
}



GH1D* GH2I::ProjectionY(const char* name,
                       int firstbin,
                       int lastbin,
                       Option_t* option) {
  return GH2ProjectionY(name,firstbin,lastbin,option);
}

/*
void GH2I::Streamer(TBuffer &b) {
  if(b.IsReading()) {
    Version_t v = b.ReadVersion(); 
    TH2I::Streamer(b);
    TDirectory *current = gDirectory;
    if(TDirectory::Cd(Form("%s_projections",this->GetName()))) { 
      TList *list = gDirectory->GetList();
      TIter iter(list);
      while(TObject *obj = iter.Next()) {
        if(obj->InheritsFrom(TH1::Class())) {
          GH1D *h = new GH1D(*obj);
          h->SetParent(this);
          fProjections.Add(h);
        }  
      }
    }
    current->cd();
  } else {
    b.WriteVersion(GH2I::IsA());
    TH2I::Streamer(b);
    if(fProjections.GetEntries()) {
      TDirectory *current = gDirectory;
      TDirectory *newdir  =  current->mkdir(Form("%s_projections",this->GetName());
      newdir->cd();
      fProjections->Write();
      current->cd();
    }

  }
}
*/







