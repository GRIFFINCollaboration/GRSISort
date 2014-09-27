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
#include "TROOT.h"
//#include "../include/TNucleus.h"

#include "../include/TGRSITransition.h"

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

//gSystem->Load("libMathCore"); //Might be able to include this through linking libraries etc.

//This is a good example for how to fit nearby peaks together/multiple peaks
Int_t npeaks = 30;


////////////////////////     This is where the fits are defined     /////////////////////////////////////


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



/////////////////////////    This is there the fitting takes place    //////////////////////////////


bool FitPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea){

   Double_t binWidth = h->GetXaxis()->GetBinWidth(1000);//Need to find the bin widths so that the integral makes sense
   Int_t rw = binWidth*40;  //This number may change depending on the source used   
   //Set the number of iterations. The code is pretty quick, so having a lot isn't an issue	
   TVirtualFitter::SetMaxIterations(4999);
   Int_t xp = par[1];

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
   pp->SetParLimits(1,xp-rw,xp+rw);
   pp->SetParLimits(3,0,30);
   pp->SetParLimits(4,0,10);
   pp->SetParLimits(5,0.000,1000000);
   pp->SetParLimits(9,xp-40,xp+40);

   //Actually set the parameters in the photopeak function
   pp->SetParameters(par);

//   pp->FixParameter(4,0);
 //  pp->FixParameter(5,0);

   pp->SetNpx(1000); //Draws a nice smooth function on the graph
   TFitResultPtr fitres = h->Fit("photopeak","RFS");
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

   area = integral;
   darea = sigma_integral;

   if(fitres->Chi2()/fitres->Ndf() > 30.0)
      return false;

   return true;

//   myFitResult.fIntegral = integral;


 //  return photopeak;

}


/*
void FitSpectrum(Int_t npeaks, TH1F* h1){

      //Use TSpectrum to find the peak candidates
      //This should be done once per histogram

      Double_t par[15];

      TSpectrum *s = new TSpectrum(npeaks, 100);
      Int_t nfound = s->Search(h1,2,"",0.75); //This will be dependent on the source used.
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
         f = FitPeak(par,h1);
      //   fitlist->Add(f);
         npeaks++;
       }
     delete s;



}

/////////////////////     This is the loop that finds the peaks and sends them to the fitter    ////////////////////////////

int autoefficGARBAGE(const char *histfile,           //File with all of the histograms to be fit
              const char *sourcename = "60Co",//Name of the source being used. Not used at this time
              Double_t activity = 0.0,        //This will be dependent on the source used. //Activity of the source in uCi
              bool kvis = true        ) {     //Display The fits on a TPad  
                                   
   TList *fitlist = new TList;

   //This defines the 60Co source
   Int_t np = 2;
   npeaks = TMath::Abs(np);
   std::vector<Double_t> energy(1173.228,1332.492);
   std::vector<Double_t> intensity(99.85,99.9826);

   TH2F * eff = new TH2F("efficiency","efficiency",64,0,64,npeaks,0,npeaks);

   TFile *file = new TFile(histfile,"READ"); 

//  file.ls(); 
  
   Int_t counter = 0;
   Int_t plotcounter = 0;
   char detname[20];
   char canvname[10];

   for(Int_t j = 0; j < 16*16*16*16; j++){
      sprintf(detname, "Charge_0x%04x", j);
      TH1F * h1 = (TH1F*)file->Get(detname);
 
   if(h1){

      if(kvis && counter%16 == 0){
         sprintf(canvname, "c%d",plotcounter);
         TCanvas *c1 = new TCanvas(canvname,canvname,10,10,1000,900);
         c1->DivideSquare(16);
         plotcounter++;   
      }
      
      std::cout << counter++ << std::endl;

      c1->cd(counter%16+1); 

      FitSpectrum(npeaks,h1);

     }
   }  
}

*/


TH1D *GetProjectionY(TH2 *mat,double first,double last =0.0) {
   if(!mat) {
      printf("this happened.\n");
      return 0;
   }
   if(last == 0.0)
      last = first;

   int fbin = mat->GetXaxis()->FindBin(first);
   int lbin = mat->GetXaxis()->FindBin(last);

   if(first == last)
      return mat->ProjectionY(Form("proj_%i",(int)first),fbin,lbin);
   else
      return mat->ProjectionY(Form("proj_%i-%i",(int)first,(int)last),fbin,lbin);
}



int autogain(TH1 *hist,const char *name) {
  TNucleus nuc(name);
  autogain(hist,&nuc);
  return 1;
}

TGraph* autogain(TH1 *hist,TNucleus *nuc) {    //Display The fits on a TPad  

   if(!hist || !nuc)
      return 0;

   nuc->SetSourceData();

   if(nuc->GetA() == 152) {
      return autogain152(hist);
   }

// Search
   hist->GetXaxis()->SetRangeUser(200.,16000.);
   TSpectrum *s = new TSpectrum();
   Int_t nfound = s->Search(hist,6,"",0.1); //This will be dependent on the source used.
   printf("Found %d candidate peaks to fit\n",nfound);
// Match

   nuc->TransitionList.Sort();

   std::vector<float> engvec;
   TIter iter(&(nuc->TransitionList));
   TObject* obj;
   while(obj = iter.Next()) {
      if(!obj->InheritsFrom("TGRSITransition"))
         continue;
      TGRSITransition *tran = (TGRSITransition*)obj;

      engvec.push_back(static_cast<float>(tran->energy));
      if(engvec.size() == nfound)
         break;
   }

   if(nfound != engvec.size())
      return 0;

   Float_t *posPeaks = s->GetPositionX();
   Float_t *energies = &(engvec[0]);

   for(int x=0;x<nfound;x++) {
      printf("posPeaks[%i] = %f\t\tenrgies[%i] = %f\n",x,posPeaks[x],x,energies[x]);
   }

   TGraph *slopefit = new TGraph(nfound,posPeaks,energies ); 

   printf("Now fitting: Be patient\n");
   slopefit->Fit("pol1");
   slopefit->Draw("AC*");

   return slopefit;

}

