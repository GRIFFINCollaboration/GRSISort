#include "TPeak.h"
#include "TGraph.h"

#include "Math/Minimizer.h"

/// \cond CLASSIMP
ClassImp(TPeak)
/// \endcond

Bool_t TPeak::fLogLikelihoodFlag = true;

//We need c++ 11 for constructor delegation....
TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh, TF1* background) : TGRSIFit("photopeakbg",TGRSIFunctions::PhotoPeakBG,xlow,xhigh,10) {
   fResiduals = 0;
   fBackground = 0;
   Clear();
   fOwnBgFlag = false;
   Bool_t outOfRangeFlag = false;

   if(cent > xhigh) {
      printf("centroid is higher than range\n");
      outOfRangeFlag = true;
   } else if (cent < xlow) {
      printf("centroid is lower than range\n");
      outOfRangeFlag = true;
   }

   //This fixes things if your user is like me and screws up a lot.
   if(outOfRangeFlag) {
      if (xlow > cent)
         std::swap(xlow, cent);
      if (xlow > xhigh)
         std::swap(xlow, xhigh);
      if (cent > xhigh)
         std::swap(cent, xhigh);
      printf("Something about your range was wrong. Assuming:\n");
      printf("centroid: %d \t range: %d to %d\n",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh));
   }

   SetRange(xlow,xhigh);
	//We also need to make initial guesses at parameters
	//We need nice ways to access parameters etc.
	//Need to make a TMultipeak-like thing (does a helper class come into play then?)

   //Set the fit function to be a radware style photo peak.
   //This function might be unnecessary. Will revist this later. rd.
   SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh))); //Gives a default name to the peak

   //We need to set parameter names now.
   InitNames();
   SetParameter("centroid",cent);

   //Check to see if background is good.
   if(background) {
      fBackground = background;
      fOwnBgFlag = false;
   } else{
      printf("Bad background pointer. Creating basic background.\n");
      fBackground = new TF1(Form("background%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh)),TGRSIFunctions::StepBG,xlow,xhigh,10);
      TGRSIFit::AddToGlobalList(fBackground,kFALSE);
      fOwnBgFlag = true;
   }
      
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);

   fResiduals = new TGraph;
}

//This makes a temporary TF1 I think, but I'm not sure an easier (that is nice) way to do it
TPeak::TPeak(Double_t cent, Double_t xlow, Double_t xhigh) : TGRSIFit("photopeakbg",TGRSIFunctions::PhotoPeakBG,xlow,xhigh,10) { 
   fResiduals = 0;
   fBackground = 0;
   Clear();
   fOwnBgFlag = true;
   Bool_t outOfRangeFlag = false;

   if(cent > xhigh) {
      printf("centroid is higher than range\n");
      outOfRangeFlag = true;
   } else if (cent < xlow) {
      printf("centroid is lower than range\n");
      outOfRangeFlag = true;
   }

   //This fixes things if your user is like me and screws up a lot.
   if(outOfRangeFlag) {
      if (xlow > cent)
         std::swap(xlow, cent);
      if (xlow > xhigh)
         std::swap(xlow, xhigh);
      if (cent > xhigh)
         std::swap(cent, xhigh);
      printf("Something about your range was wrong. Assuming:\n");
      printf("centroid: %d \t range: %d to %d\n",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh));
   }

   SetRange(xlow,xhigh);
	//We also need to make initial guesses at parameters
	//We need nice ways to access parameters etc.
	//Need to make a TMultipeak-like thing (does a helper class come into play then?)

   //Set the fit function to be a radware style photo peak.
   //This function might be unnecessary. Will revist this later. rd.
   SetName(Form("Chan%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh))); //Gives a default name to the peak

   //We need to set parameter names now.
   InitNames();
   SetParameter("centroid",cent);

   fBackground = new TF1(Form("background%d_%d_to_%d",(Int_t)(cent),(Int_t)(xlow),(Int_t)(xhigh)),TGRSIFunctions::StepBG,xlow,xhigh,10);
      
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground,kFALSE);

   fResiduals = new TGraph;
}

TPeak::TPeak() : TGRSIFit("photopeakbg",TGRSIFunctions::PhotoPeakBG,0,1000,10) {
   fResiduals = 0;
   fBackground = 0;
   InitNames();
   fOwnBgFlag = true;
   fBackground = new TF1("background",TGRSIFunctions::StepBG,0,1000,10);
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground,kFALSE);

   fResiduals = new TGraph;
}

