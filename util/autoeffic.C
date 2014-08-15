#include "TH1.h"
#include "TF1.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMath.h"
#include "TCanvas.h"
#include <map>
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

Double_t photo_peak(Double_t *dim, Double_t *a){
   
   Double_t x        = dim[0];
   Double_t constant = a[0]; 
   Double_t c        = a[1]; //Peak Centroid
   Double_t beta     = a[2]; //"skewedness" of the skewed gaussian
   Double_t sigma    = a[3]; //standard deviation  of gaussian

   Double_t fitval = constant*TMath::Exp((x-c)/beta)*(ROOT::Math::erfc((x-c)/TMath::Sqrt(2.0)*sigma) + sigma/(TMath::Sqrt(2.0)*beta));

   return fitval;
}

Double_t background(Double_t *dim, Double_t *a){

   Double_t x        = dim[0];
   Double_t constant = a[0];
   Double_t c        = a[1];
   Double_t beta     = a[2]; //Not really used here but put in here to make things more consistent later
   Double_t sigma    = a[3];   

   Double_t fitval = constant*ROOT::Math::erfc((x-c)/(TMath::Sqrt(2)*sigma));

   return fitval;
}

Double_t fitFunction(Double_t *dim, Double_t *a){
   return photo_peak(dim, a) + background(dim,a);
}

int autoeffic(Int_t np = 2){
   npeaks = TMath::Abs(np);
   TH1F *h = new TH1F("h", "test", 500,0,1000);
   //Generate n peaks at random 
   //This will be replaced by real data.
   Double_t par[3000];
   par[0] = 0.8;
   par[1] = -0.6/1000;
   Int_t p;
   for (p=0;p<npeaks;p++) {
      par[3*p+2] = 1;
      par[3*p+3] = 10+gRandom->Rndm()*980;
      par[3*p+4] = 3+2*gRandom->Rndm();
   }

   TF1 *f = new TF1("f",fpeaks,0,1000,2+3*npeaks);
   f->SetNpx(1000);
   f->SetParameters(par);
   TCanvas *c1 = new TCanvas("c1","c1",10,10,1000,900);
   c1->Divide(1,2);
   c1->cd(1);
   h->FillRandom("f",200000);
   h->Draw();
   TH1F *h2 = (TH1F*)h->Clone("h2");
   //Use TSpectrum to find the peak candidates
   TSpectrum *s = new TSpectrum(2*npeaks);
   Int_t nfound = s->Search(h,2,"",0.10);
   printf("Found %d candidate peaks to fit\n",nfound);
   //Estimate backgroundfit using TSpectrum::Background
   TH1 *hb = s->Background(h,20,"same");
   if (hb) c1->Update();
   if (np <0) return;

   //estimate linear background using a fitting method
   c1->cd(2);
   TF1 *fline = new TF1("fline","pol1",0,1000);
   h->Fit("fline","qn");
   //Loop on all found peaks. Eliminate peaks at the background level
   par[0] = fline->GetParameter(0);
   par[1] = fline->GetParameter(1);
   npeaks = 0;
   Float_t *xpeaks = s->GetPositionX();
   for (p=0;p<nfound;p++) {
      Float_t xp = xpeaks[p];
      Int_t bin = h->GetXaxis()->FindBin(xp);
      Float_t yp = h->GetBinContent(bin);
      if (yp-TMath::Sqrt(yp) < fline->Eval(xp)) continue;
      par[3*npeaks+2] = yp;
      par[3*npeaks+3] = xp;
      par[3*npeaks+4] = 3;
      npeaks++;
   }
   printf("Found %d useful peaks to fit\n",npeaks);
   printf("Now fitting: Be patient\n");
   TF1 *fit = new TF1("fit",fpeaks,0,1000,2+3*npeaks);
   //we may have more than the default 25 parameters
   TVirtualFitter::Fitter(h2,10+3*npeaks);
   fit->SetParameters(par);
   fit->SetNpx(1000);
   h2->Fit("fit");             
}
  
