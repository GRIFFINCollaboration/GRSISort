#include "TGainMatch.h"

ClassImp(TGainMatch)

Bool_t TGainMatch::CoarseMatch(TH1* hist, Int_t chanNum, Double_t energy1, Double_t energy2){
//This functions is used to perform a rough gain matching on a 60Co
//source by default. This makes gain matching over a wide range much easier to do afterwards
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
      Error("CoarseMatch","Channel Number %d does not exist in current memory.",chanNum);
      return false;
   }

   //Set the channel number
   SetChannel(chan);

   std::vector<Double_t> engvec; //This is the vector of expected energies
   //We do coarse gain matching on 60Co. So I have hardcoded the energies for now
   engvec.push_back(energy1);
   engvec.push_back(energy2);

   //Sort these in case the peak is returned in the wrong order
   std::sort(engvec.begin(),engvec.end()); 
 
   //Use a TSpectrum to find the two largest peaks in the spectrum
   TSpectrum *s = new TSpectrum; //This might not have to be allocated
   Int_t nfound = s->Search(hist); 

   //If we didn't find two peaks, it is likely we gave it garbage
   if(nfound <2){
      Error("CoarseMatch","Did not find enough peaks");
      delete s;
      return false;
   }

   //We want to store the centroids of the found peaks
   std::vector<Double_t> foundbin;
   for(int x=0;x<nfound;x++){
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
      tmpPeak.SetName(Form("GM_Chan%u_%lf",GetChannel()->GetNumber(),foundbin[x]));//Change the name of the TPeak to know it's origin
      tmpPeak.Fit(hist,"EM+");
      Graph()->SetPoint(x,engvec[x],tmpPeak.GetParameter("centroid"));
   }

   TF1* gainfit = new TF1("gain","pol1");

   Graph()->Fit(gainfit,"SC0");

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
   
   if(!hist1 || !hist2) return false;

   //Check to see that the histogram isn't empty
   if(hist1->GetEntries() < 1 || hist2->GetEntries() < 1){
      Error("FineMatch","Histogram is empty");
      return false;
   }

   //See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   TChannel *chan = TChannel::GetChannelByNumber(channelNum);
   if(!chan){
      Error("FineMatch","Channel Number %d does not exist in current memory.",channelNum);
      return false;
   }

   //Set the channel number
   SetChannel(chan);

   Graph()->Set(2);

   //The first thing we need to do is "un-gain correct" the centroids of the TPeaks.
   //First read in the rough gain coefficients
   std::vector<Double_t> rough_coeffs = chan->GetENGCoeff();
   
   Double_t gain = rough_coeffs.at(1);

   //Find the energy of the peak that we want to use
   Double_t energy[2] = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};

   //Offsets are very small right now so I'm not including them until they become a problem.
   peak1->SetParameter("centroid",energy[0]/gain);
   peak2->SetParameter("centroid",energy[1]/gain);

   //Change the range for the fit to be in the gain corrected spectrum
   peak1->SetRange(peak1->GetXmin()/gain,peak1->GetXmax()/gain);
   peak2->SetRange(peak2->GetXmin()/gain,peak2->GetXmin()/gain);

   peak1->Fit(hist1,"EM");
   peak2->Fit(hist2,"EM");

   Double_t centroid[2] = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};

   //Put the peaks in order for ease (if the user put them in the wrong order)
   if ( energy[0] > energy[1]){
         std::swap(energy[0], energy[1]);
         std::swap(centroid[0],centroid[1]);
   }

   Graph()->SetPoint(0,energy[0],centroid[0]);
   Graph()->SetPoint(1,energy[1],centroid[1]);
   

   TF1* gainfit = new TF1("gain","pol1");

   Graph()->Fit(gainfit,"SC0");

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
}

std::vector<Double_t> TGainMatch::GetParameters() const{
   std::vector<Double_t> paramlist;
   if(!(this->Graph()->GetFunction("gain"))){
      Error("GetParameters","Gains have not been fitted yet");
      return paramlist;
   }
   
   Int_t nparams = this->Graph()->GetFunction("gain")->GetNpar();

   for(int i=0;i<nparams;i++)
      paramlist.push_back(GetParameter(i));

   return paramlist;
}

Double_t TGainMatch::GetParameter(Int_t parameter) const{
   if(!(this->Graph()->GetFunction("gain"))){
      Error("GetParameter","Gains have not been fitted yet");
      return 0;
   }
   return Graph()->GetFunction("gain")->GetParameter(parameter); //Root does all of the checking for us.
}

void TGainMatch::WriteToChannel() const {
   if(!GetChannel()){
      Error("WriteToChannel","No Channel Set");
      return;
   }
   GetChannel()->DestroyENGCal();
   //Set the energy parameters based on the fitted gains.
   GetChannel()->AddENGCoefficient(this->GetParameter(0));
   GetChannel()->AddENGCoefficient(this->GetParameter(1));
}

void TGainMatch::Print(Option_t *opt) const {
   printf("GainMatching: ");
   if(fcoarse_match) 
      printf("COARSE\n");
   else              
      printf("FINE\n");
   TCal::Print();
}

void TGainMatch::Clear(Option_t *opt) {
   this->fcoarse_match = true;
   TCal::Clear();
}

