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
      fBackgroundFunction->SetLineStyle(9);
   }
   return fBackgroundFunction;
}

void TSinglePeak::Print(Option_t *) const{

   std::cout << "Centroid = " << std::fixed << Centroid() << " +/- " << CentroidErr() << std::endl;
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

void TSinglePeak::DrawComponents(Option_t *opt){
   //This behaves like the draw function except each daughter class decides how to break the draw into multiple components.
   //This means that we should delegate this task to the daughted class.
}

Double_t TSinglePeak::PeakOnGlobalFunction(Double_t *dim, Double_t *par){
   if(!fGlobalBackground)
      return 0.0;

   return PeakFunction(dim,par) + fGlobalBackground->EvalPar(dim,&par[fTotalFunction->GetNpar()]);

}

void TSinglePeak::Draw(Option_t *opt){
   //We need to draw this on top of the global background. Probably easiest to make another temporary TF1?
   if(!fGlobalBackground)
      return;

   Double_t low, high;
   fGlobalBackground->GetRange(low,high);
   if(fPeakOnGlobal) fPeakOnGlobal->Delete();
   //Make a copy of the total function, and then tack on the global background parameters.
   fPeakOnGlobal = new TF1("draw_peak", this, &TSinglePeak::PeakOnGlobalFunction,low,high,fTotalFunction->GetNpar()+fGlobalBackground->GetNpar(),"TSinglePeak","PeakOnGlobalFunction");
   for(int i = 0; i < fTotalFunction->GetNpar(); ++i){
      fPeakOnGlobal->SetParameter(i,fTotalFunction->GetParameter(i));
   }
   for(int i = 0; i < fGlobalBackground->GetNpar(); ++i){
      fPeakOnGlobal->SetParameter(i+fTotalFunction->GetNpar(),fGlobalBackground->GetParameter(i));
   }
   //Draw a copy of this function
   fPeakOnGlobal->SetLineColor(fTotalFunction->GetLineColor());
   fPeakOnGlobal->SetLineStyle(fTotalFunction->GetLineStyle());
   fPeakOnGlobal->Draw(opt);
}

