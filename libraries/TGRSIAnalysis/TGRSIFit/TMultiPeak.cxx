#include "TMultiPeak.h"


#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"


/// \cond CLASSIMP
ClassImp(TMultiPeak)
/// \endcond

Bool_t TMultiPeak::fLogLikelihoodFlag = false;


TMultiPeak::TMultiPeak(Double_t xlow, Double_t xhigh, const std::vector<Double_t> &centroids, Option_t *opt) : TGRSIFit("multipeakbg",MultiPhotoPeakBG,xlow,xhigh,centroids.size()*6 +5){
   this->Clear();
   //We make the background first so we can send it to the TPeaks.
   fBackground = new TF1(Form("MPbackground_%d_to_%d",(Int_t)(xlow),(Int_t)(xhigh)),MultiStepBG,xlow,xhigh,centroids.size()*6+5);
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground,kFALSE);

   for(size_t i=0; i<centroids.size(); i++){
      Bool_t out_of_range_flag = false;
      Double_t cent = centroids.at(i);
      if(cent > xhigh){
         printf("centroid %lf is higher than range\n",cent);
         out_of_range_flag = true;
      }
      else if (cent < xlow){
         printf("centroid %lf is lower than range\n", cent);
         out_of_range_flag = true;
      }
      if(out_of_range_flag){
         printf("ignoring peak at %lf, make a new multi peak with the corrected energy\n",cent);
      }
      else{
         TPeak* peak = new TPeak(cent,xlow,xhigh,fBackground);
         peak->AddToGlobalList(kFALSE);
         fPeakVec.push_back(peak);
      }
   }
   this->SetRange(xlow,xhigh);

   this->SetName(Form("MultiPeak_%d_to_%d",(Int_t)(xlow),(Int_t)(xhigh))); //Gives a default name to the peak
   this->SortPeaks();//Defaults to sorting by TPeak::CompareEnergy
   this->InitNames();

}

TMultiPeak::TMultiPeak() : TGRSIFit("multipeakbg",MultiPhotoPeakBG,0,1000,10){
   //I don't think this constructor should be used, RD.
   this->InitNames();
   fBackground = new TF1("background",MultiStepBG,1000,10);//This is a weird nonsense line.
   fBackground->SetNpx(1000);
   fBackground->SetLineStyle(2);
   fBackground->SetLineColor(kBlack);
   TGRSIFit::AddToGlobalList(fBackground,kFALSE);
}

TMultiPeak::~TMultiPeak(){
   if(fBackground){ 
      delete fBackground;
   }
   
   for(size_t i=0; i<fPeakVec.size(); ++i){
      if(fPeakVec.at(i)){
         delete fPeakVec.at(i);
      }
   }

}

void TMultiPeak::SortPeaks(Bool_t (*SortFunction)(const TPeak* ,const TPeak* )) { 
   std::sort( fPeakVec.begin(), fPeakVec.end(), SortFunction);
}


void TMultiPeak::InitNames(){
   this->SetParName(0,"N_Peaks");
   this->SetParName(1,"A");
   this->SetParName(2,"B");
   this->SetParName(3,"C");
   this->SetParName(4,"bg_offset");

   for(int i=0; i<(int)fPeakVec.size();++i){
      this->SetParName(6*i+5,Form("Height_%i",i));
      this->SetParName(6*i+6,Form("Centroid_%i",i));
      this->SetParName(6*i+7,Form("Sigma_%i",i));
      this->SetParName(6*i+8,Form("Beta_%i",i));
      this->SetParName(6*i+9,Form("R_%i",i));
      this->SetParName(6*i+10,Form("Step_%i",i));
   }
   this->FixParameter(0,fPeakVec.size());
}


TMultiPeak::TMultiPeak(const TMultiPeak& copy) : TGRSIFit(), fBackground(0) {
   copy.Copy(*this);
}

void TMultiPeak::Copy(TObject& obj) const {
   TGRSIFit::Copy(obj);
   TMultiPeak* mpobj = static_cast<TMultiPeak*>(&obj);
   if(!(mpobj->fBackground)) 
      mpobj->fBackground = new TF1(*(fBackground));
   else
      *(mpobj->fBackground) = *fBackground;

   TGRSIFit::AddToGlobalList(fBackground,kFALSE);

   //Copy all of the TPeaks.
   for(size_t i=0; i<fPeakVec.size();++i){
      TPeak* peak = new TPeak(*(fPeakVec.at(i)));
      peak->AddToGlobalList(kFALSE);
      mpobj->fPeakVec.push_back(peak);
   }

}

