#include "TGainMatch.h"

#include <algorithm>
#include <map>

#include "TError.h"
#include "TRandom2.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

Double_t TGainMatch::fDefaultCoarseRange = 40.;

TGainMatch::TGainMatch(const TGainMatch& copy)
   : TCal(copy), fCoarseRange(fDefaultCoarseRange)
{
   std::cout << "THIS ---> " << fCoarseRange << std::endl;
   copy.Copy(*this);
}

void TGainMatch::Copy(TObject& obj) const
{
   static_cast<TGainMatch&>(obj).fCoarseMatch = fCoarseMatch;
   static_cast<TGainMatch&>(obj).fCoarseRange = fCoarseRange;
   TCal::Copy(obj);
}

void TGainMatch::CalculateGain(Double_t cent1, Double_t cent2, Double_t eng1, Double_t eng2)
{
   // Put the peaks in order for ease (if the user put them in the wrong order)
   // Apparantly there is a TGraph Sort method. Might look into this later.
   if(eng1 > eng2) {
      std::swap(eng1, eng2);
      std::swap(cent1, cent2);
   }

   SetPoint(0, cent1, eng1);
   SetPoint(1, cent2, eng2);

   auto* gainFit = new TF1("gain", "pol1");

   TFitResultPtr res = Fit(gainFit, "SC0");
   SetFitFunction(GetFunction("gain"));   // Have to do this because I want to delete gainfit
   fGainCoeffs[0] = res->Parameter(0);
   fGainCoeffs[1] = res->Parameter(1);

   delete gainFit;
}

Bool_t TGainMatch::CoarseMatch(TH1* hist, Int_t chanNum, Double_t energy1, Double_t energy2)
{
   // This functions is used to perform a rough gain matching on a 60Co
   // source by default. This makes gain matching over a wide range much easier to do afterwards
   // This might have to be changed slightly if someone wants a different type of gaussian fitted
   // for gain matching purposes.
   if(hist == nullptr) {
      return false;
   }

   fHist = hist;

   // I might want to do a clear of the gainmatching parameters at this point.

   // Check to see that the histogram isn't empty
   if(hist->GetEntries() < 1) {
      Error("CoarseMatch", "The histogram is empty");
      return false;
   }

   // See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   TChannel* chan = TChannel::GetChannelByNumber(chanNum);
   if(chan == nullptr) {
      if(chanNum != 9999) {
         Warning("CoarseMatch", "Channel Number %d does not exist in current memory.", chanNum);
      }
   } else {
      // Set the channel number
      SetChannel(chan);
   }

   std::vector<Double_t> engVec;   // This is the vector of expected energies
   // We do coarse gain matching on 60Co. So I have hardcoded the energies for now
   engVec.push_back(energy1);
   engVec.push_back(energy2);

   // Sort these in case the peak is returned in the wrong order
   std::sort(engVec.begin(), engVec.end());

   // Change the histogram range so that the search ignores potential low energy noise.
   Int_t high = hist->GetXaxis()->GetLast();
   hist->GetXaxis()->SetRangeUser(100, hist->GetXaxis()->GetBinCenter(high));
   // Use a TSpectrum to find the two largest peaks in the spectrum
   auto* spec   = new TSpectrum;                     // This might not have to be allocated
   Int_t nFound = spec->Search(hist, 2, "", 0.50);   // This returns peaks in order of their height in the spectrum.

   // return histogram to proper range
   hist->GetXaxis()->UnZoom();
   // If we didn't find two peaks, it is likely we gave it garbage
   if(nFound < 2) {
      Error("CoarseMatch", "Did not find enough peaks");
      delete spec;
      return false;
   }

   // We want to store the centroids of the found peaks
   std::vector<Double_t> foundBin;
   for(int x = 0; x < 2;
       x++) {   // I have hard-coded this to 2 because I'm assuming the rough match peaks will be by far the largest.
      foundBin.push_back(spec->GetPositionX()[x]);
      std::cout << "Found peak at bin " << foundBin[x] << std::endl;
   }
   std::sort(foundBin.begin(), foundBin.end());

   // Get Bin Width for use later. I am using the first peak found to set the bin width
   // If you are using antisymmetric binning... god help you.
   // Double_t binWidth = hist->GetBinWidth(foundBin[0]);

   // Set the number of data points to 2. In the gain matching graph.
   Set(2);

   // We now want to create a peak for each one we found (2) and fit them.
   for(int x = 0; x < 2; x++) {
      TPeak tmpPeak(foundBin[x], foundBin[x] - fCoarseRange, foundBin[x] + fCoarseRange);
      tmpPeak.SetName(Form("GM_Cent_%lf", foundBin[x]));   // Change the name of the TPeak to know it's origin
      tmpPeak.SetLineColor(static_cast<Color_t>(2 * x + 2));
      tmpPeak.Fit(hist, "Q+");
      tmpPeak.ReleaseParameter(3);
      tmpPeak.ReleaseParameter(4);
      tmpPeak.SetLineColor(static_cast<Color_t>(2 * x + 3));
      tmpPeak.Fit(hist, "Q+");
      SetPoint(x, tmpPeak.GetParameter("centroid"), engVec[x]);
   }

   auto* gainFit = new TF1("gain", "pol1");

   TFitResultPtr res = Fit(gainFit, "SC0");
   SetFitFunction(GetFunction("gain"));   // Have to do this because I want to delete gainFit.
   fGainCoeffs[0] = res->Parameter(0);
   fGainCoeffs[1] = res->Parameter(1);

   delete gainFit;
   // We have finished gain matching so let the TGainMatch know that it is a coarse gain
   fCoarseMatch = true;
   delete spec;
   return true;
}

