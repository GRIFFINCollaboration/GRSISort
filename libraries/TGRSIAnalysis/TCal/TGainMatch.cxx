#include "TGainMatch.h"

#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "TRandom2.h"
#include "TError.h"
#include <algorithm>
#include <map>


ClassImp(TGainMatch)

TGainMatch::TGainMatch(const TGainMatch &copy) : TCal(copy){
   ((TGainMatch&)copy).Copy(*this);
}

void TGainMatch::Copy(TObject &obj) const{
   ((TGainMatch&)obj).fcoarse_match = fcoarse_match; 
   TCal::Copy(obj);
}

void TGainMatch::CalculateGain(Double_t cent1, Double_t cent2, Double_t eng1, Double_t eng2){

   //Put the peaks in order for ease (if the user put them in the wrong order)
   //Apparantly there is a TGraph Sort method. Might look into this later.
   if ( eng1 > eng2){
         std::swap(eng1, eng2);
         std::swap(cent1,cent2);
   }

   this->SetPoint(0,cent1,eng1);
   this->SetPoint(1,cent2,eng2);

   TF1* gainfit = new TF1("gain","pol1");

   TFitResultPtr res = this->Fit(gainfit,"SC0");
   SetFitFunction(this->GetFunction("gain"));//Have to do this because I want to delete gainfit
   fGain_coeffs[0] = res->Parameter(0);
   fGain_coeffs[1] = res->Parameter(1);
    
   delete gainfit;

}

Bool_t TGainMatch::CoarseMatch(TH1* hist, Int_t chanNum, Double_t energy1, Double_t energy2){
//This functions is used to perform a rough gain matching on a 60Co
//source by default. This makes gain matching over a wide range much easier to do afterwards
//This might have to be changed slightly if someone wants a different type of gaussian fitted
//for gain matching purposes.
   if(!hist) return false;

   //I might want to do a clear of the gainmatching parameters at this point.

   //Check to see that the histogram isn't empty
   if(hist->GetEntries() < 1){
      Error("CoarseMatch","The histogram is empty");
      return false;
   }

   //See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   TChannel *chan = TChannel::GetChannelByNumber(chanNum);
   if(!chan){
      if(chanNum != 9999)   
         Warning("CoarseMatch","Channel Number %d does not exist in current memory.",chanNum);
   }
   else{
      //Set the channel number
      SetChannel(chan);
   }
 
   std::vector<Double_t> engvec; //This is the vector of expected energies
   //We do coarse gain matching on 60Co. So I have hardcoded the energies for now
   engvec.push_back(energy1);
   engvec.push_back(energy2);

   //Sort these in case the peak is returned in the wrong order
   std::sort(engvec.begin(),engvec.end()); 
 
   //Use a TSpectrum to find the two largest peaks in the spectrum
   TSpectrum *s = new TSpectrum; //This might not have to be allocated
   Int_t nfound = s->Search(hist,2,"",0.50); //This returns peaks in order of their height in the spectrum.

   //If we didn't find two peaks, it is likely we gave it garbage
   if(nfound <2){
      Error("CoarseMatch","Did not find enough peaks");
      delete s;
      return false;
   }

   //We want to store the centroids of the found peaks
   std::vector<Double_t> foundbin;
   for(int x=0;x<2;x++){ //I have hard-coded this to 2 because I'm assuming the rough match peaks will be by far the largest.
      foundbin.push_back((Double_t)(s->GetPositionX()[x]));
      printf("Found peak at bin %lf\n",foundbin[x]);
   }
   std::sort(foundbin.begin(),foundbin.end()); 

   //Get Bin Width for use later. I am using the first peak found to set the bin width
   //If you are using antisymmetric binning... god help you.
   Double_t binWidth = hist->GetBinWidth(foundbin[0]);

   //Set the number of data points to 2. In the gain matching graph.
   this->Set(2);

   //We now want to create a peak for each one we found (2) and fit them.
   for(int x=0; x<2; x++){
      TPeak tmpPeak(foundbin[x],foundbin[x] - 20./binWidth, foundbin[x] + 20./binWidth);
      tmpPeak.SetName(Form("GM_Cent_%lf",foundbin[x]));//Change the name of the TPeak to know it's origin
      tmpPeak.Fit(hist,"M+");
      this->SetPoint(x,tmpPeak.GetParameter("centroid"),engvec[x]);
   }

   TF1* gainfit = new TF1("gain","pol1");

   TFitResultPtr res = this->Fit(gainfit,"SC0");
   SetFitFunction(this->GetFunction("gain"));//Have to do this because I want to delete gainfit.
   fGain_coeffs[0] = res->Parameter(0);
   fGain_coeffs[1] = res->Parameter(1);


   delete gainfit;
   //We have finished gain matching so let the TGainMatch know that it is a coarse gain
   fcoarse_match = true;
   delete s;
   return true;
}