TPeak::~TPeak() {
   if(fBackground && fOwnBgFlag) {
      delete fBackground;
   }
   if(fResiduals) delete fResiduals;
}

TPeak::TPeak(const TPeak &copy) : TGRSIFit(), fBackground(0), fResiduals(0) {
   fBackground = 0;
   fResiduals = 0;
   copy.Copy(*this);
}

void TPeak::InitNames() {
   SetParName(0,"Height");
   SetParName(1,"centroid");
   SetParName(2,"sigma");
   SetParName(3,"beta");
   SetParName(4,"R");
   SetParName(5,"step");
   SetParName(6,"A");
   SetParName(7,"B");
   SetParName(8,"C");
   SetParName(9,"bg_offset");
}

void TPeak::Copy(TObject &obj) const {
   TGRSIFit::Copy(obj);

   if(!static_cast<TPeak&>(obj).fBackground) {
      static_cast<TPeak&>(obj).fBackground = new TF1(*fBackground);
   }
   if(!static_cast<TPeak&>(obj).fResiduals)
      static_cast<TPeak&>(obj).fResiduals = new TGraph(*fResiduals);

   static_cast<TPeak&>(obj).fArea = fArea;
   static_cast<TPeak&>(obj).fDArea = fDArea;

   static_cast<TPeak&>(obj).fChi2 = fChi2;
   static_cast<TPeak&>(obj).fNdf  = fNdf;

   *(static_cast<TPeak&>(obj).fBackground) = *fBackground;
   //We are making a direct copy of the background so the ownership is that of the copy
   static_cast<TPeak&>(obj).fOwnBgFlag = true;
   *(static_cast<TPeak&>(obj).fResiduals) = *fResiduals;

   static_cast<TPeak&>(obj).SetHist(GetHist());

}

Bool_t TPeak::InitParams(TH1* fitHist) {
//Makes initial guesses at parameters for the fit. Uses the histogram to
	if(fitHist == NULL) {
		return false;
	}
   Double_t xlow,xhigh;
   GetRange(xlow,xhigh);
   Int_t bin = fitHist->GetBinCenter(GetParameter("centroid"));
   Int_t binlow = fitHist->GetXaxis()->FindBin(xlow);
   Int_t binhigh = fitHist->GetXaxis()->FindBin(xhigh);
   //Double_t binWidth = fitHist->GetBinWidth(bin);
   SetParLimits(0,0,fitHist->GetMaximum());
   SetParLimits(1,xlow,xhigh);
   SetParLimits(2,0.1,(xhigh-xlow)); // sigma should be less than the window width - JKS
   SetParLimits(3,0.000001,10);
   SetParLimits(4,0.000001,100); // this is a percentage. no reason for it to go to 500% - JKS
   //Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0
   SetParLimits(5,0.0,1.0E2);
   SetParLimits(6,0.0,fitHist->GetBinContent(bin)*100.);
   SetParLimits(9,xlow,xhigh);

   if(!fitHist && GetHist()) 
      fitHist = GetHist();

   if(!fitHist) {
      printf("No histogram is associated yet, no initial guesses made\n");
      return false;
   }
   //Make initial guesses
   //Actually set the parameters in the photopeak function
  //Fixing has to come after setting
  //Might have to include bin widths eventually
  //The centroid should already be set by this point in the ctor
   SetParameter("Height",fitHist->GetBinContent(bin));
   SetParameter("centroid",GetParameter("centroid"));
 //  SetParameter("sigma",(xhigh-xlow)*0.5); // slightly more robust starting value for sigma -JKS
 //  SetParameter("sigma",1.0/binWidth); // slightly more robust starting value for sigma -JKS
   SetParameter("sigma",TMath::Sqrt(9.0 + 4.*GetParameter("centroid")/1000.));
   SetParameter("beta",GetParameter("sigma")/2.0);
   SetParameter("R", 1.0);
   SetParameter("step",1.0);
   SetParameter("A",fitHist->GetBinContent(binhigh));
   SetParameter("B",(fitHist->GetBinContent(binlow) - fitHist->GetBinContent(binhigh))/(xlow-xhigh));
   SetParameter("C",0.0000);
   SetParameter("bg_offset",GetParameter("centroid"));
 //  FixParameter(8,0.00);
   FixParameter(3,GetParameter("beta"));
   FixParameter(4,0.00);
   SetInitialized();
   return true;
}