Bool_t TGainMatch::FineMatchFast(TH1* hist1, TPeak* peak1, TH1* hist2, TPeak* peak2, Int_t channelNum)
{
   // You need to pass a TPeak with the centroid and range set to the real energy centroid and ranges.
   // The function uses the coarse gain parameters to find the peak and gain matches the raw spectrum.
   // This is more useful as it allows a script to find all of the peaks.
   if((hist1 == nullptr) || (hist2 == nullptr)) {
      Error("FineMatchFast", "No histogram being pointed to");
      return false;
   }

   // Check to see that the histogram isn't empty
   if(hist1->GetEntries() < 1 || hist2->GetEntries() < 1) {
      Error("FineMatchFast", "Histogram is empty");
      return false;
   }

   // See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   Double_t  gain   = 0.;
   Double_t  offset = 0.;
   TChannel* chan   = TChannel::GetChannelByNumber(channelNum);
   if(chan == nullptr) {
      if(channelNum != 9999) {
         Warning("FineMatchFast", "Channel Number %d does not exist in current memory.", channelNum);
      }
      if(GetFitFunction() != nullptr) {
         gain   = GetParameter(1);
         offset = GetParameter(0);
      } else {
         Error("Fine Match", "There needs to be a coarse gain set to do a fine gain");
         return false;
      }
   } else {
      SetChannel(chan);
      // The first thing we need to do is "un-gain correct" the centroids of the TPeaks.
      // What we are actually doing is applying the recirpocal gain to the energy of the real peak
      // to figure out where the centroid of the charge is spectrum is roughly going to be
      // First read in the rough gain coefficients
      std::vector<Float_t> roughCoeffs = chan->GetENGCoeff();
      gain                             = roughCoeffs.at(1);
      offset                           = roughCoeffs.at(0);
   }

   // The reason I'm using TPeak here is we might want to gain match "TPhotopeaks", or "TElectronPeaks", or
   //"TCrazyNonGaussian" Peak. All we care about is that it has a centroid.
   if((peak1 == nullptr) || (peak2 == nullptr)) {
      Error("FineMatchFast", "No TPeak being pointed to");
      return false;
   }

   // Set the channel number
   Set(2);

   // Find the energy of the peak that we want to use
   std::array<Double_t, 2> energy = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};
   std::cout << peak1->GetParameter("centroid") << " ENERGIES " << energy[1] << std::endl;
   // Offsets are very small right now so I'm not including them until they become a problem.
   peak1->SetParameter("centroid", (energy[0] - offset) / gain);
   peak2->SetParameter("centroid", (energy[1] - offset) / gain);
   // Change the range for the fit to be in the gain corrected spectrum

   peak1->SetRange((peak1->GetXmin() - offset) / gain, (peak1->GetXmax() - offset) / gain);
   peak2->SetRange((peak2->GetXmin() - offset) / gain, (peak2->GetXmax() - offset) / gain);

   // The gains won't be perfect, so we need to search for the peak within a range.
   hist1->GetXaxis()->SetRangeUser(peak1->GetXmin() - 20., peak1->GetXmax() + 20.);
   TSpectrum spec;
   Int_t     nFound = spec.Search(hist1, 2, "", 0.3);

   for(int x = 0; x < nFound; x++) {
      std::cout << spec.GetPositionX()[x] << std::endl;
   }
   Double_t closestPeak = 0;
   Double_t closestDiff = 10000;
   for(int x = 0; x < nFound; x++) {
      if(fabs(peak1->GetCentroid() - spec.GetPositionX()[x]) < closestDiff) {
         closestPeak = spec.GetPositionX()[x];
         closestDiff = fabs(peak1->GetCentroid() - spec.GetPositionX()[x]);
      }
   }

   Double_t rangeWidth = (peak1->GetXmax() - peak1->GetXmin()) / 2.;
   peak1->SetParameter("centroid", closestPeak);
   peak1->SetRange(peak1->GetCentroid() - rangeWidth, peak1->GetCentroid() + rangeWidth);
   std::cout << "Centroid Guess " << peak1->GetCentroid() << std::endl;
   std::cout << "Range Low " << peak1->GetXmin() << " " << peak1->GetXmax() << std::endl;

   closestPeak = 0;
   closestDiff = 10000;
   hist2->GetXaxis()->SetRangeUser(peak2->GetXmin() - 20., peak2->GetXmax() + 20.);
   TSpectrum spec2;
   nFound = spec2.Search(hist2, 2, "", 0.3);   // Search the next histogram
   for(int x = 0; x < nFound; x++) {
      std::cout << spec2.GetPositionX()[x] << std::endl;
   }

   for(int x = 0; x < nFound; x++) {
      if(fabs(peak2->GetCentroid() - spec2.GetPositionX()[x]) < closestDiff) {
         closestPeak = spec2.GetPositionX()[x];
         closestDiff = fabs(peak2->GetCentroid() - spec2.GetPositionX()[x]);
      }
   }
   Double_t rangeWidth2 = (peak2->GetXmax() - peak2->GetXmin()) / 2.;
   peak2->SetParameter("centroid", closestPeak);
   peak2->SetRange(peak2->GetCentroid() - rangeWidth2, peak2->GetCentroid() + rangeWidth2);

   std::cout << "Centroid Guess " << peak2->GetCentroid() << std::endl;
   std::cout << "Range High " << peak2->GetXmin() << " " << peak2->GetXmax() << std::endl;

   hist1->GetXaxis()->UnZoom();
   hist2->GetXaxis()->UnZoom();
   peak1->Fit(hist1, "MS+");
   peak2->Fit(hist2, "MS+");

   hist1->Draw();
   peak1->Draw("same");
   peak2->Draw("same");

   std::array<Double_t, 2> centroid = {peak1->GetCentroid(), peak2->GetCentroid()};

   // Put the peaks in order for ease (if the user put them in the wrong order)
   // Apparantly there is a TGraph Sort method. Might look into this later.
   if(energy[0] > energy[1]) {
      std::swap(energy[0], energy[1]);
      std::swap(centroid[0], centroid[1]);
   }

   SetPoint(0, centroid[0], energy[0]);
   SetPoint(1, centroid[1], energy[1]);

   auto* gainFit = new TF1("gain", "pol1");

   TFitResultPtr res = Fit(gainFit, "SC0");
   SetFitFunction(GetFunction("gain"));   // Have to do this because I want to delete gainFit
   fGainCoeffs[0] = res->Parameter(0);
   fGainCoeffs[1] = res->Parameter(1);

   delete gainFit;

   fCoarseMatch = false;
   return true;
}