Bool_t TMultiPeak::InitParams(TH1 *fithist){
//Makes initial guesses at parameters for the fit. Uses the histogram to make the initial guesses
   if(!fithist && GetHist()) 
      fithist = GetHist();

   if(!fithist){
      printf("No histogram is associated yet, no initial guesses made\n");
      return false;
   }

   this->FixParameter(0,fPeakVec.size());
   //This is the range for the fit.
   Double_t xlow,xhigh;
   GetRange(xlow,xhigh);
   Int_t binlow = fithist->GetXaxis()->FindBin(xlow);
   Int_t binhigh = fithist->GetXaxis()->FindBin(xhigh);

   //Initialize background
   this->SetParLimits(1,0.0,fithist->GetBinContent(binlow)*100.0);
   this->SetParameter("A",fithist->GetBinContent(binlow));
   this->SetParameter("B",(fithist->GetBinContent(binlow) - fithist->GetBinContent(binhigh))/(xlow-xhigh));
   this->SetParameter("C",0.0000);
   this->SetParameter("bg_offset",(xhigh+xlow)/2.0);

   this->FixParameter(3,0);

   //We need to initialize parameters for every peak in the fit
   for(int i=0; i<(int)fPeakVec.size();++i){
      Double_t centroid = fPeakVec.at(i)->GetCentroid();
      Int_t bin = fithist->GetXaxis()->FindBin(centroid);
      this->SetParLimits(6*i+5,0,fithist->GetBinContent(bin)*5.);
      this->SetParLimits(6*i+6,centroid-4,centroid+4);
      this->SetParLimits(6*i+7,0.1,xhigh-xlow);//This will be linked to other peaks eventually.
      this->SetParLimits(6*i+8,0.000001,10);
      this->SetParLimits(6*i+9,0.000001,100);
      this->SetParLimits(6*i+10,0.0,1.0E2);
      //Step size is allow to vary to anything. If it goes below 0, the code will fix it to 0

      //Now set the actual paramter to start the fit from these points
      this->SetParameter(Form("Height_%i",i),fithist->GetBinContent(bin));
      this->SetParameter(Form("Centroid_%i",i),centroid);
      //  this->SetParameter("sigma",(xhigh-xlow)*0.5); // slightly more robust starting value for sigma -JKS
      //  this->SetParameter("sigma",1.0/binWidth); // slightly more robust starting value for sigma -JKS
      this->SetParameter(Form("Sigma_%i",i),TMath::Sqrt(9.0 + 4.*GetParameter(Form("Centroid_%i",i))/1000.));
      this->SetParameter(Form("Beta_%i",i),GetParameter(Form("Sigma_%i",i))/2.0);
      this->SetParameter(Form("R_%i",i), 1.0);
      this->SetParameter(Form("Step_%i",i),1.0);

      //Fix beta and R. These will be released if they are needed (or can be asked to be released).
      this->FixParameter(6*i+8,GetParameter(Form("Beta_%i",i)));
      this->FixParameter(6*i+9,0.00);

   }

   SetInitialized();
   return true;
}

