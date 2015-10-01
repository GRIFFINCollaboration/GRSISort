#include "TPeak.h"
#include "TGraph.h"

ClassImp(TPeak)

Bool_t TPeak::fLogLikelihoodFlag = true;

//We need c++ 11 for constructor delegation....
TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh, TF1* background) : TGRSIFit("photopeakbg",TGRSIFunctions::PhotoPeakBG,xlow,xhigh,10){
   fResiduals = 0;
   fBackground = 0;
   this->Clear();
   fOwnBgFlag = false;
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

   this->SetRange(xlow,xhigh);
      //We also need to make initial guesses at parameters
      //We need nice ways to access parameters etc.
      //Need to make a TMultipeak-like thing (does a helper class come into play then?)

   //Set the fit function to be a radware style photo peak.
   //This function might be unnecessary. Will revist this later. rd.
   this->SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh))); //Gives a default name to the peak

   //We need to set parameter names now.
   this->InitNames();
   this->SetParameter("centroid",cent);

   //Check to see if background is good.
   if(background){
      fBackground = background;
      fOwnBgFlag = false;
   }
   else{
      printf("Bad background pointer. Creating basic background.\n");
      this->fBackground = new TF1(Form("background%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh)),TGRSIFunctions::StepBG,xlow,xhigh,10);
      TGRSIFit::AddToGlobalList(fBackground,kFALSE);
      fOwnBgFlag = true;
   }
      
   this->fBackground->SetNpx(1000);
   this->fBackground->SetLineStyle(2);
   this->fBackground->SetLineColor(kBlack);

   this->fResiduals = new TGraph;

}

//This makes a temporary TF1 I think, but I'm not sure an easier (that is nice) way to do it
TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh) : TGRSIFit("photopeakbg",TGRSIFunctions::PhotoPeakBG,xlow,xhigh,10){ 
   fResiduals = 0;
   fBackground = 0;
   this->Clear();
   fOwnBgFlag = true;
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

   this->SetRange(xlow,xhigh);
      //We also need to make initial guesses at parameters
      //We need nice ways to access parameters etc.
      //Need to make a TMultipeak-like thing (does a helper class come into play then?)

   //Set the fit function to be a radware style photo peak.
   //This function might be unnecessary. Will revist this later. rd.
   this->SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh))); //Gives a default name to the peak

   //We need to set parameter names now.
   this->InitNames();
   this->SetParameter("centroid",cent);

   this->fBackground = new TF1(Form("background%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh)),TGRSIFunctions::StepBG,xlow,xhigh,10);
      
   this->fBackground->SetNpx(1000);
   this->fBackground->SetLineStyle(2);
   this->fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground,kFALSE);

   this->fResiduals = new TGraph;

}

TPeak::TPeak() : TGRSIFit("photopeakbg",TGRSIFunctions::PhotoPeakBG,0,1000,10){
   fResiduals = 0;
   fBackground = 0;
   this->InitNames();
   fOwnBgFlag = true;
   fBackground = new TF1("background",TGRSIFunctions::StepBG,0,1000,10);
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground,kFALSE);

   fResiduals = new TGraph;
}

TPeak::~TPeak(){
   if(fBackground && fOwnBgFlag){
      delete fBackground;
   }
   if(fResiduals) delete fResiduals;
}

TPeak::TPeak(const TPeak &copy) : fBackground(0), fResiduals(0){
   fBackground = 0;
   fResiduals = 0;
   ((TPeak&)copy).Copy(*this);
}

void TPeak::InitNames(){
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
}

void TPeak::Copy(TObject &obj) const {
   TGRSIFit::Copy(obj);

   if(!((TPeak&)obj).fBackground){
      ((TPeak&)obj).fBackground = new TF1(*fBackground);
   }
   if(!((TPeak&)obj).fResiduals)
      ((TPeak&)obj).fResiduals = new TGraph(*fResiduals);

   ((TPeak&)obj).farea = farea;
   ((TPeak&)obj).fd_area = fd_area;

   ((TPeak&)obj).fchi2 = fchi2;
   ((TPeak&)obj).fNdf  = fNdf;

   *(((TPeak&)obj).fBackground) = *fBackground;
   //We are making a direct copy of the background so the ownership is that of the copy
   ((TPeak&)obj).fOwnBgFlag = true;
   *(((TPeak&)obj).fResiduals) = *fResiduals;

   ((TPeak&)obj).SetHist(GetHist());

}

