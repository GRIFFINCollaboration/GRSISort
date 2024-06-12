#include "GH2I.h"

#include <iostream>

#include <TDirectory.h>

#include "GH1D.h"

ClassImp(GH2I)

   GH2I::GH2I(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy,
              const Double_t* ybins)
   : TH2I(name, title, nbinsx, xbins, nbinsy, ybins), GH2Base()
{
}

GH2I::GH2I(const char* name, const char* title, Int_t nbinsx, const Float_t* xbins, Int_t nbinsy, const Float_t* ybins)
   : TH2I(name, title, nbinsx, xbins, nbinsy, ybins), GH2Base()
{
}

GH2I::GH2I(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, Double_t ylow,
           Double_t yup)
   : TH2I(name, title, nbinsx, xbins, nbinsy, ylow, yup), GH2Base()
{
}

GH2I::GH2I(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy,
           Double_t* ybins)
   : TH2I(name, title, nbinsx, xlow, xup, nbinsy, ybins), GH2Base()
{
}

GH2I::GH2I(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow,
           Double_t yup)
   : TH2I(name, title, nbinsx, xlow, xup, nbinsy, ylow, yup), GH2Base()
{
}

GH2I::GH2I(const TObject& obj)
{
   if(obj.InheritsFrom(TH2::Class())) {
      obj.Copy(*this);
   }
}

GH2I::~GH2I() = default;

void GH2I::Copy(TObject& obj) const
{
   TH2::Copy(obj);
   // fProjections->Copy(*(((GH2I&)obj).fProjections));
   // fSummaryProjections->Copy(*(((GH2I&)obj).fSummaryProjections));
}

TObject* GH2I::Clone(const char* newname) const
{
   std::string name = newname;
   if(name.length() == 0u) {
      name = Form("%s_clone", GetName());
   }
   return TH2::Clone(name.c_str());
}

void GH2I::Clear(Option_t* opt)
{
   TString sopt(opt);
   if(!sopt.Contains("projonly")) {
      TH2I::Clear(opt);
   }
   GH2Clear();
}

void GH2I::Print(Option_t*) const
{
}

void GH2I::Draw(Option_t* opt)
{
   std::string option = opt;
   if(option == "") {
      option = "colz";
   }
   TH2I::Draw(option.c_str());
   if(gPad) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
TH1* GH2I::DrawCopy(Option_t* opt) const
{
   TH1* h = TH2I::DrawCopy(opt);
#else
TH1* GH2I::DrawCopy(Option_t* opt, const char* name_postfix) const
{
   TH1* h = TH2I::DrawCopy(opt, name_postfix);
#endif
   if(gPad) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return h;
}

TH1* GH2I::DrawNormalized(Option_t* opt, Double_t norm) const
{
   TH1* h = TH2I::DrawNormalized(opt, norm);
   if(gPad) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return h;
}

GH1D* GH2I::ProjectionX(const char* name, int firstbin, int lastbin, Option_t* option)
{
   return GH2ProjectionX(name, firstbin, lastbin, option);
}

GH1D* GH2I::ProjectionY(const char* name, int firstbin, int lastbin, Option_t* option)
{
   return GH2ProjectionY(name, firstbin, lastbin, option);
}
