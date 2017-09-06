#include "TSinglePeak.h"

/// \cond CLASSIMP
ClassImp(TSinglePeak)
/// \endcond

TSinglePeak::TSinglePeak() : TObject(){
}

bool TSinglePeak::IsBackgroundParameter(const Int_t& par) const{
   try{
      return fListOfBGPars.at(par);
   }
   catch(const std::out_of_range& oor){
      std::cerr << "Parameter not in list: " << par << std::endl;
      return true;
   }
   return true; //never gets here...appeals to some compilers.
}

bool TSinglePeak::IsPeakParameter(const Int_t& par) const{
   return !IsBackgroundParameter(par);
}

Int_t TSinglePeak::GetNParameters() const{
   if(fTotalFunction)
      return fTotalFunction->GetNpar();
   else
      return 0;
}

TF1* TSinglePeak::GetBackgroundFunction(){
   if(!fBackgroundFunction){
      fBackgroundFunction = new TF1("peak_bg", this, &TSinglePeak::BackgroundFunction,0,1, fTotalFunction->GetNpar(), "TSinglePeak", "BackgroundFunction");
   }
   return fBackgroundFunction;
}

void TSinglePeak::Print(Option_t *opt) const{

   std::cout << "Centroid = " << Centroid() << " +/- " << CentroidErr() << std::endl;
   std::cout << "Area = " << Area() << " +/- " << AreaErr() << std::endl;
/*   std::cout << "BG params = ";
   for(int i = 0; i < 6; ++i){
      if(IsBackgroundParameter(i)){
         std::cout << fFitFunction->GetParName(i) << ", ";
      }
   }*/
   std::cout << std::endl;

}

Double_t TSinglePeak::TotalFunction(Double_t *dim, Double_t *par){
   return PeakFunction(dim,par) + BackgroundFunction(dim,par);
}

void TSinglePeak::UpdateBackgroundParameters(){
   fBackgroundFunction->SetParameters(fTotalFunction->GetParameters());
}