Bool_t TPeak::InitParams(TH1 *fithist){
//Makes initial guesses at parameters for the fit. Uses the histogram to
	if(fithist == NULL) {
		return false;
	}
   Double_t xlow,xhigh;
   GetRange(xlow,xhigh);
   Int_t bin = fithist->GetBinCenter(GetParameter("centroid"));
   Int_t binlow = fithist->GetXaxis()->FindBin(xlow);
   Int_t binhigh = fithist->GetXaxis()->FindBin(xhigh);
   Double_t binWidth = fithist->GetBinWidth(bin);
   this->SetParLimits(1,xlow,xhigh);
   this->SetParLimits(2,0.1,(xhigh-xlow)); // sigma should be less than the window width - JKS
   this->SetParLimits(3,0.000001,10);
   this->SetParLimits(4,0.000001,100); // this is a percentage. no reason for it to go to 500% - JKS
   //Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
   this->SetParLimits(6,0.0,fithist->GetBinContent(bin)*100.);
   this->SetParLimits(9,xlow,xhigh);
   this->SetParLimits(5,0.0,1.0E2);

   if(!fithist && GetHist()) 
      fithist = GetHist();

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
 //  this->SetParameter("sigma",(xhigh-xlow)*0.5); // slightly more robust starting value for sigma -JKS
 //  this->SetParameter("sigma",1.0/binWidth); // slightly more robust starting value for sigma -JKS
   this->SetParameter("sigma",TMath::Sqrt(9.0 + 4.*GetParameter("centroid")/1000.));
   this->SetParameter("beta",GetParameter("sigma")/2.0);
   this->SetParameter("R", 1.0);
   this->SetParameter("step",1.0);
   this->SetParameter("A",fithist->GetBinContent(binhigh));
   this->SetParameter("B",(fithist->GetBinContent(binlow) - fithist->GetBinContent(binhigh))/(xlow-xhigh));
   this->SetParameter("C",0.0000);
   this->SetParameter("bg_offset",GetParameter("centroid"));
//   this->FixParameter(8,0.00);
   this->FixParameter(3,GetParameter("beta"));
   this->FixParameter(4,0.00);
   SetInitialized();
   return true;
}

