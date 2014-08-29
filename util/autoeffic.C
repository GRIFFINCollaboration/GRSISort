#include "TH1.h"
#include "TF1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMath.h"
#include "TCanvas.h"
#include <map>
#include <vector>
#include <stdint>

//This fitting function is based on the fitting function used in gf3 written by D.C. Radford
//Each peak is composed of three components:
//(1) a Gaussian, 
//(2) a skewed Gaussian, and 
//(3) a smoothed step function to increase the background on the low-energy side of the peak. Components (2) and/or (3) can easily be set to zero if not required.
//Component (1), the Gaussian, is usually the main component of the peak, and in Ge detectors, physically arises from complete charge collection of a photoelectric event in the detector.
//Component (2), the skewed Gaussian, arises from incomplete charge collection , often due to "trapping" of charge at dislocations in the crystal lattice caused by impurities or neutron damage. If the detector and electronics had perfect resolution, component (1) would be a delta-function and component (2) would yield an exponential tail on the low-energy side. Convolution of this exponential tail with a Gaussian resolution function yields the functional form:

//y = constant * EXP( (x-c)/beta ) * ERFC( (x-c)/(SQRT(2)*sigma) + sigma/(SQRT(2)*beta) )
//where ERFC is the complement of the error function, x is the channel number, c and sigma are the centroid and standard deviation of the Gaussian in component (1), and beta is the decay constant of the exponential. Beta now corresponds to the "skewedness" of the skewed Gaussian.
//Component (3) arises mainly from Compton scattering of photons INTO the detector and from escape of photoelectrons from the Ge crystal, which result in a slightly higher background on the low-energy side of the peak. The functional form used in gf3 is:

//y = constant * ERFC( (x-c)/(SQRT(2)*sigma) )
//which is produced by the convolution of a step function with a Gaussian of width sigma.

//Might have to include namespace?

gSystem->Load("libMathCore"); //Might be able to include this through linking libraries etc.

//This is a good example for how to fit nearby peaks together/multiple peaks
Int_t npeaks = 30;


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

// Quadratic background function
Double_t background(Double_t *x, Double_t *par) {
   return par[0] + par[1]*(x[0]-par[3]) + par[2]*(x[0]-par[3])*(x[0]-par[3]);
}

Double_t photo_peak(Double_t *dim, Double_t *par){
   
   //Define the parameters for easy use.
   Double_t x        = dim[0]; //channel number used for fitting 
   Double_t height   = par[0]; //height of photopeak
   Double_t c        = par[1]; //Peak Centroid of non skew gaus
   Double_t sigma    = par[2]; //standard deviation  of gaussian
   Double_t beta     = par[3]; //"skewedness" of the skewed gaussian
   Double_t R        = par[4]; //relative height of skewed gaussian

   Double_t gaussian    = height*(1.0-R/100.0)*TMath::Gaus(x,c,sigma);
   Double_t skewed_gaus = R*height/100.0*(TMath::Exp((x-c)/beta))*(ROOT::Math::erfc((x-c)/(TMath::Sqrt(2.0)*sigma)) + sigma/(TMath::Sqrt(2.0)*beta));

   return gaussian + skewed_gaus;
}

Double_t step_function(Double_t *dim, Double_t *par){

   Double_t x       = dim[0];
   Double_t height  = par[0];
   Double_t c       = par[1];
   Double_t sigma   = par[2];
   Double_t step    = par[5];

   return TMath::Abs(step)*height/100.0*ROOT::Math::erfc((x-c)/(TMath::Sqrt(2.0)*sigma));
}

Double_t fitFunction(Double_t *dim, Double_t *par){

   return photo_peak(dim, par) + step_function(dim,par) + background(dim,&par[6]);
}