Bool_t TGainMatch::FineMatchFast(TH1* hist1, TPeak* peak1, TH1* hist2, TPeak* peak2, Int_t channelNum){
//You need to pass a TPeak with the centroid and range set to the real energy centroid and ranges.
//The function uses the coarse gain parameters to find the peak and gain matches the raw spectrum.
//This is more useful as it allows a script to find all of the peaks.
   
   if(!hist1 || !hist2){
      Error("FineMatchFast","No histogram being pointed to");
      return false;
   }

   //Check to see that the histogram isn't empty
   if(hist1->GetEntries() < 1 || hist2->GetEntries() < 1){
      Error("FineMatchFast","Histogram is empty");
      return false;
   }

   //See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   Double_t gain,offset;
   TChannel *chan = TChannel::GetChannelByNumber(channelNum);
   if(!chan){
      if(channelNum !=9999)
         Warning("FineMatchFast","Channel Number %d does not exist in current memory.",channelNum);
      if(GetFitFunction()){
         gain = GetParameter(1);
         offset = GetParameter(0);
      }
      else{
         Error("Fine Match","There needs to be a coarse gain set to do a fine gain");
         return false;
      }
   }
   else
   {
      SetChannel(chan);
      //The first thing we need to do is "un-gain correct" the centroids of the TPeaks.
      //What we are actually doing is applying the recirpocal gain to the energy of the real peak
      //to figure out where the centroid of the charge is spectrum is roughly going to be
      //First read in the rough gain coefficients
      std::vector<Double_t> rough_coeffs = chan->GetENGCoeff();
      gain = rough_coeffs.at(1);
      offset = rough_coeffs.at(0);
   }

   //The reason I'm using TPeak here is we might want to gain match "TPhotopeaks", or "TElectronPeaks", or 
   //"TCrazyNonGaussian" Peak. All we care about is that it has a centroid. 
   if(!peak1 || !peak2){
      Error("FineMatchFast","No TPeak being pointed to");
      return false;
   }

   //Set the channel number
   this->Set(2);

   //Find the energy of the peak that we want to use
   Double_t energy[2] = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};
   std::cout << peak1->GetParameter("centroid") << " ENERGIES " << energy[1] << std::endl;  
   //Offsets are very small right now so I'm not including them until they become a problem.
   peak1->SetParameter("centroid",(energy[0]-offset)/gain);
   peak2->SetParameter("centroid",(energy[1]-offset)/gain);
   //Change the range for the fit to be in the gain corrected spectrum

   peak1->SetRange((peak1->GetXmin()-offset)/gain,(peak1->GetXmax()-offset)/gain);
   peak2->SetRange((peak2->GetXmin()-offset)/gain,(peak2->GetXmax()-offset)/gain);

   //The gains won't be perfect, so we need to search for the peak within a range.
   hist1->GetXaxis()->SetRangeUser(peak1->GetXmin()-20.,peak1->GetXmax()+20.);
   TSpectrum s;
   Int_t nfound = s.Search(hist1,2,"",0.3);

   for(int x=0;x<nfound;x++)
      std::cout << s.GetPositionX()[x] << std::endl;
   Double_t closest_peak = 0;
   Double_t closest_diff = 10000;
   for(int x=0;x<nfound;x++){
      if(fabs(peak1->GetCentroid() - s.GetPositionX()[x]) < closest_diff){
         closest_peak = s.GetPositionX()[x];
         closest_diff = fabs(peak1->GetCentroid() - s.GetPositionX()[x]);
      }
   }

   Double_t range_width = (peak1->GetXmax() - peak1->GetXmin())/2.;
   peak1->SetParameter("centroid",closest_peak);
   peak1->SetRange(peak1->GetCentroid()-range_width,peak1->GetCentroid()+range_width);
   std::cout << "Centroid Guess " << peak1->GetCentroid() << std::endl;
   std::cout << "Range Low " << peak1->GetXmin() << " " << peak1->GetXmax() << std::endl;
   
   closest_peak = 0;
   closest_diff = 10000;
   hist2->GetXaxis()->SetRangeUser(peak2->GetXmin()-20.,peak2->GetXmax()+20.);
   TSpectrum s2;
   nfound = s2.Search(hist2,2,"",0.3); //Search the next histogram
   for(int x=0;x<nfound;x++)
      std::cout << s2.GetPositionX()[x] << std::endl;

   for(int x=0;x<nfound;x++)
      if(fabs(peak2->GetCentroid() - s2.GetPositionX()[x]) < closest_diff){
         closest_peak = s2.GetPositionX()[x];
         closest_diff = fabs(peak2->GetCentroid() - s2.GetPositionX()[x]);
      }
   Double_t range_width2 = (peak2->GetXmax() - peak2->GetXmin())/2.;
   peak2->SetParameter("centroid",closest_peak);
   peak2->SetRange(peak2->GetCentroid()-range_width2,peak2->GetCentroid()+range_width2);
   
   std::cout << "Centroid Guess " << peak2->GetCentroid() << std::endl;
   std::cout << "Range High " << peak2->GetXmin() << " " << peak2->GetXmax() << std::endl;

   hist1->GetXaxis()->UnZoom();
   hist2->GetXaxis()->UnZoom();
   peak1->Fit(hist1,"MS+");
   peak2->Fit(hist2,"MS+");
   
   hist1->Draw();
   peak1->Draw("same");
   peak2->Draw("same");

   Double_t centroid[2] = {peak1->GetCentroid(), peak2->GetCentroid()};

   //Put the peaks in order for ease (if the user put them in the wrong order)
   //Apparantly there is a TGraph Sort method. Might look into this later.
   if ( energy[0] > energy[1]){
         std::swap(energy[0], energy[1]);
         std::swap(centroid[0],centroid[1]);
   }

   this->SetPoint(0,centroid[0],energy[0]);
   this->SetPoint(1,centroid[1],energy[1]);

   TF1* gainfit = new TF1("gain","pol1");

   TFitResultPtr res = this->Fit(gainfit,"SC0");
   SetFitFunction(this->GetFunction("gain"));//Have to do this because I want to delete gainfit
   fGain_coeffs[0] = res->Parameter(0);
   fGain_coeffs[1] = res->Parameter(1);
    
   delete gainfit;

   fcoarse_match = false;
   return true;

}

