#include "TPeak.h"

ClassImp(TPeak)

TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh,  Option_t* type) : fcentroid(cent){

   ffitfunc = new TF1("photopeak",TGRSIFunctions::PhotoPeak,xlow,xhigh,10);


}

void TPeak::SetType(Option_t * type){
// This sets the style of gaussian fit function to use for the fitted peak. 
// Probably won't be used for much, but I'm leaving the option here for others

   //Not using this right now, just starting with all of these components added in
   if(strchr(type,'g') != NULL){
      //Gaussian     
   }
   if(strchr(type,'s') != NULL){
      //skewed gaussian
   }
   if(strchr(type,'c') != NULL){
      //include a step function to the background
   }

//   fpeakfit = new TF1("photopeak","gauss",fxlow,fxhigh);  


}

void TPeak::Clear(){
   fcentroid     = 0.0;
   fd_centroid   = 0.0;
   farea         = 0.0;
   fd_area       = 0.0;
}

void TPeak::Print(){
   printf("Name:        %s \n", this->GetName()); 
   printf("Centroid:    %lf +/- %lf \n", fcentroid,fd_centroid);
   printf("Area: 	      %lf +/- %lf \n", farea, fd_area);
}