Bool_t TPeak::Fit(TH1* fithist,Option_t *opt){
   TString optstr = opt;
   if(!fithist && !GetHist()){
      printf("No hist passed, trying something...");
      fithist = fHistogram;
   }
   if(!fithist){
      printf("No histogram associated with Peak\n");
      return false;
   }
   if(!IsInitialized()) 
      InitParams(fithist);
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Combination");
   TVirtualFitter::SetMaxIterations(100000);
   TVirtualFitter::SetPrecision(1e-5);

   SetHist(fithist);

   TString options(opt); bool print_flag = true;
   if(options.Contains("Q"))
     print_flag = false;

   //Now that it is initialized, let's fit it.
   //Just in case the range changed, we should reset the centroid and bg energy limits
	//check first if the parameter has been fixed!
	double parmin, parmax;
	this->GetParLimits(1,parmin,parmax);
	if(parmin < parmax) {
		this->SetParLimits(1,GetXmin(),GetXmax());
	}
	this->GetParLimits(9,parmin,parmax);
	if(parmin < parmax) {
		this->SetParLimits(9,GetXmin(),GetXmax());
	}

   TFitResultPtr fitres;
   //Log likelihood is the proper fitting technique UNLESS the data is a result of an addition or subtraction.
   if(GetLogLikelihoodFlag()){
      fitres = fithist->Fit(this,Form("%sRLS",opt));//The RS needs to always be there
   }
   else{
      fitres = fithist->Fit(this,Form("%sRS",opt));//The RS needs to always be there
   }

   //After performing this fit I want to put something here that takes the fit result (good,bad,etc)
   //for printing out. RD

   Int_t fitStatus = fitres; //This returns a fit status from the TFitResult Ptr

   if(fitres->ParError(2) != fitres->ParError(2)){ //Check to see if nan
      if(fitres->Parameter(3) < 1){
         InitParams(fithist);
         FixParameter(4,0);
         FixParameter(3,1);
         std::cout << "Beta may have broken the fit, retrying with R=0" << std::endl;
   	 // Leaving the log-likelihood argument out so users are not constrained to just using that. - JKS
         fithist->GetListOfFunctions()->Last()->Delete();
         if(GetLogLikelihoodFlag()){
            fitres = fithist->Fit(this,Form("%sRLS",opt));//The RS needs to always be there
         }
         else{
            fitres = fithist->Fit(this,Form("%sRS",opt));
         }
      }
   }
/*   if(fitres->Parameter(5) < 0.0){
      FixParameter(5,0);
      std::cout << "Step < 0. Retrying fit with stp = 0" << std::endl;
      fitres = fithist->Fit(this,Form("%sRSML",opt));
   }
*/
   Double_t binWidth = fithist->GetBinWidth(GetParameter("centroid"));
   Double_t width = this->GetParameter("sigma");
   if(print_flag) printf("Chi^2/NDF = %lf\n",fitres->Chi2()/fitres->Ndf());
   fchi2 = fitres->Chi2();  fNdf = fitres->Ndf();
   Double_t xlow,xhigh;
   Double_t int_low, int_high; 
   this->GetRange(xlow,xhigh);
   int_low = xlow - 5.*width; // making the integration bounds a bit smaller, but still large enough. -JKS
   int_high = xhigh + 5.*width;

   //Make a function that does not include the background
   //Intgrate the background.
   //TPeak *tmppeak = new TPeak(*this);
   TPeak *tmppeak = new TPeak;
   this->Copy(*tmppeak);
   tmppeak->SetParameter("step",0.0);
   tmppeak->SetParameter("A",0.0);
   tmppeak->SetParameter("B",0.0);
   tmppeak->SetParameter("C",0.0);
   tmppeak->SetParameter("bg_offset",0.0);
   tmppeak->SetRange(int_low,int_high);//This will help get the true area of the gaussian 200 ~ infinity in a gaus
   tmppeak->SetName("tmppeak");
   
   //SOMETHING IS WRONG WITH THESE UNCERTAINTIES
   //This is where we will do integrals and stuff.
   farea = (tmppeak->Integral(int_low,int_high))/binWidth;
   //Set the background values in the covariance matrix to 0, while keeping their covariance errors
   TMatrixDSym CovMat = fitres->GetCovarianceMatrix();
   CovMat(5,5) = 0.0;
   CovMat(6,6) = 0.0;
   CovMat(7,7) = 0.0;
   CovMat(8,8) = 0.0;
   CovMat(9,9) = 0.0;
   fd_area = (tmppeak->IntegralError(int_low,int_high,tmppeak->GetParameters(),CovMat.GetMatrixArray())) /binWidth;

   if(print_flag) printf("Integral: %lf +/- %lf\n",farea,fd_area);
   //Set the background for drawing later
   fBackground->SetParameters(this->GetParameters());
   //To DO: put a flag in signalling that the errors are not to be trusted if we have a bad cov matrix
   Copy(*fithist->GetListOfFunctions()->Last());
 //  if(optstr.Contains("+"))
  //    Copy(*fithist->GetListOfFunctions()->Before(fithist->GetListOfFunctions()->Last()));
   
   delete tmppeak;
   return true;
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

void TPeak::Clear(Option_t *opt){
//Clear the TPeak including functions and histogram, does not
//currently clear inherited members such as name.
//want to make a clear that doesn't clear everything
   farea         = 0.0;
   fd_area       = 0.0;
   fchi2         = 0.0;
   fNdf          = 0.0;
   TGRSIFit::Clear();
   //Do deep clean stuff maybe? require an option?

}

void TPeak::Print(Option_t *opt) const {
//Prints TPeak properties. To see More properties use the option "+"
   printf("Name:        %s \n", this->GetName()); 
   printf("Centroid:    %lf +/- %lf \n", this->GetParameter("centroid"),this->GetParError(GetParNumber("centroid")));
   printf("Area: 	      %lf +/- %lf \n", farea, fd_area);
   printf("FWHM:        %lf +/- %lf \n", this->GetParameter("sigma")*2.3548, this->GetParError(GetParNumber("sigma"))*2.3548);
   printf("Chi^2/NDF:   %lf\n",fchi2/fNdf);
   if(strchr(opt,'+') != NULL){
      TF1::Print();
      TGRSIFit::Print(opt); //Polymorphise this a bit better
   }
}

const char * TPeak::PrintString(Option_t *opt) const {
//Prints TPeak properties to a string, returns the string.
   std::string temp;
   temp.assign("Name:        ");temp.append(this->GetName()); temp.append("\n");
   temp.append("Centroid:    ");temp.append(Form("%lf",this->GetParameter("centroid")));
                                temp.append(" +/- ");
                                temp.append(Form("%lf",this->GetParError(GetParNumber("centroid")))); temp.append("\n");
   temp.append("Area: 	     ");temp.append(Form("%lf",farea)); 
                                temp.append(" +/- ");
                                temp.append(Form("%lf",fd_area));    temp.append("\n"); 
   temp.append("Chi^2/NDF:   ");temp.append(Form("%lf",fchi2/fNdf)); temp.append("\n");
   //if(strchr(opt,'+') != NULL){
   //   TF1::Print();
   TGRSIFit::Print(opt); //Polymorphise this a bit better
   //}
   return temp.c_str();
}

void TPeak::DrawBackground(Option_t *opt) const{
   fBackground->Draw(opt);
}

void TPeak::DrawResiduals() {
   if(!GetHist()){
      printf("No hist set\n");
      return;
   }
   if(fchi2<0.000000001){
      printf("No fit performed\n");
      return;
   }

   Double_t xlow,xhigh;
   GetRange(xlow,xhigh);
   Int_t nbins = GetHist()->GetXaxis()->GetNbins();
   Double_t *res = new Double_t[nbins];
   Double_t *bin = new Double_t[nbins];
   Int_t points = 0;
   fResiduals->Clear();

   for(int i =1;i<=nbins;i++) {
      if(GetHist()->GetBinCenter(i) <= xlow || GetHist()->GetBinCenter(i) >= xhigh)
         continue;
      res[points] = (GetHist()->GetBinContent(i) - this->Eval(GetHist()->GetBinCenter(i)))+ this->GetParameter("Height")/2;///GetHist()->GetBinError(i));// + this->GetParameter("Height") + 10.;
      bin[points] = GetHist()->GetBinCenter(i);
      fResiduals->SetPoint(i,bin[i],res[i]);

      points++;
   }


   fResiduals->Draw();

   delete[] res;
   delete[] bin;

}