Bool_t TGainMatch::FineMatchFast(TH1* hist, TPeak* peak1, TPeak* peak2, Int_t channelNum){
//You need to pass a TPeak with the centroid and range set to the real energy centroid and ranges.
//The function uses the coarse gain parameters to find the peak and gain matches the raw spectrum.
//This is more useful as it allows a script to find all of the peaks.

   return FineMatchFast(hist,peak1,hist,peak1,channelNum);

}


Bool_t TGainMatch::FineMatchFast(TH1* hist, Double_t energy1, Double_t energy2, Int_t channelNum){
//Performs fine gain matching on one histogram once we have set the rough energy
//coefficients.

   return FineMatchFast(hist,energy1,hist,energy2,channelNum);
}

Bool_t TGainMatch::FineMatchFast(TH1* hist1, Double_t energy1, TH1* hist2, Double_t energy2, Int_t channelNum){
//Performs fine gain matching on two histograms once we have set the rough energy
//coefficients. You use this if you have a two different sources giving your full range 
//of energy. This histograms should be binned the same. NOT IMPLEMENTED

   //using an automatic range of 10 keV for testing purposes.
   TPeak *peak1 = new TPeak(energy1,energy1-10.0,energy1+10.0);
   TPeak *peak2 = new TPeak(energy2,energy2-10.0,energy2+10.0);
   
   Bool_t result = FineMatchFast(hist1,peak1,hist2,peak2,channelNum);

   delete peak1;
   delete peak2;
   return result;
   return 0;
}

void TGainMatch::WriteToChannel() const {
   if(!GetChannel()){
      Error("WriteToChannel","No Channel Set");
      return;
   }
   GetChannel()->DestroyENGCal();
   printf("Writing to channel %d\n",GetChannel()->GetNumber());
   printf("p0 = %lf \t p1 = %lf\n",this->GetParameter(0),this->GetParameter(1));
   //Set the energy parameters based on the fitted gains.
   GetChannel()->AddENGCoefficient(this->GetParameter(0));
   GetChannel()->AddENGCoefficient(this->GetParameter(1));
}

