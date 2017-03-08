//g++ AngularCorrelations.C -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries -lGRSIFormat `root-config --cflags --libs` -lTreePlayer -lNucleus -o AngularCorrelations


#include <map>
#include <vector>
#include <sys/stat.h>

#include "TROOT.h"
#include "TSystem.h"
#include "TFile.h"
#include "TList.h"
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
#include "TROOT.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TH3F.h"

#include "TNucleus.h"
#include "TGRSITransition.h"

Double_t BestFitEver(Double_t *dim, Double_t *par);
TFitResultPtr FitPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, bool verbosity = false);

TList* AngularCorrelations(TH1D** slice, TNucleus* nuc, bool verbosity) {
   gSystem->Load("libMathMore");
   if(!nuc)
      return nullptr;
   Double_t counter = 0;
   Double_t par[40];

   Float_t  Counts[51]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
   Float_t cosTheta[51]     = { 0.944771731934288,    0.904566789892864,    0.889543984431678,    0.849340400263317,    0.837089096669585,    0.715192775009554,    0.686998737834932,    0.659962330958193,    0.647707774342544,    0.592153830802231,    0.497625071673957,    0.458339340618090,    0.447712269418652,    0.419531299378301,    0.392481596985632,    0.389943759514329,    0.350648738728823,    0.324676879730513,    0.295424600236195,    0.230151646979774,    0.200582087822817,    0.122475632570626,    0.0669008244293115,    0.0571981117143259,    0.0276076005714703,    -0.0276076005714704,    -0.0571981117143258,    -0.0669008244293116,    -0.122475632570626,    -0.200564989204778,    -0.230219585438792,    -0.295374576981412,    -0.324742909980579,    -0.350697773642815,    -0.389927687788188,    -0.392497649763699,    -0.419610513115510,    -0.447759087838770,    -0.458339340618090,    -0.497579654440663,    -0.592153830802231,    -0.647721071713080,    -0.660001667960937,    -0.686960689137425,    -0.715204973482290,    -0.837050902177404,    -0.849340400263317,    -0.889575876378338,    -0.904603990928734,    -0.944777451742548,    -1};

   Float_t err_cosTheta[51] = {0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083,0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083, 0.083};
   Float_t weights[51] = {128,64,64,64,48,128,96,128,96,48,96,48,64,96,64,64,64,96,64,96,64,64,64,48,128,128,48,64,64,64,96,64,96,64,64,64,96,64,48,96,48,96,128,96,128,48,64,64,64,128,64}; 
	
   Float_t err_counts[51]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  
   Float_t err_angles[51]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   Float_t angles[51]       = {19.131, 25.235, 27.184, 31.860, 33.166, 44.341, 46.607, 48.703, 49.631, 53.690, 60.157, 62.720, 63.403, 65.195, 66.891, 67.049, 69.473, 71.054, 72.817, 76.694, 78.429, 82.965, 86.164, 86.721, 88.418, 91.582, 93.279, 93.836, 97.035, 101.57, 103.31, 107.18, 108.95, 110.53, 112.95, 113.11, 114.81, 116.60, 117.28, 119.84, 126.31, 130.37, 131.30, 133.39, 135.66, 146.83, 148.14, 152.82, 154.77, 160.87, 180.00};

   
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
   Double_t binWidth = slice[0]->GetXaxis()->GetBinWidth(1000);
   Double_t dummyarray[4];

   TList* list = new TList;

   double min = 1e9;
   for(int i = 0; i<51;i++){
      slice[i]->GetXaxis()->SetRangeUser(250,2500);

      if(slice[i]->Integral(250,2500) <= 0.) {
         std::cout<<"Slice '"<<slice[i]->GetName()<<"' has no entries in range 250-2500!"<<std::endl;
         list->Add(slice[i]);
         continue;
      }
      for (int p=0;p<engvec.size();p++) {
         Float_t xp = engvec.at(p);
    
         Int_t bin = slice[i]->GetXaxis()->FindBin(xp);
         Float_t yp = slice[i]->GetBinContent(bin);
     
         par[0] = yp;  //height
         par[1] = xp;  //centroid
         par[2] = 1.0;   //sigma
         par[3] = 1.0;   //beta
         par[4] = 40.0;   //R
         par[5] = 1.0; //stp
         par[6] = slice[i]->GetBinContent(xp-15);  //A
         par[7] = (slice[i]->GetBinContent(xp-15) - slice[i]->GetBinContent(xp+15))/100.*binWidth;//B
         par[8] = -0.5;   //C
         par[9] = xp;  //bg offset
         if(xp <1200) {
            try {
	            TFitResultPtr fitresult = FitPeak(par,slice[i],integral, sigma,dummyarray,verbosity);
            } catch(std::exception& exc) {
               std::cout<<"FitPeak failed with exception "<<exc.what()<<std::endl;
            }
         }
      }
      list->Add(slice[i]);
      counter = counter + 1;
      Counts[i] = integral/weights[i];
      if(Counts[i] < min) {
         min = Counts[i];
      }
      err_counts[i] = sqrt(Counts[i]*weights[i])/weights[i];
      //printf("angle = %f weight = %f \n",angles[i],weights[i]);
      printf(" %f \t %f %f \t %f %f \n",integral, Counts[i], err_counts[i], cosTheta[i], err_cosTheta[i]);
      //printf("angle = %f weight = %f \n",angles[i],weights[i]);
   }     

   TF1 *Fit = new TF1("Correlation_Fit",BestFitEver,-1.,1.,3);

   TGraphErrors *gr = new TGraphErrors(counter,cosTheta,Counts,err_cosTheta,err_counts);
   gr->SetTitle("Gamma-Gamma-Angular-Correlation");
   gr->GetXaxis()->SetTitle("cos theta");
   gr->GetYaxis()->SetTitle("normalized counts");

   Fit->SetParameters(min, Counts[0]-min, 0.01);
   //Fit->FixParameter(2,0.);
   //TF1 *Start = Fit->Clone("Start");
   //Start->SetLineStyle(2);

   gr->Fit(Fit,"RME");
   gr->SetMarkerColor(4);
   gr->SetMarkerStyle(21);
   Fit->SetLineColor(2);
   Fit->SetLineWidth(1);
   Fit->SetLineStyle(3);

   //gr->GetListOfFunctions()->Add(Start);

   printf("chi square = %f, NDF = %d => reduced chi square = %f\n",Fit->GetChisquare(), Fit->GetNDF(), Fit->GetChisquare()/Fit->GetNDF());
   if(Fit->GetParError(2) > Fit->GetParameter(2)) {
      printf("The error on a4 is larger than it's value. If you got a lot of error messages above this, you might want to fix a4 to a reasonable value.\n");
   }

#ifdef __CINT__ 
   gr->Draw("PA");
#endif
   list->Add(gr);
 
   return list;
}

