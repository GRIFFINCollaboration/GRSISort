#include "TGRSIFunctions.h"

//Without this macro the THtml doc for TGRSIFunctions can't be generated
NamespaceImp(TGRSIFunctions);

// Polynomial background function
Double_t TGRSIFunctions::polybg(Double_t *x, Double_t *par,Int_t order) {
   if(sizeof(par)/sizeof(par[0]) < (order + 2)){
      std::cout << "not enough parameters passed to function" << std::endl;
      return 0;
   }   

   Double_t result = 0.0;
   for(Int_t i = 0; i<order; i++) {
      result += par[i]*TMath::Power(x[0]-par[order+1],i);
   }
   return result;
}

Double_t TGRSIFunctions::step_function(Double_t *dim, Double_t *par){

   Double_t x       = dim[0];
   Double_t height  = par[0];
   Double_t c       = par[1];
   Double_t sigma   = par[2];
   Double_t step    = par[5];

   return TMath::Abs(step)*height/100.0*TMath::Erfc((x-c)/(TMath::Sqrt(2.0)*sigma));
}


