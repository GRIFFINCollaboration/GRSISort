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