Bool_t TPeak::Fit(TH1* fitHist,Option_t* opt) {
   TString optstr = opt;
   if(!fitHist && !GetHist()) {
      printf("No hist passed, trying something...");
      fitHist = fHistogram;
   }
   if(!fitHist) {
      printf("No histogram associated with Peak\n");
      return false;
   }
   if(!IsInitialized()) 
      InitParams(fitHist);
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Combination");
   TVirtualFitter::SetMaxIterations(100000);
   TVirtualFitter::SetPrecision(1e-5);

   SetHist(fitHist);

   TString options(opt); bool print_flag = true;
   if(options.Contains("Q"))
     print_flag = false;

   //Now that it is initialized, let's fit it.
   //Just in case the range changed, we should reset the centroid and bg energy limits
	//check first if the parameter has been fixed!
	double parmin, parmax;
	GetParLimits(1,parmin,parmax);
	if(parmin < parmax) {
		SetParLimits(1,GetXmin(),GetXmax());
	}
	GetParLimits(9,parmin,parmax);
	if(parmin < parmax) {
		SetParLimits(9,GetXmin(),GetXmax());
	}

   TFitResultPtr fitres;
   //Log likelihood is the proper fitting technique UNLESS the data is a result of an addition or subtraction.
   if(GetLogLikelihoodFlag()) {
      fitres = fitHist->Fit(this,Form("%sRLS",opt));//The RS needs to always be there
   } else{
      fitres = fitHist->Fit(this,Form("%sRS",opt));//The RS needs to always be there
   }

   //After performing this fit I want to put something here that takes the fit result (good,bad,etc)
   //for printing out. RD

   //Int_t fitStatus = fitres; //This returns a fit status from the TFitResult Ptr

   if(fitres->ParError(2) != fitres->ParError(2)) { //Check to see if nan
      if(fitres->Parameter(3) < 1) {
         InitParams(fitHist);
         FixParameter(4,0);
         FixParameter(3,1);
         std::cout << "Beta may have broken the fit, retrying with R=0" << std::endl;
   	 // Leaving the log-likelihood argument out so users are not constrained to just using that. - JKS
         fitHist->GetListOfFunctions()->Last()->Delete();
         if(GetLogLikelihoodFlag()) {
            fitres = fitHist->Fit(this,Form("%sRLS",opt));//The RS needs to always be there
         } else{
            fitres = fitHist->Fit(this,Form("%sRS",opt));
         }
      }
   }
/*   if(fitres->Parameter(5) < 0.0) {
      FixParameter(5,0);
      std::cout << "Step < 0. Retrying fit with stp = 0" << std::endl;
      fitres = fitHist->Fit(this,Form("%sRSML",opt));
   }
*/
   Double_t binWidth = fitHist->GetBinWidth(GetParameter("centroid"));
   Double_t width = GetParameter("sigma");
   if(print_flag) printf("Chi^2/NDF = %lf\n",fitres->Chi2()/fitres->Ndf());
   fChi2 = fitres->Chi2();  fNdf = fitres->Ndf();
   Double_t xlow,xhigh;
   Double_t int_low, int_high; 
   GetRange(xlow,xhigh);
   int_low = xlow - 10.*width; // making the integration bounds a bit smaller, but still large enough. -JKS
   int_high = xhigh + 10.*width;

   //Make a function that does not include the background
   //Intgrate the background.
   //TPeak* tmppeak = new TPeak(*this);
   TPeak* tmppeak = new TPeak;
   Copy(*tmppeak);
   tmppeak->SetParameter("step",0.0);
   tmppeak->SetParameter("A",0.0);
   tmppeak->SetParameter("B",0.0);
   tmppeak->SetParameter("C",0.0);
   tmppeak->SetParameter("bg_offset",0.0);
   tmppeak->SetRange(int_low,int_high);//This will help get the true area of the gaussian 200 ~ infinity in a gaus
   tmppeak->SetName("tmppeak");
   
   //SOMETHING IS WRONG WITH THESE UNCERTAINTIES
   //This is where we will do integrals and stuff.
   fArea = (tmppeak->Integral(int_low,int_high))/binWidth;
   //Set the background values in the covariance matrix to 0, while keeping their covariance errors
   TMatrixDSym CovMat = fitres->GetCovarianceMatrix();
   CovMat(5,5) = 0.0;
   CovMat(6,6) = 0.0;
   CovMat(7,7) = 0.0;
   CovMat(8,8) = 0.0;
   CovMat(9,9) = 0.0;
   fDArea = (tmppeak->IntegralError(int_low,int_high,tmppeak->GetParameters(),CovMat.GetMatrixArray())) /binWidth;

   if(print_flag) printf("Integral: %lf +/- %lf\n",fArea,fDArea);
   //Set the background for drawing later
   fBackground->SetParameters(GetParameters());
   //To DO: put a flag in signalling that the errors are not to be trusted if we have a bad cov matrix
   Copy(*fitHist->GetListOfFunctions()->Last());
 //  if(optstr.Contains("+"))
  //    Copy(*fitHist->GetListOfFunctions()->Before(fitHist->GetListOfFunctions()->Last()));
   
   delete tmppeak;
   return true;
}

