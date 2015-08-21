#include "TGRSIFit.h"

ClassImp(TGRSIFit);

TString TGRSIFit::fDefaultFitType("");

TGRSIFit::TGRSIFit(){
   this->Clear();
}

TGRSIFit::TGRSIFit(const TGRSIFit &copy) : TF1(copy){
   ((TGRSIFit&)copy).Copy(*this);
}

TGRSIFit::~TGRSIFit(){
   this->AddToGlobalList(kFALSE);
}

void TGRSIFit::Copy(TObject &obj) const{
   ((TGRSIFit&)obj).init_flag   = init_flag;
   ((TGRSIFit&)obj).goodfit_flag= goodfit_flag;
   TF1::Copy(obj);
}

void TGRSIFit::Print(Option_t *opt) const {
   if(strchr(opt,'+') != NULL){
      printf("Params Init: %d\n", init_flag);
      printf("Good Fit:    %d\n", goodfit_flag);
      TNamed::Print(opt);
   }
}

void TGRSIFit::Clear(Option_t *opt) {
   init_flag = false;
   goodfit_flag = false;
   fDefaultFitType.Clear();
}

Bool_t TGRSIFit::AddToGlobalList(Bool_t on){
   // Add to global list of functions (gROOT->GetListOfFunctions() )
   // return previous status (true of functions was already in the list false if not)
   
   if (!gROOT) return false;

   if (on )  {
      if(gROOT->GetListOfFunctions()->FindObject(this) != nullptr){
         return true;
      }
      gROOT->GetListOfFunctions()->Add(this);
      // do I need to delete previous one with the same name ???
      //TF1 * old = dynamic_cast<TF1*>( gROOT->GetListOfFunctions()->FindObject(GetName()) );
      //if (old) gROOT->GetListOfFunctions()->Remove(old);
      return false;
   }
   else {
      // if previous status was on and now is off
      TF1 * old = dynamic_cast<TF1*>( gROOT->GetListOfFunctions()->FindObject(this) );
      if (!old) {
         //Warning("AddToGlobalList","Function is supposed to be in the global list but it is not present");
         return false;
      }
      gROOT->GetListOfFunctions()->Remove(this);
      return true;
   }
   return true;
}

Bool_t TGRSIFit::AddToGlobalList(TF1* func, Bool_t on){
   // Add to global list of functions (gROOT->GetListOfFunctions() )
   // return previous status (true of functions was already in the list false if not)
   
   if (!gROOT) return false;

   if (on )  {
      if(gROOT->GetListOfFunctions()->FindObject(func) != nullptr){
         return true;
      }
      gROOT->GetListOfFunctions()->Add(func);
      // do I need to delete previous one with the same name ???
      //TF1 * old = dynamic_cast<TF1*>( gROOT->GetListOfFunctions()->FindObject(GetName()) );
      //if (old) gROOT->GetListOfFunctions()->Remove(old);
      return false;
   }
   else {
      // if previous status was on and now is off
      TF1 * old = dynamic_cast<TF1*>( gROOT->GetListOfFunctions()->FindObject(func) );
      if (!old) {
         //func->Warning("AddToGlobalList","Function is supposed to be in the global list but it is not present");
         return false;
      }
      gROOT->GetListOfFunctions()->Remove(func);
      return true;
   }
   return true;
}

//This delete is here because we want things to be removed from the global list when we delete them. 
//However, the delete[] operator does not work, so one might cause a seg fault when they use a dynamic array
//of TGRSIFits. So don't do it until this is sorted out. Make a vector or something instead.
void TGRSIFit::operator delete(void *ptr){
      //operator delete
      
      TGRSIFit* fitptr = (static_cast<TGRSIFit*>(ptr));
      if(fitptr->AddToGlobalList(kFALSE))
         TF1::operator delete(ptr);
}

void* TGRSIFit::operator new[](size_t sz){
      //operator delete. You must use delete[] to free the array before program completion
      // Otherwise ROOT will try to delete the array using a normal delete call which is
      //bad news bears and will result in a segmentation violation.
      printf(DRED"It is DANGEROUS allocate dynamic arrays of TGRSIFits, use delete[] on the array before program completion, otherwise you will get a segfault.\n" RESET_COLOR);
      return TF1::operator new[](sz);
}

void* TGRSIFit::operator new[](size_t sz, void* vp){
      //operator delete. You must use delete[] to free the array before program completion
      // Otherwise ROOT will try to delete the array using a normal delete call which is
      //bad news bears and will result in a segmentation violation.
      printf(DRED"It is DANGEROUS allocate dynamic arrays of TGRSIFits, use delete[] on the array before program completion, otherwise you will get a segfault.\n" RESET_COLOR);
      return TF1::operator new[](sz,vp);
}


