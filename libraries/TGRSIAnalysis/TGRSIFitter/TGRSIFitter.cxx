#include "TGRSIFitter.h"

ClassImp(TGRSIFitter)

//This will be changed
Double_t TGRSIFitter::fitFunction(Double_t *dim, Double_t *par){
   return PhotoPeak(dim, par) + StepFunction(dim,par) + PolyBg(dim,&par[6],3);  
}

Double_t TGRSIFitter::multifitFunction(Double_t *dim, Double_t *par,Int_t npeaks){
   Double_t result = PolyBg(dim,par,3);
   for(Int_t p=0;p<npeaks;p++){
      result += PhotoPeak(dim,&par[6*p+4]) + StepFunction(dim,&par[6*p+4]);
   }
   return result;
}


/*
Double_t fpeaks(Double_t *x, Double_t *par) {
   Double_t result = par[0] + par[1]*x[0];
   for (Int_t p=0;p<npeaks;p++) {
      Double_t norm  = par[3*p+2];
      Double_t mean  = par[3*p+3];
      Double_t sigma = par[3*p+4];
      result += norm*TMath::Gaus(x[0],mean,sigma);
   }
   return result;
}
*/

//void TGRSIFitter::FitPeak(Int_t limit1, Int_t limit2, std::initializer_list<double> centroid){

//}

int TGRSIFitter::FitPeak(Int_t limit1, Int_t limit2, Double_t centroid) {} // termination version

/*
void TGRSIFitter::FitNPeaks(Int_t limit1, Int_t limit2, Int_t npeaks, ...){
//This fits a GRIFFIN photopeak. This apparantly may not be portable to OSX. Need to test.
   va_list centroids;
   va_start(centroids, npeaks+2);
   while (npeaks-- > 2)
             va_arg(centroids, Double_t));
            va_end(args);

// return first + sum(...);
}
*/

//This function is used to perform the actual fit
Bool_t TGRSIFitter::FitPhotoPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, Bool_t verbosity){

   //Change the bin width to the bin containing the centroid
   Double_t binWidth = h->GetXaxis()->GetBinWidth(1000);//Need to find the bin widths so that the integral makes sense
   Int_t rw = binWidth*120;  //This number may change depending on the source used   
   //Set the number of iterations. The code is pretty quick, so having a lot isn't an issue	
   TVirtualFitter::SetMaxIterations(10000);
   Int_t xp = par[1];
   Int_t yp = par[0];
   Int_t A = par[6];
   //Define the fit function and the range of the fit
   TF1 *pp = new TF1("photopeak",fitFunction,xp-rw,xp+rw,10);

   //Name the parameters so it is easy to see what the code is doing
   pp->SetParName(0,"Height");
   pp->SetParName(1,"centroid");
   pp->SetParName(2,"sigma");
   pp->SetParName(3,"beta");
   pp->SetParName(4,"R");
   pp->SetParName(5,"step");
   pp->SetParName(6,"A");
   pp->SetParName(7,"B");
   pp->SetParName(8,"C");
   pp->SetParName(9,"bg offset");

   //Set some physical limits for parameters
   pp->SetParLimits(0,0.5*yp, 2*yp);
   pp->SetParLimits(1,xp-rw,xp+rw);
 //  pp->SetParLimits(2,4,12);
   pp->SetParLimits(3,0.000,10);
   pp->SetParLimits(4,0,500);
   pp->SetParLimits(6,0,A*1.4);
   pp->SetParLimits(5,0,1000000);
   pp->SetParLimits(9,xp-40,xp+40);
   //Actually set the parameters in the photopeak function
   pp->SetParameters(par);
  //Fixing has to come after setting
  pp->FixParameter(8,0);
  // pp->FixParameter(4,0);
 //  pp->FixParameter(7,0);
//   pp->FixParameter(4,0);
  // pp->FixParameter(5,1);

   std::string optionstring;

   if(verbosity)
      optionstring = "RFSM0+";
   else
      optionstring = "RFSM0Q+";
 
   const char * options = optionstring.c_str();

   pp->SetNpx(1000); //Draws a nice smooth function on the graph
   TFitResultPtr fitres = h->Fit("photopeak",options); 
//   pp->Draw("same");      
   
   if(fitres->ParError(2) != fitres->ParError(2)){ //Check to see if nan
      if(fitres->Parameter(3) < 1){
         pp->FixParameter(4,0);
         pp->FixParameter(3,1);
         std::cout << "Beta may have broken the fit, retrying with R=0" << std::endl;
         fitres = h->Fit("photopeak",options);
         pp->ReleaseParameter(4);
         pp->SetParLimits(4,0,500);
         fitres = h->Fit("photopeak",options);
         pp->ReleaseParameter(3);
         pp->SetParLimits(3,0.000,10);
         fitres = h->Fit("photopeak",options);
      }
   }
   

   pp->GetParameters(&par[0]); 
   TF1 *photopeak = new TF1("photopeak",PhotoPeak,xp-rw,xp+rw,10);//I Think this is supposed to be just a gaussian maybe.
   photopeak->SetParameters(par);

   Double_t integral = photopeak->Integral(xp-rw,xp+rw)/binWidth;

   if(verbosity){
      std::cout << "FIT RESULT CHI2 " << fitres->Chi2() << std::endl;
      std::cout << "FWHM = " << 2.35482*fitres->Parameter(2)/binWidth <<"(" << fitres->ParError(2)/binWidth << ")" << std::endl;
      std::cout << "NDF: " << fitres->Ndf() << std::endl;
   }
   std::cout << "X sq./v = " << fitres->Chi2()/fitres->Ndf() << std::endl;

   TVirtualFitter *fitter = TVirtualFitter::GetFitter();

   assert(fitter != 0); //make sure something was actually fit
   TMatrixDSym covMatrix = fitres->GetCovarianceMatrix(); //This allows us to find the uncertainty in the integral

   Double_t sigma_integral = photopeak->IntegralError(xp-rw,xp+rw)/binWidth;

   std::cout << "Integral = " << integral << " +/- " << sigma_integral << std::endl;

   area = integral;
   darea = sigma_integral;
   *energy = fitres->Parameter(1);

	delete photopeak;
	delete pp;
   if(fitres->Chi2()/fitres->Ndf() > 5.0){
      std::cout << "THIS FIT WAS NOT GOOD (Reduced Chi2 = " << fitres->Chi2()/fitres->Ndf() << " )" << std::endl;
      return false;
   }

   return true;

//   myFitResult.fIntegral = integral;


 //  return photopeak;

}