Bool_t TGainMatch::FineMatchFast(TH1* hist, TPeak* peak1, TPeak*, Int_t channelNum)
{
   // You need to pass a TPeak with the centroid and range set to the real energy centroid and ranges.
   // The function uses the coarse gain parameters to find the peak and gain matches the raw spectrum.
   // This is more useful as it allows a script to find all of the peaks.
   return FineMatchFast(hist, peak1, hist, peak1, channelNum);
}

Bool_t TGainMatch::FineMatchFast(TH1* hist, Double_t energy1, Double_t energy2, Int_t channelNum)
{
   // Performs fine gain matching on one histogram once we have set the rough energy
   // coefficients.
   return FineMatchFast(hist, energy1, hist, energy2, channelNum);
}

Bool_t TGainMatch::FineMatchFast(TH1* hist1, Double_t energy1, TH1* hist2, Double_t energy2, Int_t channelNum)
{
   // Performs fine gain matching on two histograms once we have set the rough energy
   // coefficients. You use this if you have a two different sources giving your full range
   // of energy. This histograms should be binned the same. NOT IMPLEMENTED

   // using an automatic range of 10 keV for testing purposes.
   auto* peak1 = new TPeak(energy1, energy1 - 10.0, energy1 + 10.0);
   auto* peak2 = new TPeak(energy2, energy2 - 10.0, energy2 + 10.0);

   Bool_t result = FineMatchFast(hist1, peak1, hist2, peak2, channelNum);

   delete peak1;
   delete peak2;
   return result;
   return false;
}

void TGainMatch::WriteToChannel() const
{
   if(GetChannel() == nullptr) {
      Error("WriteToChannel", "No Channel Set");
      return;
   }
   GetChannel()->DestroyENGCal();
   std::cout << "Writing to channel " << GetChannel()->GetNumber() << std::endl;
   std::cout << "p0 = " << GetParameter(0) << " \t p1 = " << GetParameter(1) << std::endl;
   // Set the energy parameters based on the fitted gains.
   GetChannel()->AddENGCoefficient(static_cast<float>(GetParameter(0)));
   GetChannel()->AddENGCoefficient(static_cast<float>(GetParameter(1)));
}