TF1* PeakFitFuncs(Double_t *par, TH1F *h){

   Double_t binWidth = h->GetXaxis()->GetBinWidth(0);//Need to find the bin widths so that the integral makes sense
   Int_t rw = binWidth*60;  //This number may change depending on the source used   
   //Set the number of iterations. The code is pretty quick, so having a lot isn't an issue	
   TVirtualFitter::SetMaxIterations(4999);
   Int_t xp = par[1];

   //Define the fit function and the range of the fit
   TF1 *pp = new TF1("pp",fitFunction,xp-rw,xp+rw,10);

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
   pp->SetParLimits(1,xp-rw,xp+rw);
   pp->SetParLimits(3,0,30);
   pp->SetParLimits(4,0,10);
   pp->SetParLimits(5,0.000,1000000);
   pp->SetParLimits(9,xp-3,xp+3);

   //Actually set the parameters in the photopeak function
   pp->SetParameters(par);

//   pp->FixParameter(4,0);
 //  pp->FixParameter(5,0);

   pp->SetNpx(1000); //Draws a nice smooth function on the graph
   TFitResultPtr fitres = h->Fit("pp","RFS");
   pp->Draw("same");      

   pp->GetParameters(&par[0]); 
   TF1 *photopeak = new TF1("photopeak",photo_peak,xp-rw,xp+rw,10);
   photopeak->SetParameters(par);

   Double_t integral = photopeak->Integral(xp-rw,xp+rw)/binWidth;


   std::cout << "FIT RESULT CHI2 " << fitres->Chi2() << std::endl;


   std::cout << "FWHM = " << 2.35482*fitres->Parameter(2)/binWidth <<"(" << fitres->ParError(2)/binWidth << ")" << std::endl;
   std::cout << "NDF: " << fitres->Ndf() << std::endl;
   std::cout << "X sq./v = " << fitres->Chi2()/fitres->Ndf() << std::endl;

   TVirtualFitter *fitter = TVirtualFitter::GetFitter();

   assert(fitter != 0); //make sure something was actually fit
   Double_t * covMatrix = fitres->GetCovarianceMatrix(); //This allows us to find the uncertainty in the integral

   Double_t sigma_integral = photopeak->IntegralError(xp-rw,xp+rw)/binWidth;

   std::cout << "Integral = " << integral << " +/- " << sigma_integral << std::endl;

   myFitResult.fIntegral = integral;


   return pp;

}

int autoeffic(const char *histfile,const char *sourcename = "60Co"){

   TList *fitlist = new TList;

   //This defines the 60Co source
   Int_t np = 2;
   std::vector<Double_t> energy(1173.228,1332.492);
   std::vector<Double_t> intensity(99.85,99.9826);

   peaks = TMath::Abs(np);

   Double_t par[3000];

//Instead of generating peaks read in a histogram
   TFile *file = new TFile(histfile,"READ"); 

//  file.ls(); 
  
   TH1F * h1 = (TH1F*)file->Get("Charge_0x0007"); 

   TCanvas *c1 = new TCanvas("c1","c1",10,10,1000,900);
   c1->Divide(1,2);
   c1->cd(1);
   h1->Draw();
   TH1F *h2 = (TH1F*)h1->Clone("h2");

   //make a TList to store functions for fitting.
//   TList *fitFunctions = new TList;

   c1->cd(2);
   //Use TSpectrum to find the peak candidates
   //This should be done once per histogram
   TSpectrum *s = new TSpectrum(npeaks, 100);
   Int_t nfound = s->Search(h1,2,"",0.75);
   printf("Found %d candidate peaks to fit\n",nfound);
   printf("Now fitting: Be patient\n");
   Float_t *xpeaks = s->GetPositionX();
   for (int p=0;p<nfound;p++) {
      Float_t xp = xpeaks[p];
      Int_t bin = h1->GetXaxis()->FindBin(xp);
      Float_t yp = h1->GetBinContent(bin);
      par[0] = yp;  //height
      par[1] = xp;  //centroid
      par[2] = 2;   //sigma
      par[3] = 5;   //beta
      par[4] = 0;   //R
      par[5] = 1.0;//stp
      par[6] = 50;  //A
      par[7] = -0.2;//B
      par[8] = 0;   //C
      par[9] = xp;  //bg offset
    //  fitFunctions->Add(PeakFitFuncs(par));
      f = PeakFitFuncs(par,h2);
      fitlist->Add(f);
      npeaks++;
   }
   //c1->cd(2)
     
}
  