TList* AngularCorrelations(TH1D** slice, const char *name,bool verbosity = false) {
   gSystem->Load("libNucleus");
   TNucleus nuc(name);
   nuc.SetSourceData();
  
   return AngularCorrelations(slice, &nuc,verbosity);
}
                  

Double_t BestFitEver(Double_t *dim, Double_t *par){
   return par[0] + par[1]/2.*(3.*dim[0]*dim[0] -1.) + par[2]/8.*(35.*dim[0]*dim[0]*dim[0]*dim[0]-30.*dim[0]*dim[0]+3.);
}

/////Ryans Peak fitting function
//Double_t fpeaks(Double_t *x, Double_t *par) {
//   Double_t result = par[0] + par[1]*x[0];
//   for (Int_t p=0;p<npeaks;p++) {
//      Double_t norm  = par[3*p+2];
//      Double_t mean  = par[3*p+3];
//      Double_t sigma = par[3*p+4];
//      result += norm*TMath::Gaus(x[0],mean,sigma);
//   }
//   return result;
//}

Double_t background(Double_t *x, Double_t *par) {
   return par[0] + par[1]*(x[0]-par[3]) + par[2]*(x[0]-par[3])*(x[0]-par[3]);
}

Double_t photo_peak(Double_t *dim, Double_t *par){
   Double_t x        = dim[0]; //channel number used for fitting 
   Double_t height   = par[0]; //height of photopeak
   Double_t c        = par[1]; //Peak Centroid of non skew gaus
   Double_t sigma    = par[2]; //standard deviation  of gaussian
   Double_t beta     = par[3]; //"skewedness" of the skewed gaussian
   Double_t R        = par[4]; //relative height of skewed gaussian

   Double_t gaussian    = height*(1.0-R/100.0)*TMath::Gaus(x,c,sigma);
   Double_t skewed_gaus = R*height/100.0*(TMath::Exp((x-c)/beta))*(TMath::Erfc(((x-c)/(TMath::Sqrt(2.0)*sigma)) + sigma/(TMath::Sqrt(2.0)*beta)));

   return gaussian + skewed_gaus;
}

Double_t step_function(Double_t *dim, Double_t *par){
   Double_t x       = dim[0];
   Double_t height  = par[0];
   Double_t c       = par[1];
   Double_t sigma   = par[2];
   Double_t step    = par[5];

  return TMath::Abs(step)*height/100.0*TMath::Erfc((x-c)/(TMath::Sqrt(2.0)*sigma));
}

Double_t fitFunction(Double_t *dim, Double_t *par){
   return photo_peak(dim, par) + step_function(dim,par) + background(dim,&par[6]);
}


