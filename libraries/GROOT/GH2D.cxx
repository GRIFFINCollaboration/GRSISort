#include "GH2D.h"

#include <iostream>

#include "TDirectory.h"

#include "GH1D.h"

GH2D::GH2D(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, const Double_t* ybins)
   : TH2D(name, title, nbinsx, xbins, nbinsy, ybins)
{
}

GH2D::GH2D(const char* name, const char* title, Int_t nbinsx, const Float_t* xbins, Int_t nbinsy, const Float_t* ybins)
   : TH2D(name, title, nbinsx, xbins, nbinsy, ybins)
{
}

GH2D::GH2D(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, Double_t ylow, Double_t yup)
   : TH2D(name, title, nbinsx, xbins, nbinsy, ylow, yup)
{
}

GH2D::GH2D(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t* ybins)
   : TH2D(name, title, nbinsx, xlow, xup, nbinsy, ybins)
{
}

GH2D::GH2D(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup)
   : TH2D(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup)
{
}

GH2D::GH2D(const TObject& obj)
{
   if(obj.InheritsFrom(TH2::Class())) {
      obj.Copy(*this);
   }
}

GH2D::~GH2D() = default;

void GH2D::Copy(TObject& obj) const
{
   TH2::Copy(obj);
   // fProjections->Copy(*(((GH2D&)obj).fProjections));
   // fSummaryProjections->Copy(*(((GH2D&)obj).fSummaryProjections));
}

TObject* GH2D::Clone(const char* newname) const
{
   std::string name = newname;
   if(name.length() == 0u) {
      name = Form("%s_clone", GetName());
   }
   return TH2::Clone(name.c_str());
}

void GH2D::Clear(Option_t* opt)
{
   TString sopt(opt);
   if(!sopt.Contains("projonly")) {
      TH2D::Clear(opt);
   }
   GH2Clear();
}

void GH2D::Print(Option_t*) const
{
}

void GH2D::Draw(Option_t* opt)
{
   std::string option = opt;
   if(option.empty()) {
      option = "colz";
   }
   TH2D::Draw(option.c_str());
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
}

void GH2D::Draw(TCutG* cut)
{
   if(cut == nullptr) {
      return;
   }
   std::string option = Form("colz [%s]", cut->GetName());
   TH2D::Draw(option.c_str());
}

TH1* GH2D::DrawCopy(Option_t* opt, const char* name_postfix) const
{
   TH1* hist = TH2D::DrawCopy(opt, name_postfix);
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return hist;
}

TH1* GH2D::DrawNormalized(Option_t* opt, Double_t norm) const
{
   TH1* hist = TH2D::DrawNormalized(opt, norm);
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return hist;
}

GH1D* GH2D::ProjectionX(const char* name, int firstbin, int lastbin, Option_t* option)
{
   return GH2ProjectionX(name, firstbin, lastbin, option);
}

GH1D* GH2D::ProjectionY(const char* name, int firstbin, int lastbin, Option_t* option)
{
   return GH2ProjectionY(name, firstbin, lastbin, option);
}
