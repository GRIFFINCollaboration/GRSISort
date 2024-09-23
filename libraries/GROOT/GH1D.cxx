#include "GH1D.h"

#include <iostream>
#include <fstream>
#include <cstring>

#include "TVirtualPad.h"
#include "TString.h"
#include "TF1.h"
#include "TFrame.h"
//#include "TROOT.h"
//#include "TSystem.h"

#include "GCanvas.h"
#include "GH2I.h"
#include "GH2D.h"

GH1D::GH1D(const TH1& source) : parent(nullptr), projection_axis(-1)
{
   source.Copy(*this);
}

GH1D::GH1D(const TF1& function, Int_t nbinsx, Double_t xlow, Double_t xup)
   : TH1D(Form("%s_hist", function.GetName()), Form("%s_hist", function.GetName()), nbinsx, xlow, xup), parent(nullptr),
     projection_axis(-1)
{
   for(int i = 0; i < nbinsx; i++) {
      Fill(GetBinCenter(i), function.Eval(i));
   }
}

bool GH1D::WriteDatFile(const char* outFile)
{
   if(strlen(outFile) < 1) {
      return false;
   }

   std::ofstream out;
   out.open(outFile);

   if(!(out.is_open())) {
      return false;
   }

   for(int i = 0; i < GetNbinsX(); i++) {
      out << GetXaxis()->GetBinCenter(i) << "\t" << GetBinContent(i) << std::endl;
   }
   out << std::endl;
   out.close();

   return true;
}

/*
GH1D::GH1D(const TH1 *source)
  : parent(nullptr), projection_axis(-1) {
  if(source->GetDiminsion()>1) {
    return;
  }

  // Can copy from any 1-d TH1, not just a TH1D
  source->Copy(*this);

  // Force a refresh of any parameters stored in the option string.
  SetOption(GetOption());
}

void GH1D::SetOption(Option_t* opt) {
  fOption = opt;

  TString sopt = opt;
  if(sopt.Index("axis:")) {
    projection_axis = 0;// TODO
  }
}
*/

void GH1D::Clear(Option_t* opt)
{
   TH1D::Clear(opt);
   parent = nullptr;
}

void GH1D::Print(Option_t* opt) const
{
   TH1D::Print(opt);
   std::cout << "\tParent: " << parent.GetObject() << std::endl;
}

void GH1D::Copy(TObject& obj) const
{
   TH1D::Copy(obj);

   static_cast<GH1D&>(obj).parent = parent;
}

void GH1D::Draw(Option_t* opt)
{
   TString option(opt);
   if(option.Contains("new", TString::kIgnoreCase)) {
      option.ReplaceAll("new", "");
      new GCanvas;
   }
   TH1D::Draw(option.Data());
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
TH1* GH1D::DrawCopy(Option_t* opt) const
{
   TH1* hist = TH1D::DrawCopy(opt);
#else
TH1* GH1D::DrawCopy(Option_t* opt, const char* name_postfix) const
{
   TH1* hist = TH1D::DrawCopy(opt, name_postfix);
#endif
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return hist;
}

TH1* GH1D::DrawNormalized(Option_t* opt, Double_t norm) const
{
   TH1* hist = TH1D::DrawNormalized(opt, norm);
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return hist;
}

GH1D* GH1D::GetPrevious(bool DrawEmpty) const
{
   if((parent.GetObject() != nullptr) && parent.GetObject()->InheritsFrom(GH2Base::Class())) {
      GH2D* gpar  = static_cast<GH2D*>(parent.GetObject());
      int   first = GetXaxis()->GetFirst();
      int   last  = GetXaxis()->GetLast();
      GH1D* prev  = gpar->GetPrevious(this, DrawEmpty);
      prev->GetXaxis()->SetRange(first, last);
      return prev;   // gpar->GetPrevious(this,DrawEmpty);
   }
   return nullptr;
}

GH1D* GH1D::GetNext(bool DrawEmpty) const
{
   if((parent.GetObject() != nullptr) && parent.GetObject()->InheritsFrom(GH2Base::Class())) {
      GH2D* gpar  = static_cast<GH2D*>(parent.GetObject());
      int   first = GetXaxis()->GetFirst();
      int   last  = GetXaxis()->GetLast();
      GH1D* next  = gpar->GetNext(this, DrawEmpty);
      next->GetXaxis()->SetRange(first, last);
      return next;   // gpar->GetNext(this,DrawEmpty);
   }
   return nullptr;
}

GH1D* GH1D::Project(double value_low, double value_high) const
{

   if((parent.GetObject() != nullptr) && parent.GetObject()->InheritsFrom(GH2Base::Class()) && projection_axis != -1) {
      if(value_low > value_high) {
         std::swap(value_low, value_high);
      }
      GH2D* gpar = static_cast<GH2D*>(parent.GetObject());
      if(projection_axis == 0) {
         int bin_low  = gpar->GetXaxis()->FindBin(value_low);
         int bin_high = gpar->GetXaxis()->FindBin(value_high);
         return gpar->ProjectionY("_py", bin_low, bin_high);
      }
      int bin_low  = gpar->GetYaxis()->FindBin(value_low);
      int bin_high = gpar->GetYaxis()->FindBin(value_high);
      return gpar->ProjectionX("_px", bin_low, bin_high);
   }
   return nullptr;
}

GH1D* GH1D::Project_Background(double value_low, double value_high, double bg_value_low, double bg_value_high,
                               EBackgroundSubtraction mode) const
{
   if((parent.GetObject() != nullptr) && parent.GetObject()->InheritsFrom(GH2Base::Class()) && projection_axis != -1) {
      if(value_low > value_high) {
         std::swap(value_low, value_high);
      }
      if(bg_value_low > bg_value_high) {
         std::swap(bg_value_low, bg_value_high);
      }

      GH2D* gpar = static_cast<GH2D*>(parent.GetObject());
      if(projection_axis == 0) {
         int bin_low     = gpar->GetXaxis()->FindBin(value_low);
         int bin_high    = gpar->GetXaxis()->FindBin(value_high);
         int bg_bin_low  = gpar->GetXaxis()->FindBin(bg_value_low);
         int bg_bin_high = gpar->GetXaxis()->FindBin(bg_value_high);

         return gpar->ProjectionY_Background(bin_low, bin_high, bg_bin_low, bg_bin_high, mode);
      }
      int bin_low     = gpar->GetYaxis()->FindBin(value_low);
      int bin_high    = gpar->GetYaxis()->FindBin(value_high);
      int bg_bin_low  = gpar->GetYaxis()->FindBin(bg_value_low);
      int bg_bin_high = gpar->GetYaxis()->FindBin(bg_value_high);

      return gpar->ProjectionX_Background(bin_low, bin_high, bg_bin_low, bg_bin_high, mode);
   }
   return nullptr;
}

GH1D* GH1D::Project(int bins)
{
   GH1D*  proj = nullptr;
   double ymax = GetMinimum();
   double ymin = GetMaximum();
   if(bins == -1) {
      bins = static_cast<int>(std::abs(ymax - ymin));
      if(bins < 1) {
         bins = 100;
      }
   }
   proj = new GH1D(Form("%s_y_axis_projection", GetName()), Form("%s_y_axis_projection", GetName()), bins, ymin, ymax);
   for(int i = 0; i < GetNbinsX(); i++) {
      if(GetBinContent(i) != 0) {
         proj->Fill(GetBinContent(i));
      }
   }

   return proj;
}