void TGainMatch::Print(Option_t*) const
{
   std::cout << std::endl;
   std::cout << "GainMatching: " << std::endl;
   if(fCoarseMatch) {
      std::cout << "COARSE" << std::endl;
   } else {
      std::cout << "FINE" << std::endl;
   }

   if(fAligned) {
      std::cout << "Aligned" << std::endl;
   } else {
      std::cout << "NOT Aligned" << std::endl;
   }

   std::cout << "Gain Coefficients: " << fGainCoeffs[0] << "\t" << fGainCoeffs[1] << std::endl;
   std::cout << "Align Coefficients: " << fAlignCoeffs[0] << "\t" << fAlignCoeffs[1] << std::endl;

   TCal::Print();
}

Bool_t TGainMatch::CoarseMatchAll(TCalManager* cm, TH2* mat, Double_t, Double_t)
{
   // If you supply this function with a matrix of Channel vs. energy it will automatically slice,
   // and figure out the coarse gains. I might add a channel range option later
   std::vector<Int_t> badlist;
   auto*              gm = new TGainMatch;
   if(cm == nullptr) {
      gm->Error("CoarseMatchAll", "CalManager Pointer is nullptr");
      return false;
   }
   if(mat == nullptr) {
      gm->Error("CoarseMatchAll", "TH2 Pointer is nullptr");
      return false;
   }
   // Find the range of channels provided
   Int_t first_chan = mat->GetXaxis()->GetFirst();
   Int_t last_chan  = mat->GetXaxis()->GetLast();
   // The first thing we need to do is slice the matrix into it's channel vs energy.
   for(int chan = first_chan; chan <= last_chan; chan++) {
      gm->Clear();
      std::cout << std::endl
                << "Now fitting channel " << chan - 1 << std::endl;
      auto* h1 = static_cast<TH1D*>(mat->ProjectionY(Form("Channel%d", chan), chan, chan, "o"));
      std::cout << "BIN WIDTH " << h1->GetXaxis()->GetBinWidth(h1->GetXaxis()->GetFirst() + 1) << std::endl;
      if(h1->Integral() < 100) {
         continue;
      }

      if(!(gm->CoarseMatch(h1, chan - 1))) {
         badlist.push_back(chan - 1);
         continue;
      }
      gm->SetName(Form("gm_chan_%d", chan - 1));
      cm->AddToManager(gm, chan - 1);
   }
   if(!badlist.empty()) {
      std::cout << "The following channels did not gain match properly: ";
   }
   for(int i : badlist) {
      std::cout << i << "\t";
   }

   delete gm;

   return true;
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2* mat1, Double_t energy1, TH2* mat2, Double_t energy2)
{

   // using an automatic range of 10 keV for testing purposes.
   // We need to include bin width likely?
   auto* peak1 = new TPeak(energy1, energy1 - 10.0, energy1 + 10.0);
   auto* peak2 = new TPeak(energy2, energy2 - 10.0, energy2 + 10.0);

   Bool_t result = TGainMatch::FineMatchFastAll(cm, mat1, peak1, mat2, peak2);

   delete peak1;
   delete peak2;
   return result;
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2* mat, Double_t energy1, Double_t energy2)
{
   return TGainMatch::FineMatchFastAll(cm, mat, energy1, mat, energy2);
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2* mat1, TPeak* peak1, TH2* mat2, TPeak* peak2)
{
   // If you supply this function with a matrix of Channel vs. energy it will automatically slice,
   // and figure out the fine gains. I might add a channel range option later
   std::vector<Int_t> badlist;
   auto*              gm = new TGainMatch;
   if(cm == nullptr) {
      gm->Error("FineMatchFastAll", "CalManager Pointer is nullptr");
      return false;
   }
   if((mat1 == nullptr) || (mat2 == nullptr)) {
      gm->Error("FineMatchFastAll", "TH2 Pointer is nullptr");
      return false;
   }
   if((peak1 == nullptr) || (peak2 == nullptr)) {
      gm->Error("FineMatchFastAll", "No TPeak being pointed to");
      return false;
   }

   // Find the range of channels provided, we want to use the largest range provided
   Int_t first_chan1 = mat1->GetXaxis()->GetFirst();
   Int_t last_chan1  = mat1->GetXaxis()->GetLast();
   Int_t first_chan2 = mat2->GetXaxis()->GetFirst();
   Int_t last_chan2  = mat2->GetXaxis()->GetLast();
   Int_t first_chan  = std::min(first_chan1, first_chan2);
   Int_t last_chan   = std::max(last_chan1, last_chan2);
   // The first thing we need to do is slice the matrix into it's channel vs energy.
   for(int chan = first_chan; chan <= last_chan; chan++) {
      gm->Clear();
      // Make a copy of the TPeaks so that we can fit each of them
      // TPeak* copyPeak1 = static_cast<TPeak*>(peak1->Clone());//Clone creates smart pointers so these will be
      // dealoccated automatically.
      // TPeak* copyPeak2 = static_cast<TPeak*>(peak2->Clone());
      auto* copyPeak1 = new TPeak(*peak1);
      auto* copyPeak2 = new TPeak(*peak2);

      std::cout << std::endl
                << "Now fitting channel: " << chan - 1 << std::endl;
      auto* h1 = static_cast<TH1D*>(mat1->ProjectionY(Form("Channel%d_mat1", chan - 1), chan, chan, "o"));
      auto* h2 = static_cast<TH1D*>(mat2->ProjectionY(Form("Channel%d_mat2", chan - 1), chan, chan, "o"));
      if(h1->Integral() < 100 || h2->Integral() < 100) {
         gm->Warning("FineMatchFastAll", "Empty channel = %d", chan - 1);
         continue;
      }
      if(!(gm->FineMatchFast(h1, copyPeak1, h2, copyPeak2, chan - 1))) {
         badlist.push_back(chan - 1);
         continue;
      }
      cm->AddToManager(gm);

      delete copyPeak1;
      delete copyPeak2;
   }
   if(!badlist.empty()) {
      std::cout << "The following channels did not gain match properly: ";
   }
   for(int i : badlist) {
      std::cout << i << "\t";
   }

   delete gm;

   return true;
}