void TGainMatch::Print(Option_t *opt) const {
   printf("\n");
   printf("GainMatching: ");
   if(fcoarse_match) 
      printf("COARSE\n");
   else              
      printf("FINE\n");

   if(faligned) 
      printf("Aligned\n");
   else              
      printf("NOT Aligned\n");

   printf("Gain Coefficients: %lf\t%lf\n",fGain_coeffs[0],fGain_coeffs[1]);
   printf("Align Coefficients: %lf\t%lf\n", fAlign_coeffs[0], fAlign_coeffs[1]);

   TCal::Print();
}


Bool_t TGainMatch::CoarseMatchAll(TCalManager* cm, TH2 *mat, Double_t energy1, Double_t energy2){
//If you supply this function with a matrix of Channel vs. energy it will automatically slice, 
//and figure out the coarse gains. I might add a channel range option later
   std::vector<Int_t> badlist;
   TGainMatch *gm = new TGainMatch;
   if(!cm){
      gm->Error("CoarseMatchAll","CalManager Pointer is NULL");
      return false;
   }
   if(!mat){
      gm->Error("CoarseMatchAll","TH2 Pointer is NULL");
      return false;
   }
   //Find the range of channels provided
   Int_t first_chan = mat->GetXaxis()->GetFirst();
   Int_t last_chan  = mat->GetXaxis()->GetLast();
   //The first thing we need to do is slice the matrix into it's channel vs energy.
   TH1D* h1 = new TH1D;
	for(int chan=first_chan; chan<=last_chan;chan++){
      gm->Clear();
      printf("\nNow fitting channel: %d\n",chan);
		TH1D* h1 = (TH1D*)(mat->ProjectionY(Form("Channel%d",chan),chan+1,chan+1,"o"));
      printf("BIN WIDTH %lf\n",h1->GetXaxis()->GetBinWidth(h1->GetXaxis()->GetFirst() + 1));
		if(h1->Integral() < 100)
         continue;

      if(!(gm->CoarseMatch(h1,chan))){
         badlist.push_back(chan);
         continue;
      }
      cm->AddToManager(gm);
   }
   if(badlist.size())
      printf("The following channels did not gain match properly: ");
   for(int i=0;i<badlist.size();i++)
      printf("%d\t",badlist.at(i));
   
   delete h1;
   delete gm;
   
   return true;
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2 *mat1, Double_t energy1, TH2* mat2, Double_t energy2){

   //using an automatic range of 10 keV for testing purposes.
   //We need to include bin width likely? 
   TPeak *peak1 = new TPeak(energy1,energy1-10.0,energy1+10.0);
   TPeak *peak2 = new TPeak(energy2,energy2-10.0,energy2+10.0);
   
   Bool_t result = TGainMatch::FineMatchFastAll(cm,mat1,peak1,mat2,peak2);

   delete peak1;
   delete peak2;
   return result;
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2 *mat, Double_t energy1, Double_t energy2){
   return TGainMatch::FineMatchFastAll(cm,mat,energy1,mat,energy2);
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2 *mat1, TPeak* peak1, TH2 *mat2, TPeak* peak2){
//If you supply this function with a matrix of Channel vs. energy it will automatically slice, 
//and figure out the fine gains. I might add a channel range option later
   std::vector<Int_t> badlist;
   TGainMatch *gm = new TGainMatch;
   if(!cm){
      gm->Error("FineMatchFastAll","CalManager Pointer is NULL");
      return false;
   }
   if(!mat1 || !mat2){
      gm->Error("FineMatchFastAll","TH2 Pointer is NULL");
      return false;
   }
   if(!peak1 || !peak2){
      gm->Error("FineMatchFastAll","No TPeak being pointed to");
      return false;
   }

   //Find the range of channels provided, we want to use the largest range provided
   Int_t first_chan1 = mat1->GetXaxis()->GetFirst();
   Int_t last_chan1  = mat1->GetXaxis()->GetLast();
   Int_t first_chan2 = mat2->GetXaxis()->GetFirst();
   Int_t last_chan2  = mat2->GetXaxis()->GetLast(); 
   Int_t first_chan  = std::min(first_chan1,first_chan2);
   Int_t last_chan   = std::max(last_chan1,last_chan2);
   //The first thing we need to do is slice the matrix into it's channel vs energy.
   TH1D* h1 = new TH1D;
   TH1D* h2 = new TH1D;
	for(int chan=first_chan; chan<=last_chan;chan++){
      gm->Clear();
      //Make a copy of the TPeaks so that we can fit each of them
      //TPeak *copy_peak1 = (TPeak*)peak1->Clone();//Clone creates smart pointers so these will be dealoccated automatically.
      //TPeak *copy_peak2 = (TPeak*)peak2->Clone();
      TPeak *copy_peak1 = new TPeak(*peak1);
      TPeak *copy_peak2 = new TPeak(*peak2);

      printf("\nNow fitting channel: %d\n",chan);
		TH1D* h1 = (TH1D*)(mat1->ProjectionY(Form("Channel%d_mat1",chan),chan+1,chan+1,"o"));
		TH1D* h2 = (TH1D*)(mat2->ProjectionY(Form("Channel%d_mat2",chan),chan+1,chan+1,"o"));
		if(h1->Integral() < 100 || h2->Integral() < 100){
         gm->Warning("FineMatchFastAll","Empty channel = %d",chan);
         continue;
      }
      if(!(gm->FineMatchFast(h1,copy_peak1,h2,copy_peak2,chan))){
         badlist.push_back(chan);
         continue;
      }
      cm->AddToManager(gm);

      delete copy_peak1;
      delete copy_peak2;
   }
   if(badlist.size())
      printf("The following channels did not gain match properly: ");
   for(int i=0;i<badlist.size();i++)
      printf("%d\t",badlist.at(i));
   
   delete h1;
   delete h2;
   delete gm;
   
   return true;
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2 *mat, TPeak* peak1, TPeak* peak2){
   return TGainMatch::FineMatchFastAll(cm,mat,peak1,mat,peak2);
}

