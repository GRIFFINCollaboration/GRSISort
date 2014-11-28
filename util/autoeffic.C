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
#include "TMultiGraph.h"
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
   Double_t skewed_gaus = R*height/100.0*(TMath::Exp((x-c)/beta))*(ROOT::Math::erfc(((x-c)/(TMath::Sqrt(2.0)*sigma)) + sigma/(TMath::Sqrt(2.0)*beta)));

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


TFitResultPtr FitPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, bool verbosity = false){

   Double_t binWidth = h->GetXaxis()->GetBinWidth(par[1]);//Need to find the bin widths so that the integral makes sense
   Int_t rw = binWidth*100;  //This number may change depending on the source used   
   //Set the number of iterations. The code is pretty quick, so having a lot isn't an issue	
   TVirtualFitter::SetMaxIterations(10000);
   Int_t xp = par[1];
   Int_t yp = par[0];
   std::cout << "Now yp is: "<<yp << std::endl;
   std::cout << "A is: " << par[6] <<std::endl;
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
   pp->SetParLimits(2,1,12);
   pp->SetParLimits(3,0.000,10);
   pp->SetParLimits(4,0,500);
   pp->SetParLimits(6,0,A*1.4);
   pp->SetParLimits(5,0,1000000);
   pp->SetParLimits(9,xp-20,xp+20);
   //Actually set the parameters in the photopeak function
   pp->SetParameters(par);
  //Fixing has to come after setting
  pp->FixParameter(8,0);
 // pp->FixParameter(3,0);
 //  pp->FixParameter(7,0);
 //  pp->FixParameter(4,0);
  // pp->FixParameter(5,1);

   if(verbosity)
      const char * options = "R0FSM+";
   else
      const char * options = "R0FSMQ+";

   pp->SetNpx(1000); //Draws a nice smooth function on the graph
   TFitResultPtr fitres = h->Fit("photopeak",options); 
   
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
   
  // pp->DrawCopy("same");   
  // h->Draw();
  // fitres->DrawClone("same");
 //Will probably have to change this to not include skewed gaussian? CHECK
   TF1 *photopeak = new TF1("photopeak",photo_peak,xp-rw,xp+rw,10);
   photopeak->SetParameters(par);

   Double_t integral = photopeak->Integral(xp-rw,xp+rw)/binWidth;

   if(verbosity){
      std::cout << "FIT RESULT CHI2 " << fitres->Chi2() << std::endl;
      std::cout << "FWHM = " << 2.35482*fitres->Parameter(2) <<"(" << fitres->ParError(2) << ")" << std::endl;
      std::cout << "NDF: " << fitres->Ndf() << std::endl;
   }
   std::cout << "X sq./v = " << fitres->Chi2()/fitres->Ndf() << std::endl;

   TVirtualFitter *fitter = TVirtualFitter::GetFitter();

   assert(fitter != 0); //make sure something was actually fit
   Double_t * covMatrix = fitres->GetCovarianceMatrix(); //This allows us to find the uncertainty in the integral

   Double_t sigma_integral = photopeak->IntegralError(xp-rw,xp+rw)/binWidth;

   std::cout << "Integral = " << integral << " +/- " << sigma_integral << std::endl;

   area = integral;
   darea = sigma_integral;
   *energy = fitres->Parameter(1);

	delete photopeak;
	delete pp;
   if(fitres->Chi2()/fitres->Ndf() > 17.0){
      std::cout << "THIS FIT WAS NOT GOOD (Reduced Chi2 = " << fitres->Chi2()/fitres->Ndf() << " )" << std::endl;
      return false;
   }

   return fitres;

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

void autogain60(const char *f, int channum = -1, bool verbosity = false){

   TFile *file = new TFile(f,"READ"); 

	TH2D * matrix = (TH2D*)file->Get("hp_charge");
	std::cout << "Channum is: " << channum << std::endl;
	autogain60(matrix, channum, verbosity);
}


void autogain60(TH2D *mat, int channum = -1, bool verbosity = false){
	TH1D* h1 = new TH1D;
	if(channum == -1){
		for(int i=1; i<=64;i++){
			TH1D* h1 = (TH1D*) mat->ProjectionY(Form("Channel%d",i),i+1,i+1);
			if(h1->Integral() < 1)
				continue;
			autogain60(h1,i,verbosity);
		}
	}
	else{
		int i = channum;
		TH1D* h1 = (TH1D*) mat->ProjectionY(Form("Channel%d",i),i+1,i+1);
		if(h1->Integral() < 1)
			std::cout << "There are no counts in Channel " << channum << std::endl;
		else
			autogain60(h1,i,verbosity);
	}
	//std::cout << "IN THE SECOND FUNCTION" << std::endl;
}

TGraph* autogain60(TH1D *hist, int channum, bool verbosity = false){
   std::cout << "Now fitting channel "<<channum<<std::endl;
   static bool cal_flag = false;
//   TNucleus nuc("60Co"); 
//   TNucleus *nucptr = &nuc;
   std::vector<Double_t> engvec;
//   std::vector<float> intensvec;
//   TIter iter(&(nucptr->TransitionList));
//   TObject* obj;
//   while(obj = iter.Next()) {
//      std::cout << "Making a nucleus" << std::endl;
//      if(!obj->InheritsFrom("TGRSITransition"))
//         continue;
//      TGRSITransition *tran = (TGRSITransition*)obj;
//      intensvec.push_back(static_cast<float>(tran->intensity));
//      engvec.push_back(static_cast<float>(tran->energy));
//   }
   

   engvec.push_back(1173.228);
   engvec.push_back(1332.492);
 

  // std::vector<Float_t> areavec;
 //  std::vector<Float_t> area_uncertainty;
   std::vector<Double_t> goodenergyvec;
 //  std::vector<Float_t> goodintensvec;


   hist->GetXaxis()->SetRangeUser(200.,16000.);
   TSpectrum *s = new TSpectrum();
   Int_t nfound = s->Search(hist,2,"goff",0.08); //This will be dependent on the source used.
   
   printf("Found %d candidate peaks to fit\n",nfound);
   if(nfound > 2)
      nfound = 2;

   if(nfound <2){
      std::cout << "Did not find enough peaks" << std::endl;
      exit(1);
   }

   std::vector<Double_t> foundchan;
   for(int x=0;x<nfound;x++)
      foundchan.push_back(s->GetPositionX()[x]);

   if(verbosity)
      std::cout << "Found at position " << s->GetPositionX()[0] << std::endl;

    //std::sort(foundchan.begin(),foundchan.end());

  // Float_t energies[]={1173.228,1332.492}; //These will change
 
   Double_t par[40];  
   Float_t integral, sigma;
	bool goodfit = true;
   Double_t *centroid = new Double_t;

  for (int p=0;p<engvec.size();p++) {
     Double_t xp = foundchan[p];
  //   std::cout << "Trying to fit channel " << foundchan << " and match it to " << engvec[p]  <<std::endl; 
     Int_t bin = xp;// hist->GetXaxis()->FindBin(xp);
     Float_t yp = hist->GetBinContent(bin);
     par[0] = yp;  //height
     par[1] = xp;  //centroid
     par[2] = 5;   //sigma
     par[3] = 2;   //beta
     par[4] = 2;   //R
     par[5] = 1.0; //stp
     par[6] = hist->GetBinContent(bin-50);  //A
     par[7] = (hist->GetBinContent(bin-50) - hist->GetBinContent(bin+50))/100.;//B
     par[8] = -0.5;   //C
     par[9] = xp;  //bg offset
     goodfit = goodfit & FitPeak(par,hist,integral,sigma,centroid,verbosity);
   //  if(goodfit){ //DO STUFF HERE
  //   	areavec.push_back(integral/((intensvec.at(p)/100.0)*activitykBq*1000.0*runlengthsecs));
  //   	area_uncertainty.push_back(sigma);
         std::cout << "centroid is " << *centroid << std::endl;
        goodenergyvec.push_back(*centroid);
  //      goodintensvec.push_back(intensvec.at(p));
   //  }
  //   fitlist->Add(f);
   }

  // std::cout << "or made it here" << std::endl;

  // Float_t *area = &(areavec[0]);
   Double_t *energies = &(engvec[0]);
   Double_t *goodenergy = &(goodenergyvec[0]);

   TGraph* slopefit = new TGraph(nfound, goodenergy, energies);

   TFitResultPtr fitres = slopefit->Fit("pol1","SC0");
//   if(slopefit->GetFunction("pol1")->GetChisquare() > 10.) {
//      slopefit->RemovePoint(slopefit->GetN()-1);
//      slopefit->Fit("pol1");
//   }
   TChannel *chan = 0;
   //slopefit->Draw("AC*");
  

   if(cal_flag == false){
      TChannel::ReadCalFile("NewGrifCal.cal"); //This can be made better
      cal_flag = true;
   }

   chan = TChannel::GetChannelByNumber(channum);
   chan->DestroyENGCal();
   chan->AddENGCoefficient(fitres->Parameter(0));
   chan->AddENGCoefficient(fitres->Parameter(1));
	//chan->SetENGChi2(0);
	if(!goodfit)
	{
		chan->SetENGChi2(9999999);
	}
   delete centroid;
   return slopefit;
} 


TGraph* autogain152(TH1 *hist) {

   hist->GetXaxis()->SetRangeUser(200.,16000.);
   TSpectrum *s = new TSpectrum();
   Int_t nfound = s->Search(hist,6,"",0.08); //This will be dependent on the source used.
   printf("Found %d candidate peaks to fit\n",nfound);
   if(nfound > 3)
      nfound = 3;

   std::vector<float> vec;
   for(int x=0;x<nfound;x++)
      vec.push_back(s->GetPositionX()[x]);

   std::sort(vec.begin(),vec.end());

   Float_t energies[] = {121.7830, 244.6920, 344.276, 778.903, 964.131, 1408.011};
  // Float_t energies[]={778.908,964.131,1408.011};
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
     par[2] = 5./4.;   //sigma
     par[3] = 0.5;   //beta
     par[4] = 2;   //R
     par[5] = 1.0; //stp
     par[6] = hist->GetBinContent(bin-25);  //A
     par[7] = (hist->GetBinContent(bin-25) - hist->GetBinContent(bin+25))/100.;//B
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

int autoefficiency60(TH2D *mat,const char *name,Double_t runlengthsecs, Double_t activitykBq=1.0,bool verbosity = false) {
   TFile outfile("Calibration.root","RECREATE");
   TTree *tree = new TTree("CalibrationTree","Calibration Tree");
   TPeak *peak = 0;
   tree->Bronch("TPeak","TPeak", &peak);
     std::cout << &peak << std::endl;

   gSystem->Load("libNucleus");
   TNucleus nuc(name);
   nuc.SetSourceData();
   TMultiGraph *mg = new TMultiGraph("Efficiencies","Efficiencies");
   TGraphErrors *graph = new TGraphErrors;

   TH1D* h1 = new TH1D;
	for(int i=1; i<=64;i++){
      printf("\nNow fitting channel: %d",i);
		TH1D* h1 = (TH1D*) mat->ProjectionY(Form("Channel%d",i),i+1,i+1);
		if(h1->Integral() < 1)
			continue;
      peak = new TPeak;
      mg->Add(autoefficiency60(tree,peak,h1,i,&nuc, runlengthsecs,activitykBq,verbosity));
      delete peak;
   //   mg->Add(graph);
	}
   mg->Draw("PA0");
   tree->Write();
 
   outfile.Close();
  return 1;
}

int autoefficiency60(TH1D *hist,const char *name,Double_t runlengthsecs, Double_t activitykBq=1.0,bool verbosity = false) {
  gSystem->Load("libNucleus");
  TNucleus nuc(name);
  nuc.SetSourceData();
  TList *graphlist = new TList;
  TMultiGraph *mg_eff = new TMultiGraph("Efficiencies","Efficiencies");
 
  
  TGraphErrors *eff_1173 = new TGraphErrors("Eff_1173","Eff_1173"); graphlist->Add(eff_1173);
  TGraphErrors *eff_1332 = new TGraphErrors("Eff_1332","Eff_1332"); graphlist->Add(eff_1332);
  TGraphErrors *fwhm_1173 = new TGraphErrors("FWHM_1173","FWHM_1173"); graphlist->Add(fwhm_1173);
  TGraphErrors *fwhm_1332 = new TGraphErrors("FWHM_1332","FWHM_1332"); graphlist->Add(fwhm_1332);



     mg->Add(autoefficiency60(hist,0,&nuc, runlengthsecs,activitykBq,verbosity));
      mg->Add(graph);
   mg->Draw("PA0");
  return 1;
}


TMultiGraph* autoefficiency60(TTree *tree, TPeak *peak, TH1D *hist,int channum,TNucleus *nuc,Double_t runlengthsecs,  Double_t activitykBq,bool verbosity) {    //Activity in uCi

   if(!hist || !nuc)
      return 0;

   Double_t par[40];



 //  if(nuc->GetA() == 152) {
 //     return autogain152(hist);
 //  }

// Search
   hist->GetXaxis()->SetRangeUser(250,4000*4);

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
   std::vector<Float_t> channumvec;



   Float_t integral, sigma; 
   Double_t binWidth = hist->GetXaxis()->GetBinWidth(1000);
   Double_t dummyarray[4];

   std::cout << "bin width is " << binWidth << std::endl;
  for (int p=0;p<engvec.size();p++) {
     Float_t xp = engvec.at(p);
     std::cout << "Trying to fit " << xp << " keV" <<std::endl; 

     Int_t bin = hist->GetXaxis()->FindBin(xp);
     std::cout << "Bin is: " << bin << "xp is " << xp << std::endl;
     Float_t yp = hist->GetBinContent(bin);
     std::cout << "yp is: "<<yp << std::endl;
     par[0] = yp;  //height
     par[1] = xp;  //centroid
     par[2] = 1.0;   //sigma
     par[3] = 1.0;   //beta
     par[4] = 40.0;   //R
     par[5] = 1.0; //stp
     par[6] = hist->GetBinContent(xp-15);  //A
     par[7] = (hist->GetBinContent(xp-15) - hist->GetBinContent(xp+15))/100.*binWidth;//B
     par[8] = -0.5;   //C
     par[9] = xp;  //bg offset
     TFitResultPtr fitresult = FitPeak(par,hist,integral, sigma,dummyarray,verbosity);
     //peak = new TPeak;
     peak->Clear();
     std::cout << peak << std::endl;
     peak->SetCentroid(fitresult->Parameter(1), fitresult->ParError(1));
     peak->SetArea(integral,sigma);
     peak->SetFitResult(fitresult);
     peak->Print();
     tree->Fill();
     	areavec.push_back(integral/((intensvec.at(p)/100.0)*activitykBq*1000.0*runlengthsecs));
     	//area_uncertainty.push_back(sigma);
      area_uncertainty.push_back(0.01*integral/((intensvec.at(p)/100.0)*activitykBq*1000.0*runlengthsecs));
      std::cout<< "Eff = " << areavec.back() << "+/-" << area_uncertainty.back()<< std::endl;
      channumvec.push_back((Float_t)(channum));

 //    }
   }

  std::cout << "IM PRINTING!!!" << std::endl;
  std::cout << "Chi2 = " << fitresult->Chi2() << std::endl;
   std::cout << "or made it here" << std::endl;
   static Float_t eff1;
   static Float_t eff2;
   Float_t *area = &(areavec[0]);
 //  Float_t *energies = &(engvec[0]);
 //  float *goodenergy = &(goodenergyvec[0])
   Float_t *darea = &(area_uncertainty[0]);
   Float_t *chan = &(channumvec[0]);
  // hist->DrawCopy();
 //  TGraph *slopefit = new TGraph(areavec.size(),goodenergy,area ); 
   TGraphErrors *effic = new TGraphErrors(1,&chan[0],&area[0],0,&darea[0]);
   eff1 += area[0];
   eff2 += area[1];
   effic->SetMarkerStyle(21);
   effic->SetMarkerColor(kRed);
   effic->SetLineColor(kRed);
   TGraphErrors *effic2 = new TGraphErrors(1,&chan[1],&area[1],0,&darea[1]);
   effic2->SetMarkerStyle(20);
   effic2->SetMarkerColor(kBlue);
   effic2->SetLineColor(kBlue);
   TMultiGraph * efficmg = new TMultiGraph;
  // effic->Draw("PA0*");
   efficmg->Add(effic);
   efficmg->Add(effic2);

   std::cout << "Sum of eff so far: " <<eff1 << std::endl;
   std::cout << "Sum of eff2 so far: " << eff2 << std::endl;
   
  // printf("Now fitting: Be patient\n");
 //  slopefit->Fit("pol1");
  // slopefit->Draw("PA*");
 //  for(int x=0;x<areavec.size();x++) {
  //    printf("areavec[%i] = %f\t\tgoodenergyvec[%i] = %f\n",x,areavec[x],x,goodenergyvec[x]);
  // }

   //return slopefit;
   //delete dummyarray;
   return efficmg;

}

void gainwalk(){
   //This code is VERY SPECIFIC. It was hacked together as a test during the 32Na experiment and will hopefully
   //never ever ever ever ever ever be used again.

   //Loop over files first hists01596_000.root
   //
   Int_t highchan = 0;
   Double_t centroids[21][64];
   Double_t subrunarr[21];

   for(Int_t subrun = 0; subrun <= 0; subrun++){
      TFile *f = new TFile(Form("hists01596_0%02i.root",subrun));

      subrunarr[subrun] = subrun;
      //then loop over channels
      TString hname="hp_charge"; // From list of keys
      TH2D *mat = (TH2D*)f->Get(hname);

      TH1D* h1 = new TH1D;
      for(Int_t i = 0; i<highchan; i++)
      {  
         printf("\nNow fitting channel: %d",i);
		   TH1D* h1 = (TH1D*) mat->ProjectionY(Form("Channel%d",i+1),i+1,i+1);
         centroids[subrun][i] = gainwalk(h1);
      }
      f->Close();
      delete f;
      delete h1;
   }

   TFile* gainfile = new TFile("gainwalking.root","RECREATE");
   for(Int_t sub = 0; sub <= 0; sub++){
      for(Int_t chan = 0; chan < highchan; chan++){
         TGraph *g = new TGraph(21,subrunarr, centroids[sub]);
         g->SetName(Form("centroid_chan_%d",chan));
         g->SetTitle(Form("centroid_chan_%d",chan));
         g->Write();
         delete g;
      }

   }
   gainfile->Close();
   delete gainfile;
}


Double_t gainwalk(TH1* hist){
      
   Double_t par[40];

   hist->GetXaxis()->SetRangeUser(1000,1600);

   TSpectrum *s = new TSpectrum();
   Int_t nfound = s->Search(hist,3,"goff",0.5); //This will be dependent on the source used.
   
 //  printf("Found %d candidate peaks to fit\n",nfound);
 //  if(nfound > 1)
 //     nfound = 1;

   if(nfound <1){
      std::cout << "Did not find enough peaks" << std::endl;
      exit(1);
   }

   std::vector<Double_t> foundchan;
   printf("Found Candidate peaks at: ");
   for(int x=0;x<nfound;x++){
      Double_t tmppos = s->GetPositionX()[x];
      printf("%d ", tmppos);
      foundchan.push_back(tmppos);
   }

   std::vector<Float_t> areavec;
   std::vector<Float_t> area_uncertainty;
   std::vector<Float_t> goodenergyvec;
   std::vector<Float_t> goodintensvec;
   std::vector<Float_t> channumvec;

   Float_t integral, sigma; 
   Double_t binWidth = hist->GetXaxis()->GetBinWidth(1000);
   Double_t dummyarray[4];

   std::cout << "bin width is " << binWidth << std::endl;
   Float_t xp = s->GetPositionX()[0];
   std::cout << "Trying to fit " << xp << " Channel" <<std::endl; 

   Int_t bin = hist->GetXaxis()->FindBin(xp);
   std::cout << "Bin is: " << bin << "xp is " << xp << std::endl;
   Float_t yp = hist->GetBinContent(bin);
   std::cout << "yp is: "<<yp << std::endl;
   par[0] = yp;  //height
   par[1] = xp;  //centroid
   par[2] = 1.0;   //sigma
   par[3] = 1.0;   //beta
   par[4] = 40.0;   //R
   par[5] = 1.0; //stp
   par[6] = hist->GetBinContent(xp-15);  //A
   par[7] = (hist->GetBinContent(xp-15) - hist->GetBinContent(xp+15))/100.*binWidth;//B
   par[8] = -0.5;   //C
   par[9] = xp;  //bg offset
   TFitResultPtr fitresult = FitPeak(par,hist,integral, sigma,dummyarray,verbosity);
   //peak = new TPeak;
 //  peak->Clear();
 //  std::cout << peak << std::endl;
 //  peak->SetCentroid(fitresult->Parameter(1), fitresult->ParError(1));
 //  peak->SetArea(integral,sigma);
 //  peak->SetFitResult(fitresult);
 //  peak->Print();
 //  tree->Fill();
 //  areavec.push_back(integral/((intensvec.at(p)/100.0)*activitykBq*1000.0*runlengthsecs));
   //area_uncertainty.push_back(sigma);
 //  area_uncertainty.push_back(0.01*integral/((intensvec.at(p)/100.0)*activitykBq*1000.0*runlengthsecs));
 //  std::cout<< "Eff = " << areavec.back() << "+/-" << area_uncertainty.back()<< std::endl;
 //  channumvec.push_back((Float_t)(channum));

 //    }

  std::cout << "IM PRINTING!!!" << std::endl;
  std::cout << "Chi2 = " << fitresult->Chi2() << std::endl;
//   static Float_t eff1;
//   static Float_t eff2;
//   Float_t *area = &(areavec[0]);
 //  Float_t *energies = &(engvec[0]);
 //  float *goodenergy = &(goodenergyvec[0])
//   Float_t *darea = &(area_uncertainty[0]);
//   Float_t *chan = &(channumvec[0]);
  // hist->DrawCopy();
 //  TGraph *slopefit = new TGraph(areavec.size(),goodenergy,area ); 
/*   TGraphErrors *effic = new TGraphErrors(1,&chan[0],&area[0],0,&darea[0]);
   eff1 += area[0];
   eff2 += area[1];
   effic->SetMarkerStyle(21);
   effic->SetMarkerColor(kRed);
   effic->SetLineColor(kRed);
   TGraphErrors *effic2 = new TGraphErrors(1,&chan[1],&area[1],0,&darea[1]);
   effic2->SetMarkerStyle(20);
   effic2->SetMarkerColor(kBlue);
   effic2->SetLineColor(kBlue);
   TMultiGraph * efficmg = new TMultiGraph;
  // effic->Draw("PA0*");
   efficmg->Add(effic);
   efficmg->Add(effic2);
*/
 //  std::cout << "Sum of eff so far: " <<eff1 << std::endl;
 //  std::cout << "Sum of eff2 so far: " << eff2 << std::endl;
   
  // printf("Now fitting: Be patient\n");
 //  slopefit->Fit("pol1");
  // slopefit->Draw("PA*");
 //  for(int x=0;x<areavec.size();x++) {
  //    printf("areavec[%i] = %f\t\tgoodenergyvec[%i] = %f\n",x,areavec[x],x,goodenergyvec[x]);
  // }

   //return slopefit;
   //delete dummyarray;
   return fitresult->Parameter(1);

}