Bool_t TGainMatch::FineMatchFastAll(TCalManager* cm, TH2* mat, TPeak* peak1, TPeak* peak2)
{
   return TGainMatch::FineMatchFastAll(cm, mat, peak1, mat, peak2);
}

void TGainMatch::Clear(Option_t*)
{
   fCoarseMatch    = true;
   fAligned        = false;
   fAlignCoeffs[0] = 0.0;
   fAlignCoeffs[1] = 1.0;
   fGainCoeffs[0]  = 0.0;
   fGainCoeffs[1]  = 1.0;
   TCal::Clear();
}

Bool_t TGainMatch::Align(TH1* test, TH1* hist, Int_t low_range, Int_t high_range)
{
   // Minimizes the chi^2 between the bin contents of the test histogram and the bin contents of the histogram to be
   // matched'
   if(!((test != nullptr) && (hist != nullptr))) {
      std::cout << "Unassigned histogram" << std::endl;
      return false;
   }
   fHist = hist;   // Need this histogram to be seen by ftotal...Don't know how else to do this right now.
   //  TF1* tmpfunc = new
   //  TF1("tmpfunc",this,&TGainMatch::HistCompare,test->GetMinimumStored()+1,test->GetMaximumStored()-1,3);

   /* //Initial Guesses

      TSpectrum stest,shist;
      TGraph guess_graph;
      Int_t nFoundtest = stest.Search(test);
      Int_t nFoundhist = shist.Search(hist);
      TF1* shift_guess = new TF1("shift_guess","pol1");
      if(nFoundtest >1 && nFoundhist > 1) {
      stest.GetPositionX()[0]/shist.GetPositionX()[0];
      guess_graph.SetPoint(0,shist.GetPositionX()[0],stest.GetPositionX()[0]);
      guess_graph.SetPoint(1,shist.GetPositionX()[1],stest.GetPositionX()[1]);

      guess_graph.Print();

      TFitResultPtr res = guess_graph.Fit(shift_guess,"SC0");
      }
      */

   TF1* tmpfunc = new TF1("tmpfunc", this, &TGainMatch::HistCompare, low_range, high_range, 3);
   tmpfunc->SetNpx(10000);
   tmpfunc->SetParameters(1.0, 1.0, 1.0);

   //  hist->Sumw2();

   /* for(int i =0; i<hist->GetXaxis()->GetNbins();i++) {
   // hist->SetBinError(i,TMath::Sqrt(TMath::Sqrt(hist->GetBinContent(i))));
   hist->SetBinError(i,1./(1.+hist->GetBinContent(i)));
   }
   */
   // ftot->SetParLimits(0,.3*norm,norm);

   // const char* minName = "Minuit2";
   // const char* algoName = "Scan";
   // ROOT::Math::MinimizerOptions::SetDefaultMinimizer("TMinuit2","Simplex");
   //   TVirtualFitter::SetDefaultFitter("Fumili"); //USE COMBINATION!!!!
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2", "Combination");
   TVirtualFitter::SetPrecision(1.0e-10);
   TVirtualFitter::SetMaxIterations(10000);
   TFitResultPtr res = test->Fit("tmpfunc", "RSILV");
   fAlignCoeffs[0]   = res->Parameter(1);
   fAlignCoeffs[1]   = res->Parameter(2);
   std::cout << "Chi2: " << res->Chi2() / res->Ndf() << std::endl;
   if(std::abs(res->Parameter(0) - 1.00) > 20.) {
      return false;
   }
   if(std::abs(res->Parameter(2) - 1.00) > 5.) {
      return false;
   }
   fAligned = true;
   // delete shift_guess;
   return true;
}