void TGainMatch::Clear(Option_t *opt) {
   this->fcoarse_match = true;
   this->faligned = false;
   this->fAlign_coeffs[0] = 0.0;
   this->fAlign_coeffs[1] = 1.0;
   this->fGain_coeffs[0] = 0.0;
   this->fGain_coeffs[1] = 1.0;
   TCal::Clear();
}

Bool_t TGainMatch::Align(TH1* test, TH1* hist,Int_t low_range, Int_t high_range){
   //Minimizes the chi^2 between the bin contents of the test histogram and the bin contents of the histogram to be matched'
   if(!(test && hist)){
      printf("Unassigned histogram\n");
      return false;
   }
   int randomSeed = -1;
   fhist = hist;//Need this histogram to be seen by ftotal...Don't know how else to do this right now.
 //  TF1 *tmpfunc = new TF1("tmpfunc",this,&TGainMatch::HistCompare,test->GetMinimumStored()+1,test->GetMaximumStored()-1,3);
   
  /* //Initial Guesses

   TSpectrum stest,shist;
   TGraph guess_graph;
   Int_t nfoundtest = stest.Search(test);
   Int_t nfoundhist = shist.Search(hist);
   TF1* shift_guess = new TF1("shift_guess","pol1");
   if(nfoundtest >1 && nfoundhist > 1){
      stest.GetPositionX()[0]/shist.GetPositionX()[0];
      guess_graph.SetPoint(0,shist.GetPositionX()[0],stest.GetPositionX()[0]);
      guess_graph.SetPoint(1,shist.GetPositionX()[1],stest.GetPositionX()[1]);

      guess_graph.Print();

      TFitResultPtr res = guess_graph.Fit(shift_guess,"SC0");
   }
*/

   TF1 *tmpfunc = new TF1("tmpfunc",this,&TGainMatch::HistCompare,low_range,high_range,3);
   tmpfunc->SetNpx(10000);
   Double_t norm = test->GetMaximum();
   tmpfunc->SetParameters(1.0,1.0,1.0);

 //  hist->Sumw2();

  /* for(int i =0; i<hist->GetXaxis()->GetNbins();i++){
     // hist->SetBinError(i,TMath::Sqrt(TMath::Sqrt(hist->GetBinContent(i))));
      hist->SetBinError(i,1./(1.+hist->GetBinContent(i)));
   }
*/
  // ftot->SetParLimits(0,.3*norm,norm);

   const char* minName = "Minuit2";
   const char* algoName = "Scan";
   //ROOT::Math::MinimizerOptions::SetDefaultMinimizer("TMinuit2","Simplex");  
//   TVirtualFitter::SetDefaultFitter("Fumili"); //USE COMBINATION!!!!
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   TFitResultPtr res = test->Fit("tmpfunc","RSILV");
   fAlign_coeffs[0] = res->Parameter(1);
   fAlign_coeffs[1] = res->Parameter(2);
   std::cout << "Chi2: " << res->Chi2()/res->Ndf()<< std::endl;

   faligned = true;
  // delete shift_guess;
   return true;

}

