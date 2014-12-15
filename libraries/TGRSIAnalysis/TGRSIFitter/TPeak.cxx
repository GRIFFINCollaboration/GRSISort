#include "TPeak.h"

ClassImp(TPeak)

TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh,  Option_t* type){

   Bool_t out_of_range_flag = false;

   if(cent > xhigh){
      printf("centroid is higher than range\n");
      out_of_range_flag = true;
   }
   else if (cent < xlow){
      printf("centroid is lower than range\n");
      out_of_range_flag = true;
   }

   //This fixes things if your user is like me and screws up a lot.
   if(out_of_range_flag){
      if (xlow > cent)
         std::swap(xlow, cent);
      if (xlow > xhigh)
         std::swap(xlow, xhigh);
      if (cent > xhigh)
         std::swap(cent, xhigh);
      printf("Something about your range was wrong. Assuming:\n");
      printf("centroid: %d \t range: %d to %d\n",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh));
   }

   //Set the fit function to be a radware style photo peak.
   this->fcentroid = cent;
   ffitfunc = new TF1("photopeak",TGRSIFunctions::PhotoPeak,xlow,xhigh,10);
   ffitbg   = new TF1("photopeakbg",TGRSIFunctions::PhotoPeakBG,xlow,xhigh,10);
   this->SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh)));

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