Bool_t TGainMatch::AlignAll(TCalManager* cm, TH1* hist, TH2* mat, Int_t low_range, Int_t high_range)
{
   std::vector<Int_t> badlist;
   auto*              gm = new TGainMatch;
   if(cm == nullptr) {
      gm->Error("AlignAll", "CalManager Pointer is nullptr");
      return false;
   }
   if(mat == nullptr) {
      gm->Error("AlignAll", "TH2 Pointer is nullptr");
      return false;
   }
   if(hist == nullptr) {
      gm->Error("AlignAll", "TH1 Pointer is nullptr");
      return false;
   }
   // Find the range of channels provided
   Int_t first_chan = mat->GetXaxis()->GetFirst();
   // Int_t last_chan  = mat->GetXaxis()->GetLast();
   // The first thing we need to do is slice the matrix into it's channel vs energy.
   //	for(int chan=first_chan; chan<=last_chan;chan++) {
   for(int chan = first_chan; chan <= 2; chan++) {
      gm->Clear();
      std::cout << std::endl
                << "Now fitting channel: " << chan << std::endl;
      auto* h1 = static_cast<TH1D*>(mat->ProjectionY(Form("Channel%d", chan), chan + 1, chan + 1, "o"));
      std::cout << "BIN WIDTH " << h1->GetXaxis()->GetBinWidth(h1->GetXaxis()->GetFirst() + 1) << std::endl;
      if(h1->Integral() < 100) {
         continue;
      }

      if(!(gm->Align(hist, h1, low_range, high_range))) {
         badlist.push_back(chan);
         continue;
      }
      cm->AddToManager(gm);
   }
   if(!badlist.empty()) {
      std::cout << "The following channels did not gain match properly: ";
   }
   for(int i : badlist) {
      std::cout << i << "\t";
   }

   delete gm;

   return true;
}

Bool_t TGainMatch::FineMatchAll(TCalManager* cm, TH2* charge_mat, TH2* eng_mat, Int_t testchan, Double_t energy1,
                                Double_t energy2, Int_t low_range, Int_t high_range)
{
   // Should be run to correct drifting gains. Requires a previously gain matched spectrum, and ideally a roughly energy
   // calibrated matrix

   auto* gm = new TGainMatch;
   if(cm == nullptr) {
      gm->Error("FineMatchAll", "CalManager Pointer is nullptr");
      return false;
   }
   if((charge_mat == nullptr) || (eng_mat == nullptr)) {
      gm->Error("FineMatchAll", "TH2 Pointer is nullptr");
      return false;
   }
   auto binwidth = static_cast<Int_t>(0.5 + 1. / eng_mat->GetYaxis()->GetBinWidth(100));
   eng_mat->RebinY(binwidth);

   std::vector<Int_t> badlist;
   // Find the range of channels provided, we want to use the largest range provided
   Int_t first_chan = eng_mat->GetXaxis()->GetFirst();
   Int_t last_chan  = eng_mat->GetXaxis()->GetLast();
   // The first thing we need to do is slice the matrix into it's channel vs energy.
   auto* testhist = static_cast<TH1D*>(eng_mat->ProjectionY(Form("Test%d_mat", testchan), testchan + 1, testchan + 1, "o"));

   for(int chan = first_chan; chan <= last_chan; chan++) {
      std::cout << std::endl
                << "Now fitting channel: " << chan - 1 << std::endl;
      auto* chargeh = static_cast<TH1D*>(charge_mat->ProjectionY(Form("Charge%d_mat", chan - 1), chan, chan, "o"));
      auto* engh    = static_cast<TH1D*>(eng_mat->ProjectionY(Form("Energy%d_mat", chan - 1), chan, chan, "o"));
      if(chargeh->Integral() < 100 || chargeh->Integral() < 100) {
         gm->Warning("FineMatchAll", "Empty channel = %d", chan - 1);
         continue;
      }
      // Needs to align before matching
      if(!(gm->FineMatch(engh, testhist, chargeh, energy1, energy2, low_range, high_range, chan - 1))) {
         badlist.push_back(chan - 1);
         continue;
      }
      cm->AddToManager(gm);
   }
   if(!badlist.empty()) {
      std::cout << "The following channels did not gain match properly: ";
   }
   for(int i : badlist) {
      std::cout << i << "\t";
   }

   delete testhist;
   delete gm;

   return true;
}