void TPeak::Clear(Option_t* opt) {
//Clear the TPeak including functions and histogram, does not
//currently clear inherited members such as name.
//want to make a clear that doesn't clear everything
   fArea         = 0.0;
   fDArea       = 0.0;
   fChi2         = 0.0;
   fNdf          = 0.0;
   TGRSIFit::Clear();
   //Do deep clean stuff maybe? require an option?

}

void TPeak::Print(Option_t* opt) const {
//Prints TPeak properties. To see More properties use the option "+"
   printf("Name:        %s \n", GetName()); 
   printf("Centroid:    %lf +/- %lf \n", GetParameter("centroid"),GetParError(GetParNumber("centroid")));
   printf("Area: 	      %lf +/- %lf \n", fArea, fDArea);
   printf("FWHM:        %lf +/- %lf \n", GetParameter("sigma")*2.3548, GetParError(GetParNumber("sigma"))*2.3548);
   printf("Chi^2/NDF:   %lf\n",fChi2/fNdf);
   if(strchr(opt,'+') != NULL) {
      TF1::Print();
      TGRSIFit::Print(opt); //Polymorphise this a bit better
   }
}

const char*  TPeak::PrintString(Option_t* opt) const {
//Prints TPeak properties to a string, returns the string.
   std::string temp;
   temp.assign("Name:        ");temp.append(GetName()); temp.append("\n");
   temp.append("Centroid:    ");temp.append(Form("%lf",GetParameter("centroid")));
                                temp.append(" +/- ");
                                temp.append(Form("%lf",GetParError(GetParNumber("centroid")))); temp.append("\n");
   temp.append("Area: 	     ");temp.append(Form("%lf",fArea)); 
                                temp.append(" +/- ");
                                temp.append(Form("%lf",fDArea));    temp.append("\n"); 
   temp.append("Chi^2/NDF:   ");temp.append(Form("%lf",fChi2/fNdf)); temp.append("\n");
   //if(strchr(opt,'+') != NULL) {
   //   TF1::Print();
   TGRSIFit::Print(opt); //Polymorphise this a bit better
   //}
   return temp.c_str();
}

void TPeak::DrawBackground(Option_t* opt) const{
   fBackground->Draw(opt);
}

void TPeak::DrawResiduals() {
   if(!GetHist()) {
      printf("No hist set\n");
      return;
   }
   if(fChi2<0.000000001) {
      printf("No fit performed\n");
      return;
   }

   Double_t xlow,xhigh;
   GetRange(xlow,xhigh);
   Int_t nbins = GetHist()->GetXaxis()->GetNbins();
   Double_t* res = new Double_t[nbins];
   Double_t* bin = new Double_t[nbins];
   Int_t points = 0;
   fResiduals->Clear();

   for(int i =1;i<=nbins;i++) {
      if(GetHist()->GetBinCenter(i) <= xlow || GetHist()->GetBinCenter(i) >= xhigh)
         continue;
      res[points] = (GetHist()->GetBinContent(i) - Eval(GetHist()->GetBinCenter(i)))+ GetParameter("Height")/2;///GetHist()->GetBinError(i));// + GetParameter("Height") + 10.;
      bin[points] = GetHist()->GetBinCenter(i);
      fResiduals->SetPoint(i,bin[i],res[i]);

      points++;
   }

   fResiduals->Draw();

   delete[] res;
   delete[] bin;
}

Double_t TPeak::GetIntegralArea() {
   if(!GetHist()) {
      printf("No hist set\n");
      return 0;
   }
   if(fChi2<0.000000001) {
      printf("No fit performed\n");
      return 0;
   }

   Double_t width = GetParameter("sigma");
   Double_t xlow,xhigh;
   Double_t int_low, int_high; 
   GetRange(xlow,xhigh);
   int_low = xlow - 10.*width; // making the integration bounds a bit smaller, but still large enough. -JKS
   int_high = xhigh + 10.*width;
   return GetIntegralArea(int_low,int_high);
}