TFitResultPtr FitPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, bool verbosity) {
   Double_t binWidth = h->GetXaxis()->GetBinWidth(par[1]);//Need to find the bin widths so that the integral makes sense
   Int_t rw = binWidth*100;  //This number may change depending on the source used   
   //Set the number of iterations. The code is pretty quick, so having a lot isn't an issue 
   TVirtualFitter::SetMaxIterations(10000);
   Int_t xp = par[1];
   Int_t yp = par[0];
  // std::cout << "Now yp is: "<<yp << std::endl;
  // std::cout << "A is: " << par[6] <<std::endl;
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
   //     //Fixing has to come after setting
   pp->FixParameter(8,0);

   const char * options;
   if(verbosity)
      options = "RFSM+";
   else
      options = "RFSMQ+";

   pp->SetNpx(1000); //Draws a nice smooth function on the graph
   TFitResultPtr fitres = h->Fit("photopeak",options);

   if(fitres->ParError(2) != fitres->ParError(2)){ //Check to see if nan
      if(fitres->Parameter(3) < 1){
         pp->FixParameter(4,0);
         pp->FixParameter(3,1);
    //     std::cout << "Beta may have broken the fit, retrying with R=0" << std::endl;
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

   TF1 *photopeak = new TF1("photopeak",photo_peak,xp-rw,xp+rw,10);
   photopeak->SetParameters(par);

   Double_t integral = photopeak->Integral(xp-rw,xp+rw)/binWidth;

   if(verbosity){
      std::cout << "FIT RESULT CHI2 " << fitres->Chi2() << std::endl;
      std::cout << "FWHM = " << 2.35482*fitres->Parameter(2) <<"(" << fitres->ParError(2) << ")" << std::endl;
      std::cout << "NDF: " << fitres->Ndf() << std::endl;
   }
  // std::cout << "X sq./v = " << fitres->Chi2()/fitres->Ndf() << std::endl;

   TVirtualFitter *fitter = TVirtualFitter::GetFitter();

   //assert(fitter != 0); //make sure something was actually fit
   if(fitter == 0) {
      std::cout<<"Fitting failed!"<<std::endl;
      return fitres;
   }

   TMatrixDSym covMatrix = fitres->GetCovarianceMatrix(); //This allows us to find the uncertainty in the integral

   Double_t sigma_integral = photopeak->IntegralError(xp-rw,xp+rw)/binWidth;

   //std::cout << "Integral = " << integral << " +/- " << sigma_integral << std::endl;

   area = integral;
   darea = sigma_integral;
   *energy = fitres->Parameter(1);

   delete photopeak;
   delete pp;

   return fitres;
   return integral;
}

#ifndef __CINT__ 

int main(int argc, char **argv) {
   if(argc != 3 && argc != 2) {
      printf("try again (usage: %s <analysis tree file> <optional: output file>).\n",argv[0]);
      return 0;
   }

   std::string fileName;
   if(argc == 2) {
      fileName = argv[1];
      if(fileName.find_last_of("/") != std::string::npos) {
         fileName.insert(fileName.find_last_of("/")+1,"correlation_");
      } else {
         fileName.insert(0,"correlation_");
      }
   } else {
      fileName = argv[2];
   }

   struct stat fileInfo; 
   if(stat(fileName.c_str(),&fileInfo) == 0) {
      printf("File '%s' already exists, please remove it before re-running %s!\n",fileName.c_str(),argv[0]);
      return 1;
   }
  
   TFile* file = new TFile(argv[1]);
   if(file == nullptr) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }
   if(!file->IsOpen()) {
      printf("Failed to open file '%s'!\n",argv[1]);
      return 1;
   }

   TH3F* hist = (TH3F*) file->Get("angCorr");

   TList *list = nullptr;
   TH1D* slice[51];
   if(hist == nullptr) {
      for(int i = 0; i < 51;i++){
         slice[i] =(TH1D*) file->Get(Form("Slice_X_%i",i+1));
         if(slice[i] == nullptr) {
            std::cout<<"Failed to find histogram 'angCorr' or slice 'Slice_X_"<<i<<"' in file "<<file->GetName()<<std::endl;
            return 1;
         }
      }
   } else {
      for(int i = 0; i < 51; i++){
         slice[i] = hist->ProjectionX(Form("Slice_X_%d",i+1),hist->GetYaxis()->FindBin(1330.5),hist->GetYaxis()->FindBin(1334.5),i+1,i+1);
      }
   }
   list = AngularCorrelations(slice, "60Co");

   if(list == nullptr) {
      std::cout<<"Failed to get TList"<<std::endl;
      return 1;
   }

   TFile *outfile = new TFile(fileName.c_str(),"create");
   list->Write();
   outfile->Close();

   return 0;
}

#endif