Bool_t TGainMatch::FineMatch(TH1* energyHist, TH1* testhist, TH1* chargeHist, Double_t energy1, Double_t energy2,
                             Int_t low_range, Int_t high_range, Int_t channelNum)
{
   // Should be run to correct drifting gains. Requires a previously gain matched spectrum, and ideally a roughly energy
   // calibrated spectrum
   if(!Align(testhist, energyHist, low_range, high_range)) {
      return false;
   }
   TH1* hist2 = chargeHist;   // Cheating for easier modification later
   if((chargeHist == nullptr) || (testhist == nullptr) || (energyHist == nullptr)) {
      Error("FineMatch", "No histogram being pointed to");
      return false;
   }

   // Check to see that the histogram isn't empty
   if(chargeHist->GetEntries() < 1 || testhist->GetEntries() < 1 || energyHist->GetEntries() < 1) {
      Error("FineMatchFast", "Histogram is empty");
      return false;
   }

   // See if the channel exists. There is no point in finding the gains if we don't have anywhere to write it
   Double_t  gain   = 0.;
   Double_t  offset = 0.;
   TChannel* chan   = TChannel::GetChannelByNumber(channelNum);
   if(chan == nullptr) {
      if(channelNum != 9999) {
         Warning("FineMatch", "Channel Number %d does not exist in current memory.", channelNum);
      }
      if(GetFitFunction() != nullptr) {
         gain   = GetParameter(1);
         offset = GetParameter(0);
      } else {
         Error("FineMatch", "There needs to be a coarse gain set to do a fine gain");
         return false;
      }
   } else {
      SetChannel(chan);
      // The first thing we need to do is "un-gain correct" the centroids of the TPeaks.
      // What we are actually doing is applying the recirpocal gain to the energy of the real peak
      // to figure out where the centroid of the charge is spectrum is roughly going to be
      // First read in the rough gain coefficients
      std::vector<Float_t> roughCoeffs = chan->GetENGCoeff();
      gain                             = roughCoeffs.at(1);
      offset                           = roughCoeffs.at(0);
   }

   auto* peak1 = new TPeak(energy1, energy1 - 15.0, energy1 + 15.0);
   auto* peak2 = new TPeak(energy2, energy2 - 15.0, energy2 + 15.0);

   // Set the channel number
   // Graph()->Set(2);
   Set(2);

   // Find the energy of the peak that we want to use
   std::array<Double_t, 2> energy = {peak1->GetParameter("centroid"), peak2->GetParameter("centroid")};
   std::cout << peak1->GetParameter("centroid") << " ENERGIES " << energy[1] << std::endl;
   // Offsets are very small right now so I'm not including them until they become a problem.
   //   peak1->SetParameter("centroid",((energy[0]-offset)/gain)*fAlignCoeffs[1] + fAlignCoeffs[0]);
   //   peak2->SetParameter("centroid",((energy[0]-offset)/gain)*fAlignCoeffs[1] + fAlignCoeffs[0]);
   peak1->SetParameter("centroid", (energy[0] * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain);
   peak2->SetParameter("centroid", (energy[1] * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain);
   // Change the range for the fit to be in the gain corrected spectrum

   std::cout << " Should be: " << ((energy[0] * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain) << " from (" << energy[0] << " * " << fAlignCoeffs[1] << " + " << fAlignCoeffs[0] << " - " << offset << ")/" << gain << std::endl;
   std::cout << " Should be: " << ((energy[1] * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain) << " from (" << energy[1] << " * " << fAlignCoeffs[1] << " + " << fAlignCoeffs[0] << " - " << offset << ")/" << gain << std::endl;

   //   peak1->SetRange(((peak1->GetXmin()-offset)/gain)*fAlignCoeffs[1] +
   //   fAlignCoeffs[0],((peak1->GetXmax()-offset)/gain)*fAlignCoeffs[1] + fAlignCoeffs[0]);
   //   peak2->SetRange(((peak2->GetXmin()-offset)/gain)*fAlignCoeffs[1] +
   //   fAlignCoeffs[0],((peak2->GetXmax()-offset)/gain)*fAlignCoeffs[1] + fAlignCoeffs[0]);
   peak1->SetRange(((peak1->GetXmin() * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain),
                   ((peak1->GetXmax() * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain));
   peak2->SetRange(((peak2->GetXmin() * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain),
                   ((peak2->GetXmax() * fAlignCoeffs[1] + fAlignCoeffs[0] - offset) / gain));

   // The gains won't be perfect, so we need to search for the peak within a range.
   TSpectrum spec;
   chargeHist->GetXaxis()->SetRangeUser(peak1->GetXmin() - 20., peak1->GetXmax() + 20.);
   Int_t nFound = spec.Search(chargeHist, 2, "", 0.3);

   for(int x = 0; x < nFound; x++) {
      std::cout << spec.GetPositionX()[x] << std::endl;
   }
   Double_t closestPeak = 0;
   Double_t closestDiff = 10000;
   for(int x = 0; x < nFound; x++) {
      if(fabs(peak1->GetCentroid() - spec.GetPositionX()[x]) < closestDiff) {
         closestPeak = spec.GetPositionX()[x];
         closestDiff = fabs(peak1->GetCentroid() - spec.GetPositionX()[x]);
      }
   }

   Double_t rangeWidth = (peak1->GetXmax() - peak1->GetXmin()) / 2.;
   peak1->SetParameter("centroid", closestPeak);
   peak1->SetRange(peak1->GetCentroid() - rangeWidth, peak1->GetCentroid() + rangeWidth);
   std::cout << "Centroid Guess " << peak1->GetCentroid() << std::endl;
   std::cout << "Range Low " << peak1->GetXmin() << " " << peak1->GetXmax() << std::endl;

   closestDiff = 10000;
   TSpectrum spec2;
   hist2->GetXaxis()->SetRangeUser(peak2->GetXmin() - 20., peak2->GetXmax() + 20.);
   std::cout << "RANGE: " << peak2->GetXmin() - 20. << " " << peak2->GetXmax() + 20. << std::endl;
   nFound = spec2.Search(hist2, 2, "", 0.3);   // Search the next histogram
   if(nFound == 0) {
      std::cout << "RANGE: " << peak2->GetXmin() - 40. << " " << peak2->GetXmax() + 40. << std::endl;
      nFound = spec2.Search(hist2, 2, "", 0.3);   // Search the next histogram
   }

   for(int x = 0; x < nFound; x++) {
      std::cout << spec2.GetPositionX()[x] << std::endl;
   }

   Double_t largestPeak = spec2.GetPositionX()[0];
   for(int x = 0; x < nFound; x++) {
      if(fabs(peak2->GetCentroid() - spec2.GetPositionX()[x]) < closestDiff) {
         closestDiff = fabs(peak2->GetCentroid() - spec2.GetPositionX()[x]);
      }
   }
   Double_t rangeWidth2 = (peak2->GetXmax() - peak2->GetXmin()) / 2.;
   peak2->SetParameter("centroid", largestPeak);
   peak2->SetRange(peak2->GetCentroid() - rangeWidth2, peak2->GetCentroid() + rangeWidth2);

   std::cout << "Centroid Guess " << peak2->GetCentroid() << std::endl;
   std::cout << "Range High " << peak2->GetXmin() << " " << peak2->GetXmax() << std::endl;

   chargeHist->GetXaxis()->UnZoom();
   hist2->GetXaxis()->UnZoom();
   peak1->InitParams(chargeHist);
   peak2->InitParams(chargeHist);
   peak1->SetParameter("sigma", TMath::Sqrt(9.0 + 4. * peak1->GetParameter("centroid") / 1000.) / 2.35);
   peak2->SetParameter("sigma", TMath::Sqrt(9.0 + 4. * peak2->GetParameter("centroid") / 1000.) / 2.35);
   peak1->Fit(chargeHist, "MSL+");
   peak2->Fit(hist2, "MSL+");

   chargeHist->Draw();
   peak1->Draw("same");
   peak2->Draw("same");

   std::array<Double_t, 2> centroid = {peak1->GetCentroid(), peak2->GetCentroid()};

   // Put the peaks in order for ease (if the user put them in the wrong order)
   // Apparantly there is a TGraph Sort method. Might look into this later.
   if(energy[0] > energy[1]) {
      std::swap(energy[0], energy[1]);
      std::swap(centroid[0], centroid[1]);
   }

   SetPoint(0, centroid[0], energy[0]);
   SetPoint(1, centroid[1], energy[1]);

   auto* gainFit = new TF1("gain", "pol1");

   TFitResultPtr res = Fit(gainFit, "SC0");
   SetFitFunction(GetFunction("gain"));   // Have to do this because I want to delete gainFit
   fGainCoeffs[0] = res->Parameter(0);
   fGainCoeffs[1] = res->Parameter(1);

   delete gainFit;

   fCoarseMatch = false;
   delete peak1;
   delete peak2;
   return true;
}

Double_t TGainMatch::HistCompare(Double_t* x, Double_t* par)   // NOLINT(readability-non-const-parameter)
{
   Int_t    bin     = fHist->GetXaxis()->FindBin(x[0] * par[2] + par[1]);
   Double_t content = fHist->GetBinContent(bin);

   return par[0] * content;
}