Bool_t TMultiPeak::Fit(TH1* fithist,Option_t *opt){
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

   TVirtualFitter::SetMaxIterations(100000);
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Combination");
   SetHist(fithist);

   TString options(opt); bool print_flag = true;
   if(options.Contains("Q"))
     print_flag = false;

   //Now that it is initialized, let's fit it.

   TFitResultPtr fitres;
   //Log likelihood is the proper fitting technique UNLESS the data is a result of an addition or subtraction.
   if(GetLogLikelihoodFlag()){
      fitres = fithist->Fit(this,Form("%sLRS",opt));//The RS needs to always be there
   }
   else{
      fitres = fithist->Fit(this,Form("%sRS",opt));//The RS needs to always be there
   }

   //After performing this fit I want to put something here that takes the fit result (good,bad,etc)
   //for printing out. RD

   //Int_t fitStatus = fitres; //This returns a fit status from the TFitResult Ptr
   //This removes the background parts of the fit form the integral error, while maintaining the covariance between the fits and the background.
   TMatrixDSym CovMat = fitres->GetCovarianceMatrix();
   CovMat(0,0) = 0.0;
   CovMat(1,1) = 0.0;
   CovMat(2,2) = 0.0;
   CovMat(3,3) = 0.0;
   CovMat(4,4) = 0.0;
//   printf("covmat ");CovMat.Print();

   //This copies the parameters background but the background function doesn't have peaks
   this->CopyParameters(fBackground);
   //We now make a copy of the covariance matrix that has completel 0 diagonals so that we can remove the other peaks form the integral error.
   //This is done by adding back the peak of interest on the diagonal when it is integrated.
   TMatrixDSym emptyCovMat = CovMat;
   for(size_t i =0; i<fPeakVec.size();++i){
      emptyCovMat(6*i+5,6*i+5) = 0.0;
      emptyCovMat(6*i+6,6*i+6) = 0.0;
      emptyCovMat(6*i+7,6*i+7) = 0.0;
      emptyCovMat(6*i+8,6*i+8) = 0.0;
      emptyCovMat(6*i+9,6*i+9) = 0.0;
      emptyCovMat(6*i+10,6*i+10) = 0.0;
 //  printf("empt covmat ");emptyCovMat.Print();
   }

/*
   if(fitres->ParError(2) != fitres->ParError(2)){ //Check to see if nan
      if(fitres->Parameter(3) < 1){
         InitParams(fithist);
         FixParameter(4,0);
         FixParameter(3,1);
         std::cout << "Beta may have broken the fit, retrying with R=0" << std::endl;
   	 // Leaving the log-likelihood argument out so users are not constrained to just using that. - JKS
         fithist->GetListOfFunctions()->Last()->Delete();
         if(GetLogLikelihoodFlag()){
            fitres = fithist->Fit(this,Form("%sLRS",opt));//The RS needs to always be there
         }
         else{
            fitres = fithist->Fit(this,Form("%sRS",opt));
         }
      }
   }*/
/*   if(fitres->Parameter(5) < 0.0){
      FixParameter(5,0);
      std::cout << "Step < 0. Retrying fit with stp = 0" << std::endl;
      fitres = fithist->Fit(this,Form("%sRSML",opt));
   }
*/
   /*
*/
   if(print_flag) printf("Chi^2/NDF = %lf\n",fitres->Chi2()/fitres->Ndf());
   //We will now set the parameters of each of the peaks based on the fits.
   for(int i=0; i< (int)fPeakVec.size();++i){
      TMultiPeak *tmpMp = new TMultiPeak(*this);
      tmpMp->ClearParameters();//We need to clear all of the parameters so that we can add the ones we want back in
      Double_t binWidth = fithist->GetBinWidth(GetParameter(Form("Centroid_%i",i)));
      TPeak *peak = fPeakVec.at(i);
      TMatrixDSym tmpCovMat = emptyCovMat;
      peak->SetParameter("Height",GetParameter(Form("Height_%i",i)));
      peak->SetParameter("centroid",GetParameter(Form("Centroid_%i",i)));
      peak->SetParameter("sigma",GetParameter(Form("Sigma_%i",i)));
      peak->SetParameter("beta",GetParameter(Form("Beta_%i",i)));
      peak->SetParameter("R",GetParameter(Form("R_%i",i)));
      peak->SetParameter("step",GetParameter(Form("Step_%i",i)));
      peak->SetParameter("A",0.0);
      peak->SetParameter("B",0.0);
      peak->SetParameter("C",0.0);
      peak->SetParameter("bg_offset",0.0);
      peak->SetChi2(fitres->Chi2());  
      peak->SetNdf(fitres->Ndf());

     // printf("tmp cov mat: ");tmpCovMat.Print();

      //Set the important diagonals for the integral of the covariance matrix
      tmpCovMat(6*i+5,6*i+5) = CovMat(6*i+5,6*i+5);
      tmpCovMat(6*i+6,6*i+6) = CovMat(6*i+6,6*i+6);
      tmpCovMat(6*i+7,6*i+7) = CovMat(6*i+7,6*i+7);
      tmpCovMat(6*i+8,6*i+8) = CovMat(6*i+8,6*i+8);
      tmpCovMat(6*i+9,6*i+9) = CovMat(6*i+9,6*i+9);
      
      tmpMp->SetParameter("N_Peaks",GetParameter("N_Peaks"));
      tmpMp->SetParameter(Form("Height_%i",i),GetParameter(Form("Height_%i",i)));
      tmpMp->SetParameter(Form("Centroid_%i",i),GetParameter(Form("Centroid_%i",i)));
      tmpMp->SetParameter(Form("Sigma_%i",i),GetParameter(Form("Sigma_%i",i)));
      tmpMp->SetParameter(Form("Beta_%i",i),GetParameter(Form("Beta_%i",i)));
      tmpMp->SetParameter(Form("R_%i",i),GetParameter(Form("R_%i",i)));
   
      Double_t width = this->GetParameter(Form("Sigma_%i",i));
      Double_t xlow,xhigh;
      Double_t int_low, int_high; 
      this->GetRange(xlow,xhigh);
      int_low = xlow - 10.*width; // making the integration bounds a bit smaller, but still large enough. -JKS
      int_high = xhigh + 10.*width;

      //Make a function that does not include the background
      //Intgrate the background.
      tmpMp->SetRange(int_low,int_high);//This will help get the true area of the gaussian 200 ~ infinity in a gaus
   //   peak->SetName("tmppeak");
      
      //This is where we will do integrals and stuff.
      peak->SetArea((tmpMp->Integral(int_low,int_high))/binWidth);
      peak->SetAreaErr((tmpMp->IntegralError(int_low,int_high,tmpMp->GetParameters(),tmpCovMat.GetMatrixArray())) /binWidth);
      peak->SetParameter("centroid",GetParameter(Form("Centroid_%i",i)));
      peak->SetParError(peak->GetParNumber("centroid"),GetParError(GetParNumber(Form("Centroid_%i",i))));
      peak->SetParameter("sigma",GetParameter(GetParNumber(Form("Sigma_%i",i))));
      peak->SetParError(peak->GetParNumber("sigma"),GetParError(GetParNumber(Form("Sigma_%i",i))));
   if(print_flag) printf("Integral: %lf +/- %lf\n",peak->GetArea(),peak->GetAreaErr());
   }

   //Set the background for drawing later
 //  background->SetParameters(this->GetParameters());
  //To DO: put a flag in signalling that the errors are not to be trusted if we have a bad cov matrix
  // Copy(*fithist->GetListOfFunctions()->Last());
  // if(optstr.Contains("+"))
  //    Copy(*fithist->GetListOfFunctions()->Before(fithist->GetListOfFunctions()->Last()));
  
 //     peak->SetParameter("step",GetParameter(Form("Step_%i",i)));
//   delete tmppeak;
   return true;
}