Bool_t TGainMatch::AlignAll(TCalManager* cm, TH1* hist, TH2 *mat, Int_t low_range, Int_t high_range){
   std::vector<Int_t> badlist;
   TGainMatch *gm = new TGainMatch;
   if(!cm){
      gm->Error("AlignAll","CalManager Pointer is NULL");
      return false;
   }
   if(!mat){
      gm->Error("AlignAll","TH2 Pointer is NULL");
      return false;
   }
   if(!hist){
      gm->Error("AlignAll","TH1 Pointer is NULL");
      return false;
   }
   //Find the range of channels provided
   Int_t first_chan = mat->GetXaxis()->GetFirst();
   Int_t last_chan  = mat->GetXaxis()->GetLast();
   //The first thing we need to do is slice the matrix into it's channel vs energy.
   TH1D* h1 = new TH1D;
//	for(int chan=first_chan; chan<=last_chan;chan++){
	for(int chan=first_chan; chan<=2;chan++){
      gm->Clear();
      printf("\nNow fitting channel: %d\n",chan);
		TH1D* h1 = (TH1D*)(mat->ProjectionY(Form("Channel%d",chan),chan+1,chan+1,"o"));
      printf("BIN WIDTH %lf\n",h1->GetXaxis()->GetBinWidth(h1->GetXaxis()->GetFirst() + 1));
		if(h1->Integral() < 100)
         continue;

      if(!(gm->Align(hist,h1,low_range,high_range))){
         badlist.push_back(chan);
         continue;
      }
      cm->AddToManager(gm);
   }
   if(badlist.size())
      printf("The following channels did not gain match properly: ");
   for(int i=0;i<badlist.size();i++)
      printf("%d\t",badlist.at(i));
   
   delete h1;
   delete gm;
   
   return true;
}

Bool_t TGainMatch::FineMatchAll(TCalManager* cm, TH2 *charge_mat, TH2* eng_mat, Int_t testchan, Double_t energy1, Double_t energy2,Int_t low_range, Int_t high_range){
   //Should be run to correct drifting gains. Requires a previously gain matched spectrum, and ideally a roughly energy calibrated matrix

   TGainMatch* gm = new TGainMatch;
   if(!cm){
      gm->Error("FineMatchAll","CalManager Pointer is NULL");
      return false;
   }
   if(!charge_mat || !eng_mat){
      gm->Error("FineMatchAll","TH2 Pointer is NULL");
      return false;
   }
  /* if(!testhist){
      gm->Error("FineMatchAll","TH1 Pointer is NULL");
      return false;
   }*/
   Double_t binwidth;
   binwidth = (Int_t)(0.5+1./eng_mat->GetYaxis()->GetBinWidth(100));
   eng_mat->RebinY(binwidth);


   std::vector<Int_t> badlist;
   //Find the range of channels provided, we want to use the largest range provided
   Int_t first_chan = eng_mat->GetXaxis()->GetFirst();
   Int_t last_chan  = eng_mat->GetXaxis()->GetLast();
   //The first thing we need to do is slice the matrix into it's channel vs energy.
   TH1D* chargeh = new TH1D;
   TH1D* engh = new TH1D;

   TH1D* testhist = new TH1D;
   testhist = (TH1D*)(eng_mat->ProjectionY(Form("Test%d_mat",testchan),testchan+1,testchan+1,"o"));


	for(int chan=first_chan; chan<=last_chan;chan++){
 //  for(int chan=48;chan<=49;chan++){
      printf("\nNow fitting channel: %d\n",chan);
		TH1D* chargeh = (TH1D*)(charge_mat->ProjectionY(Form("Charge%d_mat",chan),chan+1,chan+1,"o"));
      TH1D* engh = (TH1D*)(eng_mat->ProjectionY(Form("Energy%d_mat",chan),chan+1,chan+1,"o"));
		if(chargeh->Integral() < 100 || chargeh->Integral() < 100){
         gm->Warning("FineMatchAll","Empty channel = %d",chan);
         continue;
      }
      //Needs to align before matching
      if(!(gm->FineMatch(engh,testhist,chargeh,energy1,energy2,low_range,high_range,chan))){
         badlist.push_back(chan);
         continue;
      }
      cm->AddToManager(gm);
   }
   if(badlist.size())
      printf("The following channels did not gain match properly: ");
   for(int i=0;i<badlist.size();i++)
      printf("%d\t",badlist.at(i));

   
   delete engh;
   delete chargeh;
   delete testhist;
   delete gm;
   
   return true;
}


