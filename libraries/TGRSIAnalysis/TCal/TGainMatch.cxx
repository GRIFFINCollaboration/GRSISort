#include "TGainMatch.h"

ClassImp(TGainMatch)

TGainMatch::TGainMatch(const TGainMatch &copy) : TCal(copy){
   ((TCal&)copy).Copy(*this);
}

void TGainMatch::Copy(TObject &obj) const{
   ((TGainMatch&)obj).fcoarse_match = fcoarse_match; 
   TCal::Copy(obj);
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
   Graph()->Set(2);

   //We now want to create a peak for each one we found (2) and fit them.
   for(int x=0; x<nfound; x++){
      TPeak tmpPeak(foundbin[x],foundbin[x] - 20./binWidth, foundbin[x] + 20./binWidth);
      tmpPeak.SetName(Form("GM_Cent_%lf",foundbin[x]));//Change the name of the TPeak to know it's origin
      tmpPeak.Fit(hist,"M+");
      Graph()->SetPoint(x,tmpPeak.GetParameter("centroid"),engvec[x]);
   }

   TF1* gainfit = new TF1("gain","pol1");

   Graph()->Fit(gainfit,"SC0");
   SetFitFunction(Graph()->GetFunction("gain"));//Have to do this because I want to delete gainfit.

   delete gainfit;
   //We have finished gain matching so let the TGainMatch know that it is a coarse gain
   fcoarse_match = true;
   delete s;
   return true;
}

Bool_t TGainMatch::FineMatch(TH1* hist1, TPeak* peak1, TH1* hist2, TPeak* peak2, Int_t channelNum){
//You need to pass a TPeak with the centroid and range set to the real energy centroid and ranges.
//The function uses the coarse gain parameters to find the peak and gain matches the raw spectrum.
//This is more useful as it allows a script to find all of the peaks.
   
   if(!hist1 || !hist2){
      Error("FineMatch","No histogram being pointed to");
      return false;
   }

   //Check to see that the histogram isn't empty
   if(hist1->GetEntries() < 1 || hist2->GetEntries() < 1){
      Error("FineMatch","Histogram is empty");
      return false;
   }

   //See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   Double_t gain;
   TChannel *chan = TChannel::GetChannelByNumber(channelNum);
   if(!chan){
      if(channelNum !=9999)
         Warning("FineMatch","Channel Number %d does not exist in current memory.",channelNum);
      if(GetFitFunction()){
         gain = GetParameter(1);
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
   }

   //The reason I'm using TPeak here is we might want to gain match "TPhotopeaks", or "TElectronPeaks", or 
   //"TCrazyNonGaussian" Peak. All we care about is that it has a centroid. 
   if(!peak1 || !peak2){
      Error("FineMatch","No TPeak being pointed to");
      return false;
   }

   //Set the channel number
   Graph()->Set(2);

   //Find the energy of the peak that we want to use
   Double_t energy[2] = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};
   std::cout << peak1->GetParameter("centroid") << " ENERGIES " << energy[1] << std::endl;  
   //Offsets are very small right now so I'm not including them until they become a problem.
   peak1->SetParameter("centroid",energy[0]/gain);
   peak2->SetParameter("centroid",energy[1]/gain);

   //Change the range for the fit to be in the gain corrected spectrum

   peak1->SetRange(peak1->GetXmin()/gain,peak1->GetXmax()/gain);
   peak2->SetRange(peak2->GetXmin()/gain,peak2->GetXmax()/gain);

   peak1->Fit(hist1,"M+");
   peak2->Fit(hist2,"M+");
   
   hist1->Draw();
   peak1->Draw("same");
   peak2->Draw("same");

   Double_t centroid[2] = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};

   //Put the peaks in order for ease (if the user put them in the wrong order)
   //Apparantly there is a TGraph Sort method. Might look into this later.
   if ( energy[0] > energy[1]){
         std::swap(energy[0], energy[1]);
         std::swap(centroid[0],centroid[1]);
   }

   Graph()->SetPoint(0,centroid[0],energy[0]);
   Graph()->SetPoint(1,centroid[1],energy[1]);

   TF1* gainfit = new TF1("gain","pol1");

   Graph()->Fit(gainfit,"SC0");
   SetFitFunction(Graph()->GetFunction("gain"));//Have to do this because I want to delete gainfit
    
   delete gainfit;

   fcoarse_match = false;
   return true;

}