void TMultiPeak::Clear(Option_t* opt){
   TGRSIFit::Clear(opt);
   for(size_t i =0; i<fPeakVec.size(); ++i){
      if(fPeakVec.at(i)){
         delete fPeakVec.at(i);
         fPeakVec.at(i) = 0;
      }
   }
   fPeakVec.clear();

}

void TMultiPeak::Print(Option_t *opt) const {
//Prints TMultiPeak properties. To see More properties use the option "+"
   printf("Name:        %s \n", this->GetName()); 
   printf("Number of Peaks: %lu\n",fPeakVec.size());
   TF1::Print();
   for(int i=0; i<(int)fPeakVec.size(); ++i){
      printf("Peak: %i\n",i);
      fPeakVec.at(i)->Print(opt);
      printf("\n");
   }
}

const char * TMultiPeak::PrintString(Option_t *opt) const {
//Prints TMultiPeak properties to a string, returns the string.
   /*
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
   //   TGRSIFit::Print(opt); //Polymorphise this a bit better
   //}
   return temp.c_str();*/
   return "b";
}


Double_t TMultiPeak::MultiPhotoPeakBG(Double_t *dim, Double_t *par) {
  // Limits need to be imposed or error states may occour.
  //
   //General background.
   int npeaks = (int)(par[0]+0.5);
	double result = TGRSIFunctions::PolyBg(dim,&par[1],2); // polynomial background. uses par[1->4]
	for(int i=0;i<npeaks;i++){// par[0] is number of peaks
		Double_t tmp_par[6];
  	   tmp_par[0]   = par[6*i+5]; //height of photopeak
  	   tmp_par[1]   = par[6*i+6]; //Peak Centroid of non skew gaus
  	   tmp_par[2]   = par[6*i+7]; //standard deviation  of gaussian
  	   tmp_par[3]   = par[6*i+8]; //"skewedness" of the skewed gaussian
  	   tmp_par[4]   = par[6*i+9]; //relative height of skewed gaussian
      tmp_par[5]   = par[6*i+10]; //Size of step in background
		result += TGRSIFunctions::PhotoPeak(dim,tmp_par) + TGRSIFunctions::StepFunction(dim,tmp_par);
	}
	return result;
}