Bool_t TGainMatch::FineMatch(TH1 *energy_hist, TH1* testhist, TH1* charge_hist, Double_t energy1, Double_t energy2, Int_t low_range, Int_t high_range, Int_t channelNum){
   //Should be run to correct drifting gains. Requires a previously gain matched spectrum, and ideally a roughly energy calibrated spectrum
   Align(testhist,energy_hist,low_range,high_range);
   TH1* hist2 = charge_hist; //Cheating for easier modification later
   if(!charge_hist || !testhist || !energy_hist){
      Error("FineMatch","No histogram being pointed to");
      return false;
   }

   //Check to see that the histogram isn't empty
   if(charge_hist->GetEntries() < 1 || testhist->GetEntries() < 1 || energy_hist->GetEntries()<1){
      Error("FineMatchFast","Histogram is empty");
      return false;
   }

   //See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   Double_t gain,offset;
   TChannel *chan = TChannel::GetChannelByNumber(channelNum);
   if(!chan){
      if(channelNum !=9999)
         Warning("FineMatch","Channel Number %d does not exist in current memory.",channelNum);
      if(GetFitFunction()){
         gain = GetParameter(1);
         offset = GetParameter(0);
      }
      else{
         Error("FineMatch","There needs to be a coarse gain set to do a fine gain");
         return false;
      }
   }
   else
   {
      SetChannel(chan);
      //The first thing we need to do is "un-gain correct" the centroids of the TPeaks.
      //What we are actually doing is applying the recirpocal gain to the energy of the real peak
      //to figure out where the centroid of the charge is spectrum is roughly going to be
      //First read in the rough gain coefficients
      std::vector<Double_t> rough_coeffs = chan->GetENGCoeff();
      gain = rough_coeffs.at(1);
      offset = rough_coeffs.at(0);
   }

   TPeak *peak1 = new TPeak(energy1,energy1-15.0,energy1+15.0);
   TPeak *peak2 = new TPeak(energy2,energy2-15.0,energy2+15.0);

   //Set the channel number
   //Graph()->Set(2);
   this->Set(2);

   //Find the energy of the peak that we want to use
   Double_t energy[2] = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};
   std::cout << peak1->GetParameter("centroid") << " ENERGIES " << energy[1] << std::endl;  
   //Offsets are very small right now so I'm not including them until they become a problem.
//   peak1->SetParameter("centroid",((energy[0]-offset)/gain)*fAlign_coeffs[1] + fAlign_coeffs[0]);
//   peak2->SetParameter("centroid",((energy[0]-offset)/gain)*fAlign_coeffs[1] + fAlign_coeffs[0]);
   peak1->SetParameter("centroid",(energy[0]*fAlign_coeffs[1]+fAlign_coeffs[0] -offset)/gain);
   peak2->SetParameter("centroid",(energy[1]*fAlign_coeffs[1]+fAlign_coeffs[0] -offset)/gain);
   //Change the range for the fit to be in the gain corrected spectrum
   
   std::cout << " Should be: " << ((energy[0]*fAlign_coeffs[1]+fAlign_coeffs[0] -offset)/gain)<< std::endl;
   std::cout << " Should be: " <<   ((energy[1]*fAlign_coeffs[1]+fAlign_coeffs[0] -offset)/gain)  << std::endl;


