#include "TGRSIFunctions.h"

//Without this macro the THtml doc for TGRSIFunctions can't be generated
NamespaceImp(TGRSIFunctions);

// Polynomial background function
Double_t TGRSIFunctions::PolyBg(Double_t *x, Double_t *par,Int_t order) {
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

Double_t TGRSIFunctions::StepFunction(Double_t *dim, Double_t *par){

   Double_t x       = dim[0];
   Double_t height  = par[0];
   Double_t c       = par[1];
   Double_t sigma   = par[2];
   Double_t step    = par[5];

   return TMath::Abs(step)*height/100.0*TMath::Erfc((x-c)/(TMath::Sqrt(2.0)*sigma));
}

Double_t TGRSIFunctions::PhotoPeak(Double_t *dim, Double_t *par){
   
   //Define the parameters for easy use.
   Double_t x        = dim[0]; //channel number used for fitting 
   Double_t height   = par[0]; //height of photopeak
   Double_t c        = par[1]; //Peak Centroid of non skew gaus
   Double_t sigma    = par[2]; //standard deviation  of gaussian
   Double_t beta     = par[3]; //"skewedness" of the skewed gaussian
   Double_t R        = par[4]; //relative height of skewed gaussian

   Double_t gaussian    = height*(1.0-R/100.0)*TMath::Gaus(x,c,sigma);
   Double_t skewed_gaus = R*height/100.0*(TMath::Exp((x-c)/beta))*(TMath::Erfc((x-c)/(TMath::Sqrt(2.0)*sigma)) + sigma/(TMath::Sqrt(2.0)*beta));

   return gaussian + skewed_gaus;
}