Double_t TPeak::GetIntegralArea(Double_t int_low, Double_t int_high) {
   if(!GetHist()) {
      printf("No hist set\n");
      return 0;
   }
   if(fChi2<0.000000001) {
      printf("No fit performed\n");
      return 0;
   }

   // pull appropriate properties from peak and histogram
   TH1* hist = GetHist();

   // use those properties to integrate the histogram
   Int_t binlow = hist->FindBin(int_low);
   Int_t binhigh = hist->FindBin(int_high);
   Double_t binWidth = hist->GetBinWidth(binlow);
   Double_t hist_integral = hist->Integral(binlow,binhigh);
   Double_t xlow = hist->GetXaxis()->GetBinLowEdge(binlow);
   Double_t xhigh = hist->GetXaxis()->GetBinUpEdge(binhigh);

   // ... and then integrate the background
   Double_t bg_area = (this->Background()->Integral(xlow,xhigh))/binWidth;

   // calculate the peak area and error
   Double_t peakarea = hist_integral - bg_area;

   return peakarea;
}

Double_t TPeak::GetIntegralAreaErr(Double_t int_low, Double_t int_high) {
   if(!GetHist()) {
      printf("No hist set\n");
      return 0;
   }
   if(fChi2<0.000000001) {
      printf("No fit performed\n");
      return 0;
   }

   // pull appropriate properties from peak and histogram
   TH1* hist = GetHist();

   // use those properties to integrate the histogram
   Int_t binlow = hist->FindBin(int_low);
   Int_t binhigh = hist->FindBin(int_high);
   Double_t binWidth = hist->GetBinWidth(binlow);
   Double_t hist_integral = hist->Integral(binlow,binhigh);
   Double_t xlow = hist->GetXaxis()->GetBinLowEdge(binlow);
   Double_t xhigh = hist->GetXaxis()->GetBinUpEdge(binhigh);

   // ... and then integrate the background
   Double_t bg_area = (this->Background()->Integral(xlow,xhigh))/binWidth;

   // calculate the peak error
   Double_t peakerr = sqrt(hist_integral+bg_area);

   return peakerr;

}

Double_t TPeak::GetIntegralAreaErr() {
   if(!GetHist()) {
      printf("No hist set\n");
      return 0;
   }
   if(fChi2<0.000000001) {
      printf("No fit performed\n");
      return 0;
   }

   Double_t width = GetParameter("sigma");
   Double_t xlow,xhigh;
   Double_t int_low, int_high; 
   GetRange(xlow,xhigh);
   int_low = xlow - 10.*width; // making the integration bounds a bit smaller, but still large enough. -JKS
   int_high = xhigh + 10.*width;
   return GetIntegralAreaErr(int_low,int_high);
}

void TPeak::CheckArea(Double_t int_low, Double_t int_high) {
   if(!GetHist()) {
      printf("No hist set\n");
      return;
   }
   if(fChi2<0.000000001) {
      printf("No fit performed\n");
      return;
   }

   // calculate the peak area and error
   Double_t peakarea = GetIntegralArea(int_low,int_high);
   Double_t peakerr = GetIntegralAreaErr(int_low,int_high);

   // now print properties
   printf("TPeak integral: 	      %lf +/- %lf \n", fArea, fDArea);
   printf("Histogram - BG integral:        %lf +/- %lf \n", peakarea,peakerr);
   if (abs(peakarea-fArea)<(fDArea+peakerr)) printf(DGREEN "Areas are consistent.\n" RESET_COLOR);
   else if (abs(peakarea-fArea)<2*(fDArea+peakerr)) printf(DYELLOW "Areas are consistent within 2 sigma.\n" RESET_COLOR);
   else printf (DRED "Areas are inconsistent.\n" RESET_COLOR);
   
   return;
}

void TPeak::CheckArea() {
   if(!GetHist()) {
      printf("No hist set\n");
      return;
   }
   if(fChi2<0.000000001) {
      printf("No fit performed\n");
      return;
   }

   // calculate the peak area and error
   Double_t peakarea = GetIntegralArea();
   Double_t peakerr = GetIntegralAreaErr();

   // now print properties
   printf("TPeak integral: 	      %lf +/- %lf \n", fArea, fDArea);
   printf("Histogram - BG integral:        %lf +/- %lf \n", peakarea,peakerr);
   if (abs(peakarea-fArea)<(fDArea+peakerr)) printf(DGREEN "Areas are consistent.\n" RESET_COLOR);
   else if (abs(peakarea-fArea)<2*(fDArea+peakerr)) printf(DYELLOW "Areas are consistent within 2 sigma.\n" RESET_COLOR);
   else printf(DRED "Areas are inconsistent.\n" RESET_COLOR);
   
   return;
}
