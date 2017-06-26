#include "TGRSIFit.h"

/// \cond CLASSIMP
ClassImp(TGRSIFit)
   /// \endcond

   TString TGRSIFit::fDefaultFitType("");

TGRSIFit::TGRSIFit()
{
   Clear();
}

TGRSIFit::TGRSIFit(const TGRSIFit& copy) : TF1(copy)
{
   copy.Copy(*this);
}

TGRSIFit::~TGRSIFit()
{
   this->AddToGlobalList(kFALSE);
}

void TGRSIFit::Copy(TObject& obj) const
{
   dynamic_cast<TGRSIFit&>(obj).fInitFlag    = fInitFlag;
   dynamic_cast<TGRSIFit&>(obj).fGoodFitFlag = fGoodFitFlag;
   TF1::Copy(obj);
}

void TGRSIFit::Print(Option_t* opt) const
{
   if(strchr(opt, '+') != nullptr) {
      printf("Params Init: %d\n", fInitFlag);
      printf("Good Fit:    %d\n", fGoodFitFlag);
      TNamed::Print(opt);
   }
}

void TGRSIFit::Clear(Option_t*)
{
   fInitFlag    = false;
   fGoodFitFlag = false;
   fDefaultFitType.Clear();
}

void TGRSIFit::ClearParameters(Option_t*)
{
   for(int i = 0; i < GetNpar(); ++i) {
      SetParameter(i, 0);
   }
}

void TGRSIFit::CopyParameters(TF1* copy) const
{
   if(!copy) {
      return;
   }
   for(int i = 0; i < GetNpar(); ++i) {
      if(copy->GetNpar() > i) {
         copy->SetParameter(i, GetParameter(i));
      }
   }
}

Bool_t TGRSIFit::AddToGlobalList(Bool_t on)
{
   // Add to global list of functions (gROOT->GetListOfFunctions() )
   // return previous status (true of functions was already in the list false if not)
   if(!gROOT) {
      return false;
   }

   if(on) {
      if(gROOT->GetListOfFunctions()->FindObject(this) != nullptr) {
         return true;
      }
      gROOT->GetListOfFunctions()->Add(this);
      // do I need to delete previous one with the same name ???
      // TF1*  old = dynamic_cast<TF1*>( gROOT->GetListOfFunctions()->FindObject(GetName()) );
      // if (old) gROOT->GetListOfFunctions()->Remove(old);
      return false;
   } else {
      // if previous status was on and now is off
      TF1* old = dynamic_cast<TF1*>(gROOT->GetListOfFunctions()->FindObject(this));
      if(!old) {
         // Warning("AddToGlobalList","Function is supposed to be in the global list but it is not present");
         return false;
      }
      gROOT->GetListOfFunctions()->Remove(this);
      return true;
   }
   return true;
}

Bool_t TGRSIFit::AddToGlobalList(TF1* func, Bool_t on)
{
   // Add to global list of functions (gROOT->GetListOfFunctions() )
   // return previous status (true of functions was already in the list false if not)

   if(!gROOT) {
      return false;
   }

   if(on) {
      if(gROOT->GetListOfFunctions()->FindObject(func) != nullptr) {
         return true;
      }
      gROOT->GetListOfFunctions()->Add(func);
      // do I need to delete previous one with the same name ???
      // TF1*  old = dynamic_cast<TF1*>( gROOT->GetListOfFunctions()->FindObject(GetName()) );
      // if(old) gROOT->GetListOfFunctions()->Remove(old);
      return false;
   } else {
      // if previous status was on and now is off
      TF1* old = dynamic_cast<TF1*>(gROOT->GetListOfFunctions()->FindObject(func));
      if(!old) {
         // func->Warning("AddToGlobalList","Function is supposed to be in the global list but it is not present");
         return false;
      }
      gROOT->GetListOfFunctions()->Remove(func);
      return true;
   }
   return true;
}
