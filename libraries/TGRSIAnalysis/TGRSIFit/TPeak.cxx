#include "TPeak.h"

ClassImp(TPeak)

TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh, TH1* fithist, Option_t* type) : TGRSIFit("photopeakbg",TGRSIFunctions::PhotoPeakBG,xlow,xhigh,10){ 

   this->Clear();
   Bool_t out_of_range_flag = false;
   if(!fithist) fithist = fHistogram;

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

   this->SetRange(xlow,xhigh);
      //We also need to make initial guesses at parameters
      //We need nice ways to access parameters etc.
      //Need to make a TMultipeak-like thing (does a helper class come into play then?)

   //Set the fit function to be a radware style photo peak.
   //This function might be unnecessary. Will revist this later. rd.
   this->SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh))); //Gives a default name to the peak

   //We need to set parameter names now.
   this->SetParName(0,"Height");
   this->SetParName(1,"centroid");
   this->SetParName(2,"sigma");
   this->SetParName(3,"beta");
   this->SetParName(4,"R");
   this->SetParName(5,"step");
   this->SetParName(6,"A");
   this->SetParName(7,"B");
   this->SetParName(8,"C");
   this->SetParName(9,"bg_offset");

   this->SetParameter("centroid",cent);

   if(fithist)
      this->InitParams(fithist);
}

TPeak::~TPeak(){
}
/*
TPeak::TPeak(const TPeak &copy) : TGRSIFit(copy){
   
   this->fcentroid    = copy.fcentroid;
   this->fd_centroid  = copy.fd_centroid;
   this->farea        = copy.farea;
   this->fd_area      = copy.fd_area;

   this->ffitfunc = new TF1(*(copy.ffitfunc));
   this->ffitbg   = new TF1(*(copy.ffitbg));
   this->ffithist = copy.ffithist;
}
*/

void TPeak::SetType(Option_t * type){
// This sets the style of gaussian fit function to use for the fitted peak. 
// Probably won't be used for much, but I'm leaving the option here for others
// This would be a good spot to change whether the functions is a Gaus or Landau etc.

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

Bool_t TPeak::InitParams(TH1 *fithist){
//Makes initial guesses at parameters for the fit. Uses the histogram to
   Double_t xlow,xhigh;
   GetRange(xlow,xhigh);
   Int_t bin = fithist->GetXaxis()->FindBin(GetParameter("centroid"));
   Int_t binlow = fithist->GetXaxis()->FindBin(xlow);
   Int_t binhigh = fithist->GetXaxis()->FindBin(xhigh);
   Double_t binWidth = fithist->GetBinWidth(bin);
   this->SetParLimits(1,xlow,xhigh);
   this->SetParLimits(2,0.5,12);
   this->SetParLimits(3,0.000,10);
   this->SetParLimits(4,0,500);
   this->SetParLimits(5,0,1000000);
   this->SetParLimits(6,0,GetParameter(6)*1.4);
   this->SetParLimits(9,xlow,xhigh);

   if(!fithist && fHistogram) 
      fithist = GetHistogram();

   if(!fithist){
      printf("No histogram is associated yet, no initial guesses made\n");
      return false;
   }
   //Make initial guesses
   //Actually set the parameters in the photopeak function
  //Fixing has to come after setting
  //Might have to include bin widths eventually
  //The centroid should already be set by this point in the ctor
   this->SetParameter("Height",fithist->GetBinContent(bin));
   this->SetParameter("centroid",GetParameter("centroid"));
   this->SetParameter("sigma",1.0);
   this->SetParameter("beta",0.5);
   this->SetParameter("R", 1.0);
   this->SetParameter("step",1.0);
   this->SetParameter("A",fithist->GetBinContent(binlow));
   this->SetParameter("B",(fithist->GetBinContent(binlow) - fithist->GetBinContent(binhigh))/(xlow-xhigh));
   this->SetParameter("C",-0.5);
   this->SetParameter("bg_offset",GetParameter("centroid"));
   this->FixParameter(8,0);
   SetInitialized();
   return true;
}

Bool_t TPeak::Fit(TH1* fithist,Option_t *opt){
   if(!fithist && fHistogram){
      fithist = fHistogram;
   }
   if(!fithist){
      printf("No histogram associated with Peak\n");
      return false;
   }
   if(!IsInitialized()) 
      InitParams(fithist);
   TVirtualFitter::SetMaxIterations(10000);

   //Now that it is initialized, let's fit it.
   TFitResultPtr fitres = fithist->Fit((TF1*)(this),Form("%sRS",opt));//The RS needs to always be there
   Double_t xlow,xhigh;
   this->GetRange(xlow,xhigh);
   //Make a function that does not include the background
   TF1 *tmppeak = new TF1(*((TF1*)(this)));
   tmppeak->SetName("tmppeak");
   tmppeak->SetParameter("step",0.0);
   tmppeak->SetParameter("A",0.0);
   tmppeak->SetParameter("B",0.0);
   tmppeak->SetParameter("C",0.0);
   
   //This is where we will do integrals and stuff.
   if(farea < 0.01){
      farea = tmppeak->Integral(xlow,xhigh);
      fd_area = tmppeak->IntegralError(xlow,xhigh,tmppeak->GetParameters(),fitres->GetCovarianceMatrix().GetMatrixArray());
   }
   delete tmppeak;
   
}

/*
Double_t TPeak::Fit(Option_t *opt){
//It returns the chi2 of the fit or a negative number for an error
//Errors: "-1": the TPeak* passed was empty
   if(!ffithist) {
      printf("No histogram set! Aborting...\n");
      return -1;
   }

   Bool_t verbosity = false;
   if(strchr(opt,'v') != NULL){
      verbosity = true;
   }

   if(!IsInitialized()) InitParams();
   

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
      ffithist = (TH1F*)hist;
   }
}

Bool_t TPeak::SetHist(const char* histname){
   TH1 *hist = 0;
   gROOT->GetObject(histname,hist);
   if (hist) {
      return SetHist(hist);
   }
}*/

void TPeak::Clear(){
//Clear the TPeak including functions and histogram, does not
//currently clear inherited members such as name.
//want to make a clear that doesn't clear everything
   farea         = 0.0;
   fd_area       = 0.0;
   TGRSIFit::Clear();
   //Do deep clean stuff maybe? require an option?

}

void TPeak::Print(Option_t *opt) const{
//Prints TPeak properties. To see More properties use the option "+"
   printf("Name:        %s \n", this->GetName()); 
   printf("Centroid:    %lf +/- %lf \n", this->GetParameter("centroid"),this->GetParError(GetParNumber("centroid")));
   printf("Area: 	      %lf +/- %lf \n", farea, fd_area);
   if(strchr(opt,'+') != NULL){
      TF1::Print();
      TGRSIFit::Print(opt); //Polymorphise this a bit better
   }
}




