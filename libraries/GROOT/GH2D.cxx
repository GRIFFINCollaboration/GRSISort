#include "GH2D.h"

#include <iostream>

#include <TDirectory.h>

#include "GH1D.h"

ClassImp(GH2D)

GH2D::GH2D(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins,Int_t nbinsy, const Double_t *ybins) :
  TH2D(name,title,nbinsx,xbins,nbinsy,ybins), GH2Base() { 
}


GH2D::GH2D(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins,Int_t nbinsy, const Float_t *ybins) :
  TH2D(name,title,nbinsx,xbins,nbinsy,ybins), GH2Base() {
}


GH2D::GH2D(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins,
                                            Int_t nbinsy, Double_t ylow, Double_t yup) :
  TH2D(name,title,nbinsx,xbins,nbinsy,ylow,yup), GH2Base() {
}


GH2D::GH2D(const char *name,const char *title,Int_t nbinsx, Double_t xlow, Double_t xup,
                                            Int_t nbinsy, Double_t *ybins) :
  TH2D(name,title,nbinsx,xlow,xup,nbinsy,ybins), GH2Base() {
}


GH2D::GH2D(const char *name,const char *title,Int_t nbinsx, Double_t xlow, Double_t xup,
                                            Int_t nbinsy, Double_t ylow, Double_t yup) :
  TH2D(name,title,nbinsx,xlow,xup,nbinsy,ylow,yup), GH2Base() {
}

GH2D::GH2D(const TObject &obj) {
  if(obj.InheritsFrom(TH2::Class())){
    obj.Copy(*this);
  }
}

GH2D::~GH2D() {  }


void GH2D::Copy(TObject &obj) const {
  TH2::Copy(obj);
  //fProjections->Copy(*(((GH2D&)obj).fProjections));
  //fSummaryProjections->Copy(*(((GH2D&)obj).fSummaryProjections));
}

TObject *GH2D::Clone(const char *newname) const {
  std::string name = newname;
  if(!name.length())
    name = Form("%s_clone",GetName());
  return TH2::Clone(name.c_str());
}

void GH2D::Clear(Option_t *opt) {
  TString sopt(opt);
  if(!sopt.Contains("projonly")){
    TH2D::Clear(opt);
  }
  GH2Clear();
}

void GH2D::Print(Option_t *opt) const { }

void GH2D::Draw(Option_t *opt) {
  std::string option = opt;
  if(option == ""){
    option = "colz";
  }
  TH2D::Draw(option.c_str());
  if(gPad) {
    gPad->Update();
    gPad->GetFrame()->SetBit(TBox::kCannotMove);
  }
}


void GH2D::Draw(TCutG *cut) {
  if(!cut)
    return;
  std::string option = Form("colz [%s]",cut->GetName());
  TH2D::Draw(option.c_str());
}



TH1 *GH2D::DrawCopy(Option_t *opt) const {
  TH1 *h = TH2D::DrawCopy(opt);
  if(gPad) {
    gPad->Update();
    gPad->GetFrame()->SetBit(TBox::kCannotMove);
  }
  return h;
}

TH1 *GH2D::DrawNormalized(Option_t *opt,Double_t norm) const {
  TH1 *h = TH2D::DrawNormalized(opt,norm);
  if(gPad) {
    gPad->Update();
    gPad->GetFrame()->SetBit(TBox::kCannotMove);
  }
  return h;
}



GH1D* GH2D::ProjectionX(const char* name,
                       int firstbin,
                       int lastbin,
                       Option_t* option) {
  return GH2ProjectionX(name,firstbin,lastbin,option);
}



GH1D* GH2D::ProjectionY(const char* name,
                       int firstbin,
                       int lastbin,
                       Option_t* option) {
  return GH2ProjectionY(name,firstbin,lastbin,option);
}

/*
void GH2D::Streamer(TBuffer &b) {
  if(b.IsReading()) {
    Version_t v = b.ReadVersion(); 
    TH2D::Streamer(b);
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
    b.WriteVersion(GH2D::IsA());
    TH2D::Streamer(b);
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







