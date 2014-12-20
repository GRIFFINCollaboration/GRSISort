#include "TPeak.h"

ClassImp(TPeak)

TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh,  Option_t* type){

   this->Clear();
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

      //We also need to make initial guesses at parameters
      //We need nice ways to access parameters etc.
      //Need to make a TMultipeak-like thing (does a helper class come into play then?)

   //Set the fit function to be a radware style photo peak.
   this->fcentroid = cent;
   //This function might be unnecessary. Will revist this later. rd.
   ffitfunc = new TF1("photopeak",TGRSIFunctions::PhotoPeak,xlow,xhigh,10); //This is just the photopeak
   ffitbg   = new TF1("photopeakbg",TGRSIFunctions::PhotoPeakBG,xlow,xhigh,10); //This is the photopeak +BG
   this->SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh))); //Gives a default name to the peak

   ffithist = 0; //This will be used later to determine if a histogram was set for the peak.
   ffitres  = 0;
   //We need to set parameter names now.
   ffitbg->SetParName(0,"Height");
   ffitbg->SetParName(1,"centroid");
   ffitbg->SetParName(2,"sigma");
   ffitbg->SetParName(3,"beta");
   ffitbg->SetParName(4,"R");
   ffitbg->SetParName(5,"step");
   ffitbg->SetParName(6,"A");
   ffitbg->SetParName(7,"B");
   ffitbg->SetParName(8,"C");
   ffitbg->SetParName(9,"bg offset");
}

void TPeak::SetFitResult(TFitResultPtr fitres){ 
   ffitres = fitres; 
   //I should also set the TF1* members here
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

Double_t TPeak::Fit(Option_t *opt){
//It returns the chi2 of the fit or a negative number for an error
//Errors: "-1": the TPeak* passed was empty
   Bool_t verbosity = false;
   if(strchr(opt,'v') != NULL){
      verbosity = true;
   }

   //Now we do the fitting!
   
   return 0;
}

Double_t TPeak::Fit(TH1 *hist, Option_t *opt){
//Allows you to set the hist at the time of fitting.
   SetHist(hist);
   return Fit(opt);
}

Double_t TPeak::Fit(const char* histname, Option_t *opt){
//Allows you to fit the hist by name in case of variable overwriting
   SetHist(histname);
   return Fit(opt);
}

Bool_t TPeak::SetHist(TH1* hist){
//Set this histogram that the TPeak will be fitted to
   if(!hist){
      //Return the current pad's historgram 
      //need to put this here when I get around to it. rd
      printf("No hist is set\n");
   }
   else{
      ffithist = hist;
   }

}

Bool_t TPeak::SetHist(const char* histname){
   TH1 *hist = 0;
   gROOT->GetObject(histname,hist);
   if (hist) {
      return SetHist(hist);
   }
}

void TPeak::Clear(){
//Clear the TPeak including functions and histogram, does not
//currently clear inherited members such as name.
   fcentroid     = 0.0;
   fd_centroid   = 0.0;
   farea         = 0.0;
   fd_area       = 0.0;
   ffitfunc      = 0;
   ffitbg        = 0;
   ffithist      = 0;
   ffitres       = 0;
}

void TPeak::Print(Option_t *opt) const{
//Prints TPeak properties. To see More properties use the option "+"
   printf("Name:        %s \n", this->GetName()); 
   printf("Centroid:    %lf +/- %lf \n", fcentroid,fd_centroid);
   printf("Area: 	      %lf +/- %lf \n", farea, fd_area);
   if(strchr(opt,'+') != NULL){
      if(ffithist) printf("Histogram:   %s \n", ffithist->GetName()); 
      if(ffitbg) printf("Fit Function: "); ffitbg->Print(); //Rewrite this to print errors
   }
}