TGraph* autogain152(TH1 *hist) {

   hist->GetXaxis()->SetRangeUser(200.,16000.);
   TSpectrum *s = new TSpectrum();
   Int_t nfound = s->Search(hist,6,"",0.08); //This will be dependent on the source used.
   printf("Found %d candidate peaks to fit\n",nfound);
   if(nfound > 6)
      nfound = 6;

   std::vector<float> vec;
   for(int x=0;x<nfound;x++)
      vec.push_back(s->GetPositionX()[x]);

   std::sort(vec.begin(),vec.end());

   Float_t energies[] = {121.7830, 244.6920, 344.276, 778.903, 964.131, 1408.011};
   TGraph* slopefit = new TGraph(nfound, &(vec[0]), energies);

   printf("Now fitting: Be patient\n");
   slopefit->Fit("pol1");
   if(slopefit->GetFunction("pol1")->GetChisquare() > 10.) {
      slopefit->RemovePoint(slopefit->GetN()-1);
      slopefit->Fit("pol1");
   }
   TChannel *chan = 0;
   slopefit->Draw("AC*");

   return slopefit;
}


void DoFit(TH2 *hist) {
   TChannel::DeleteAllChannels();
   TChannel::ReadCalFile("GrifCal.cal");
   printf("made %i channels.\n",TChannel::GetNumberOfChannels());
   TNucleus nuc("152eu");
   for(int x = 1; x <= 64; ++x) {
      printf(" x = %i\n",x);
      TH1 *p = GetProjectionY(hist,x);
      if(p->GetEntries() < 100)
         continue;
      TGraph* graph = autogain(p,&nuc);
      TChannel *chan = TChannel::GetChannelByNumber(x);
      if(!chan)
         continue;
      chan->DestroyCalibrations();
      chan->AddENGCoefficient(graph->GetFunction("pol1")->GetParameter(0));
      chan->AddENGCoefficient(graph->GetFunction("pol1")->GetParameter(1));
      chan->SetIntegration(125);
   }

   TChannel::WriteCalFile("NewGrifCal.cal");

}

int autoefficiency(TH1 *hist,const char *name,Double_t runlengthsecs, Double_t activitykBq=1.0) {
  gSystem->Load("libNucleus");
  TNucleus nuc(name);
  TGraph *graph;
  graph = autoefficiency(hist,&nuc, runlengthsecs,activitykBq);
  return 1;
}

TGraph* autoefficiency(TH1 *hist,TNucleus *nuc,Double_t runlengthsecs,  Double_t activitykBq) {    //Activity in uCi

   if(!hist || !nuc)
      return 0;

   Double_t par[40];

   nuc->SetSourceData();

 //  if(nuc->GetA() == 152) {
 //     return autogain152(hist);
 //  }

// Search
   hist->GetXaxis()->SetRangeUser(0,16000);

//   nuc->TransitionList.Sort();

   std::vector<float> engvec;
   std::vector<float> intensvec;
   TIter iter(&(nuc->TransitionList));
   TObject* obj;
   while(obj = iter.Next()) {
      if(!obj->InheritsFrom("TGRSITransition"))
         continue;
      TGRSITransition *tran = (TGRSITransition*)obj;
      intensvec.push_back(static_cast<float>(tran->intensity));
      engvec.push_back(static_cast<float>(tran->energy));
   }

 

   std::vector<Float_t> areavec;
   std::vector<Float_t> area_uncertainty;
   std::vector<Float_t> goodenergyvec;
   std::vector<Float_t> goodintensvec;

   Float_t integral, sigma; 
   Double_t binWidth = hist->GetXaxis()->GetBinWidth(10000);

   std::cout << "bin width is " << binWidth << std::endl;
  for (int p=0;p<engvec.size();p++) {
     Float_t xp = engvec.at(p)/binWidth;
     std::cout << "Trying to fit " << xp << " keV" <<std::endl; 

     Int_t bin = hist->GetXaxis()->FindBin(xp);
     Float_t yp = hist->GetBinContent(bin);
     par[0] = yp;  //height
     par[1] = xp;  //centroid
     par[2] = 2;   //sigma
     par[3] = 5;   //beta
     par[4] = 0;   //R
     par[5] = 5.0;//stp
     par[6] = hist->GetBinContent(bin+25);  //A
     par[7] = -1.0;//B
     par[8] = -0.5;   //C
     par[9] = xp;  //bg offset
     bool goodfit = FitPeak(par,hist,integral, sigma);
     if(goodfit){
     	areavec.push_back(integral/((intensvec.at(p)/100.0)*activitykBq*1000.0*runlengthsecs));
     	area_uncertainty.push_back(sigma);
        goodenergyvec.push_back(engvec.at(p));
  //      goodintensvec.push_back(intensvec.at(p));

     }
  //   fitlist->Add(f);
   }

   std::cout << "or made it here" << std::endl;

   Float_t *area = &(areavec[0]);
 //  Float_t *energies = &(engvec[0]);
   Float_t *goodenergy = &(goodenergyvec[0]);

   TGraph *slopefit = new TGraph(areavec.size(),goodenergy,area ); 

   printf("Now fitting: Be patient\n");
 //  slopefit->Fit("pol1");
   slopefit->Draw("PA*");
   for(int x=0;x<areavec.size();x++) {
      printf("areavec[%i] = %f\t\tgoodenergyvec[%i] = %f\n",x,areavec[x],x,goodenergyvec[x]);
   }

   return slopefit;

}