Double_t TMultiPeak::MultiStepBG(Double_t *dim, Double_t *par) {
  // Limits need to be imposed or error states may occour.
  //
   //General background.
   int npeaks = (int)(par[0]+0.5);
	double result = TGRSIFunctions::PolyBg(dim,&par[1],2); // polynomial background. uses par[1->4]
	for(int i=0;i<npeaks;i++){// par[0] is number of peaks
		Double_t tmp_par[6];
  	   tmp_par[0]   = par[6*i+5]; //height of photopeak
  	   tmp_par[1]   = par[6*i+6]; //Peak Centroid of non skew gaus
  	   tmp_par[2]   = par[6*i+7]; //standard deviation  of gaussian
  	   tmp_par[3]   = par[6*i+8]; //"skewedness" of the skewed gaussian
  	   tmp_par[4]   = par[6*i+9]; //relative height of skewed gaussian
      tmp_par[5]   = par[6*i+10]; //Size of step in background
		result += TGRSIFunctions::StepFunction(dim,tmp_par);
	}
	return result;
}

Double_t TMultiPeak::SinglePeakBG(Double_t *dim, Double_t *par) {
  // Limits need to be imposed or error states may occour.
  //
   //General background.
   
   int npeaks = (int)(par[0]+0.5);
	double result = TGRSIFunctions::PolyBg(dim,&par[1],2); // polynomial background. uses par[1->4]
	for(int i=0;i<npeaks;i++){// par[0] is number of peaks
		Double_t tmp_par[6];
  	   tmp_par[0]   = par[6*i+5]; //height of photopeak
  	   tmp_par[1]   = par[6*i+6]; //Peak Centroid of non skew gaus
  	   tmp_par[2]   = par[6*i+7]; //standard deviation  of gaussian
  	   tmp_par[3]   = par[6*i+8]; //"skewedness" of the skewed gaussian
  	   tmp_par[4]   = par[6*i+9]; //relative height of skewed gaussian
      tmp_par[5]   = par[6*i+10]; //Size of step in background
		result += TGRSIFunctions::StepFunction(dim,tmp_par);
	}
   result += TGRSIFunctions::PhotoPeak(dim,&par[npeaks*6+5]);
	return result;
}

TPeak* TMultiPeak::GetPeak(UInt_t idx){
   if(idx < fPeakVec.size())
      return fPeakVec.at(idx);
   else
      printf("No matching peak at index %u\n",idx);

   return 0;
}

void TMultiPeak::DrawPeaks() const {
//Draws the individual TPeaks that make up the TMultiPeak. ROOT makes this a complicated process. The result on the
//histogram might have memory issues.
   Double_t xlow,xhigh;
   GetRange(xlow,xhigh);
   Double_t npeaks = fPeakVec.size();
   for(size_t i=0; i<fPeakVec.size();++i){
      TPeak* peak = fPeakVec.at(i);
      //Should be good enough to draw between -2 and +2 fwhm
      Double_t centroid = peak->GetCentroid();
      Double_t range     = 2.*peak->GetFWHM();
  
      TF1* sum = new TF1(Form("tmp%s",peak->GetName()),SinglePeakBG,centroid-range,centroid+range,fPeakVec.size()*6 + 11);
  
      for(int j=0; j<GetNpar(); ++j){
         sum->SetParameter(j,GetParameter(j));
      }
      for(int j=0; j<5;++j){
         sum->SetParameter(npeaks*6 + 5 +j, peak->GetParameter(j));
      }

      sum->SetNpx(1000);
      sum->SetLineStyle(2);
      sum->SetLineColor(kMagenta);

    //  peak->DrawF1(Form("%s + %s",peak->GetName(),Background()->GetName()),centroid-range,centroid+range,"same");
      sum->SetRange(centroid-range,centroid+range);
      sum->DrawCopy("SAME");
      delete sum;
   }
}
