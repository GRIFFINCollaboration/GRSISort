#include "TGRSIFunctions.h"

//Without this macro the THtml doc for TGRSIFunctions can't be generated
NamespaceImp(TGRSIFunctions);

//////////////////////////////////////////////////////////////////////
//
// TGRSIFunctions
//
// This namespace is where we store all of our commonly used functions.
// This makes it easier to create fits etc.
//
///////////////////////////////////////////////////////////////////////

Double_t TGRSIFunctions::PolyBg(Double_t *x, Double_t *par,Int_t order) {
//Polynomial function of the form SUM(par[i]*(x - shift)^i). The shift is done to match parameters with Radware output. 

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
//This function uses the same parameters as the photopeak and gaussian. This is because in the photopeak,
//the shapes are correlated.
//Requires the following parameters:
//   - dim[0]:  channels being fit
//   - par[0]:  height of photopeak
//   - par[1]:  centroid of gaussian
//   - par[2]:  standard deviation of gaussian 
//   - par[5]:  Size of the step in the step function

   Double_t x       = dim[0];
   Double_t height  = par[0];
   Double_t c       = par[1];
   Double_t sigma   = par[2];
   Double_t step    = par[5];

   return TMath::Abs(step)*height/100.0*TMath::Erfc((x-c)/(TMath::Sqrt(2.0)*sigma));
}

Double_t TGRSIFunctions::PhotoPeak(Double_t *dim, Double_t *par){
//Returns the combination of a TGRSIFunctions::Gaus + a TGRSIFunctions::SkewedGaus  
   return Gaus(dim,par) + SkewedGaus(dim,par);
}

Double_t TGRSIFunctions::Gaus(Double_t *dim, Double_t *par){
//This is a gaussian that has been scaled to match up with Radware photopeak results. 
//It contains a scaling factor for the relative height of the skewed gaussian to the 
//normal gaussian. Requires the following parameters:
//   - dim[0]:  channels being fit
//   - par[0]:  height of photopeak
//   - par[1]:  centroid of gaussian
//   - par[2]:  standard deviation of gaussian 
//   - par[4]:  relative height of skewed gaus to gaus   

   Double_t x        = dim[0]; //channel number used for fitting 
   Double_t height   = par[0]; //height of photopeak
   Double_t c        = par[1]; //Peak Centroid of non skew gaus
   Double_t sigma    = par[2]; //standard deviation of gaussian
   Double_t R        = par[4]; //relatice height of skewed gaussian

   return height*(1.0-R/100.0)*TMath::Gaus(x,c,sigma);

}

Double_t TGRSIFunctions::SkewedGaus(Double_t *dim, Double_t *par){
//This function uses the same parameters as the photopeak and gaussian. This is because in the photopeak, 
//the shapes are correlated.
//Requires the following parameters:
//   - dim[0]:  channels being fit
//   - par[0]:  height of photopeak
//   - par[1]:  centroid of gaussian
//   - par[2]:  standard deviation of gaussian 
//   - par[3]:  "skewedness" of the skewed gaussin
//   - par[4]:  relative height of skewed gaus to gaus 

   Double_t x        = dim[0]; //channel number used for fitting 
   Double_t height   = par[0]; //height of photopeak
   Double_t c        = par[1]; //Peak Centroid of non skew gaus
   Double_t sigma    = par[2]; //standard deviation  of gaussian
   Double_t beta     = par[3]; //"skewedness" of the skewed gaussian
   Double_t R        = par[4]; //relative height of skewed gaussian

   return R*height/100.0*(TMath::Exp((x-c)/beta))*(TMath::Erfc(((x-c)/(TMath::Sqrt(2.0)*sigma)) + sigma/(TMath::Sqrt(2.0)*beta)));

}


Double_t TGRSIFunctions::Bateman(Double_t *dim, Double_t *par, Int_t nChain, Double_t SecondsPerBin){
//****NOT TESTED****The Bateman equation is the general closed form for a decay chain of nuclei. This functions returns
//the total activity from a given chain of nuclei.
//Requires the following parameters:
//   - dim[0]:  channels being fit
//   - par[0+3*i]:  Initial Activity s^(-1)
//   - par[1+3*i]:  Decay Rate in seconds^(-1)
//   - par[2+3*i]:  Activity at time t s^(-1)
// NOTE: The lowest paramters correspond to the most 'senior' nuclei


  if(sizeof(par)/sizeof(par[0]) < (nChain*3)){
     std::cout << "not enough parameters passed to function" << std::endl;
     return 0;
  }

  Double_t totalActivity = 0.0;

//LOOP OVER ALL NUCLEI

   for(Int_t n=0; n<nChain;n++){
      //Calculate this equation for the nth nucleus.
      Double_t firstterm = 1.0;
      //Compute the first multiplication
      for(Int_t j=0; j<n-1; j++){
         firstterm*=par[1+3*j];
      }
      Double_t secondterm = 0.0;
      for(Int_t i=0; i<n; i++){
         Double_t sum = 0.0;
         for(Int_t j=i; j<n; j++){
            Double_t denom = 1.0;
            for(Int_t p=i;p<n;p++){
               if(p!=j){ denom*=par[1+3*p]-par[1+3*j]; } 
            }
            sum+=par[0+3*i]/par[1+3*i]*TMath::Exp(-par[1+3*j]*dim[0])/denom; 
          }
          secondterm += sum;
      }
      par[2+3*n] = par[1+3*n]*firstterm*secondterm;
      totalActivity += par[2+3*n];
   }
   return totalActivity;
}

Double_t TGRSIFunctions::DeadTimeCorrect(Double_t *dim, Double_t deadtime, Double_t binWidth){
//This function deadtime corrects data. Not to be confused with dead time affecting of fit functions
//Dead time is in us.
//binWidth is in s/bin.

   return dim[0]/(1.0 - dim[0]*deadtime/(binWidth*1000000.0));
} 

Double_t TGRSIFunctions::DeadTimeAffect(Double_t function, Double_t deadtime, Double_t binWidth){
//This function deadtime affects fitting functions. This is useful for counting the number of decays.
//Dead time is in us.
//binWidth is in s/bin.

   return function/( 1.0 + function*deadtime/(binWidth*1000000.0));

}