Bool_t TGainMatch::FineMatch(TH1* hist, TPeak* peak1, TPeak* peak2, Int_t channelNum){
//You need to pass a TPeak with the centroid and range set to the real energy centroid and ranges.
//The function uses the coarse gain parameters to find the peak and gain matches the raw spectrum.
//This is more useful as it allows a script to find all of the peaks.

   return FineMatch(hist,peak1,hist,peak1,channelNum);

}


Bool_t TGainMatch::FineMatch(TH1* hist, Double_t energy1, Double_t energy2, Int_t channelNum){
//Performs fine gain matching on one histogram once we have set the rough energy
//coefficients.

   return FineMatch(hist,energy1,hist,energy2,channelNum);
}

Bool_t TGainMatch::FineMatch(TH1* hist1, Double_t energy1, TH1* hist2, Double_t energy2, Int_t channelNum){
//Performs fine gain matching on two histograms once we have set the rough energy
//coefficients. You use this if you have a two different sources giving your full range 
//of energy. This histograms should be binned the same. NOT IMPLEMENTED

   //using an automatic range of 10 keV for testing purposes.
   TPeak *peak1 = new TPeak(energy1,energy1-10.0,energy1+10.0);
   TPeak *peak2 = new TPeak(energy2,energy2-10.0,energy2+10.0);
   
   Bool_t result = FineMatch(hist1,peak1,hist2,peak2,channelNum);

   delete peak1;
   delete peak2;
   return result;
   return 0;
}

std::vector<Double_t> TGainMatch::GetParameters() const{
   std::vector<Double_t> paramlist;
   if(!GetFitFunction()){
      Error("GetParameters","Gains have not been fitted yet");
      return paramlist;
   }
   
   Int_t nparams = GetFitFunction()->GetNpar();

   for(int i=0;i<nparams;i++)
      paramlist.push_back(GetParameter(i));

   return paramlist;
}

Double_t TGainMatch::GetParameter(Int_t parameter) const{
   if(!GetFitFunction()){
      Error("GetParameter","Gains have not been fitted yet");
      return 0;
   }
   return GetFitFunction()->GetParameter(parameter); //Root does all of the checking for us.
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

Bool_t TGainMatch::FineMatchAll(TCalManager* cm, TH2 *mat1, Double_t energy1, TH2* mat2, Double_t energy2){

   //using an automatic range of 10 keV for testing purposes.
   //We need to include bin width likely? 
   TPeak *peak1 = new TPeak(energy1,energy1-10.0,energy1+10.0);
   TPeak *peak2 = new TPeak(energy2,energy2-10.0,energy2+10.0);
   
   Bool_t result = TGainMatch::FineMatchAll(cm,mat1,peak1,mat2,peak2);

   delete peak1;
   delete peak2;
   return result;
}

Bool_t TGainMatch::FineMatchAll(TCalManager* cm, TH2 *mat, Double_t energy1, Double_t energy2){
   return TGainMatch::FineMatchAll(cm,mat,energy1,mat,energy2);
}



Bool_t TGainMatch::FineMatchAll(TCalManager* cm, TH2 *mat1, TPeak* peak1, TH2 *mat2, TPeak* peak2){
//If you supply this function with a matrix of Channel vs. energy it will automatically slice, 
//and figure out the fine gains. I might add a channel range option later
   std::vector<Int_t> badlist;
   TGainMatch *gm = new TGainMatch;
   if(!cm){
      gm->Error("FineMatchAll","CalManager Pointer is NULL");
      return false;
   }
   if(!mat1 || !mat2){
      gm->Error("FineMatchAll","TH2 Pointer is NULL");
      return false;
   }
   if(!peak1 || !peak2){
      gm->Error("FineMatch","No TPeak being pointed to");
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
         gm->Warning("FineMatchAll","Empty channel = %d",chan);
         continue;
      }
      if(!(gm->FineMatch(h1,copy_peak1,h2,copy_peak2,chan))){
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

Bool_t TGainMatch::FineMatchAll(TCalManager* cm, TH2 *mat, TPeak* peak1, TPeak* peak2){
   return TGainMatch::FineMatchAll(cm,mat,peak1,mat,peak2);
}


void TGainMatch::Clear(Option_t *opt) {
   this->fcoarse_match = true;
   TCal::Clear();
}