//   peak1->SetRange(((peak1->GetXmin()-offset)/gain)*fAlign_coeffs[1] + fAlign_coeffs[0],((peak1->GetXmax()-offset)/gain)*fAlign_coeffs[1] + fAlign_coeffs[0]);
//   peak2->SetRange(((peak2->GetXmin()-offset)/gain)*fAlign_coeffs[1] + fAlign_coeffs[0],((peak2->GetXmax()-offset)/gain)*fAlign_coeffs[1] + fAlign_coeffs[0]);
   peak1->SetRange(((peak1->GetXmin()*fAlign_coeffs[1]+fAlign_coeffs[0] -offset)/gain),((peak1->GetXmax()*fAlign_coeffs[1] - fAlign_coeffs[0]-offset)/gain));
   peak2->SetRange(((peak2->GetXmin()*fAlign_coeffs[1]+fAlign_coeffs[0] -offset)/gain),((peak2->GetXmax()*fAlign_coeffs[1] - fAlign_coeffs[0]-offset)/gain));

   //The gains won't be perfect, so we need to search for the peak within a range.
   TSpectrum s;
   Int_t nfound = s.Search(charge_hist,2,"",0.8);
   charge_hist->GetXaxis()->SetRangeUser(peak1->GetXmin()-20.,peak1->GetXmax()+20.);
   nfound = s.Search(charge_hist,2,"",0.3);

   for(int x=0;x<nfound;x++)
      std::cout << s.GetPositionX()[x] << std::endl;
   Double_t largest_peak = s.GetPositionX()[0];
   Double_t closest_peak = 0;
   Double_t closest_diff = 10000;
   for(int x=0;x<nfound;x++){
      if(fabs(peak1->GetCentroid() - s.GetPositionX()[x]) < closest_diff){
         closest_peak = s.GetPositionX()[x];
         closest_diff = fabs(peak1->GetCentroid() - s.GetPositionX()[x]);
      }
   }
   

   Double_t range_width = (peak1->GetXmax() - peak1->GetXmin())/2.;
   peak1->SetParameter("centroid",closest_peak);
   peak1->SetRange(peak1->GetCentroid()-range_width,peak1->GetCentroid()+range_width);
   std::cout << "Centroid Guess " << peak1->GetCentroid() << std::endl;
   std::cout << "Range Low " << peak1->GetXmin() << " " << peak1->GetXmax() << std::endl;
   
   closest_peak = 0;
   closest_diff = 10000;
   TSpectrum s2;
   nfound = s2.Search(hist2,2,"",0.8); //Search the next histogram
   hist2->GetXaxis()->SetRangeUser(peak2->GetXmin()-20.,peak2->GetXmax()+20.);
   std::cout << "RANGE: " << peak2->GetXmin()-20. << " " << peak2->GetXmax()+20. << std::endl;
   nfound = s2.Search(hist2,2,"",0.3); //Search the next histogram
   for(int x=0;x<nfound;x++)
      std::cout << s2.GetPositionX()[x] << std::endl;

   largest_peak = s2.GetPositionX()[0];
   for(int x=0;x<nfound;x++)
      if(fabs(peak2->GetCentroid() - s2.GetPositionX()[x]) < closest_diff){
         closest_peak = s2.GetPositionX()[x];
         closest_diff = fabs(peak2->GetCentroid() - s2.GetPositionX()[x]);
      }
   Double_t range_width2 = (peak2->GetXmax() - peak2->GetXmin())/2.;
   peak2->SetParameter("centroid",largest_peak);
   peak2->SetRange(peak2->GetCentroid()-range_width2,peak2->GetCentroid()+range_width2);
   
   std::cout << "Centroid Guess " << peak2->GetCentroid() << std::endl;
   std::cout << "Range High " << peak2->GetXmin() << " " << peak2->GetXmax() << std::endl;

   charge_hist->GetXaxis()->UnZoom();
   hist2->GetXaxis()->UnZoom();
   peak1->Fit(charge_hist,"MS+");
   peak2->Fit(hist2,"MS+");
   
   charge_hist->Draw();
   peak1->Draw("same");
   peak2->Draw("same");

   Double_t centroid[2] = {peak1->GetCentroid(), peak2->GetCentroid()};

   //Put the peaks in order for ease (if the user put them in the wrong order)
   //Apparantly there is a TGraph Sort method. Might look into this later.
   if ( energy[0] > energy[1]){
         std::swap(energy[0], energy[1]);
         std::swap(centroid[0],centroid[1]);
   }

   this->SetPoint(0,centroid[0],energy[0]);
   this->SetPoint(1,centroid[1],energy[1]);

   TF1* gainfit = new TF1("gain","pol1");

   TFitResultPtr res = this->Fit(gainfit,"SC0");
   SetFitFunction(this->GetFunction("gain"));//Have to do this because I want to delete gainfit
   fGain_coeffs[0] = res->Parameter(0);
   fGain_coeffs[1] = res->Parameter(1);
    
   delete gainfit;

   fcoarse_match = false;
   delete peak1;
   delete peak2;
   return true;
}

Double_t TGainMatch::HistCompare(Double_t *x, Double_t *par) {
   Double_t xx = x[0];
   Int_t bin = fhist->GetXaxis()->FindBin(xx*par[2] + par[1]);
   Double_t content = fhist->GetBinContent(bin);

   return par[0]*content;
}


