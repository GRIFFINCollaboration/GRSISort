#include "ExampleEventHelper.hh"

void ExampleEventHelper::CreateHistograms(unsigned int slot)
{
   // try and get the cycle length if we have a PPG provided
   // only necessary for the first worker, this is shared with all other workers
   if(fPpg != nullptr) {
      // the ODB cycle length is in microseconds!
      fCycleLength = fPpg->OdbCycleLength();
      if(slot == 0) {
         std::stringstream str;
         str << "Got ODB cycle length " << fCycleLength << " us = " << fCycleLength / 1e6 << " s" << std::endl;
         std::cerr << str.str();
      }
   } else if(slot == 0) {
      std::stringstream str;
      str << DRED << "No ppg provided, can't fill cycle spectra!" << RESET_COLOR << std::endl;
      std::cerr << str.str();
   }

   // some variables to easily change range and binning for multiple histograms at once
   int    energyBins = 10000;
   double lowEnergy  = 0.;
   double highEnergy = 2000.;

   fH2[slot]["zdsMultGriffinMult"] = new TH2I("zdsMultGriffinMult", "ZDS multiplicity vs. GRIFFIN multiplicity (unsuppressed)", 65, -0.5, 64.5, 10, -0.5, 9.5);

   // unsuppressed spectra
   fH1[slot]["griffinE"] = new TH1F("griffinE", Form("Unsuppressed griffin energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   // suppressed spectra
   fH1[slot]["griffinESupp"] = new TH1F("griffinESupp", Form("Suppressed griffin energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   // unsuppressed addback spectra
   fH1[slot]["griffinEAddback"] = new TH1F("griffinEAddback", Form("Unsuppressed griffin addback energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   // suppressed addback spectra
   fH1[slot]["griffinESuppAddback"]             = new TH1F("griffinESuppAddback", Form("Suppressed griffin addback energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   fH1[slot]["griffinESuppAddbackBeta"]         = new TH1F("griffinESuppAddbackBeta", Form("Suppressed griffin addback energy w/ #beta-tag;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   fH2[slot]["griffinESuppAddbackMatrixBeta"]   = new TH2F("griffinESuppAddbackMatrixBeta", "Suppressed griffin addback energy matrix w/ #beta-tag;energy [keV];energy [keV]", energyBins / 5, lowEnergy, highEnergy, energyBins / 5, lowEnergy, highEnergy);
   fH2[slot]["griffinESuppAddbackMatrixBetaBg"] = new TH2F("griffinESuppAddbackMatrixBetaBg", "Suppressed griffin addback energy matrix w/ #beta-tag (time random BG);energy [keV];energy [keV]", energyBins / 5, lowEnergy, highEnergy, energyBins / 5, lowEnergy, highEnergy);

   // timing spectra
   fH2[slot]["griffinZdsTS"]       = new TH2F("griffinZdsTS", "GRIFFIN crystal vs. GRIFFIN-ZDS timestamp difference (suppressed addback);#DeltaTS_{GRIFFIN-ZDS}", 200, -1000., 1000., 64, 0.5, 64.5);
   fH2[slot]["griffinZdsTime"]     = new TH2F("griffinZdsTime", "GRIFFIN crystal vs. GRIFFIN-ZDS timing (suppressed addback);#Deltat_{GRIFFIN-ZDS}", 2000, -1000., 1000., 64, 0.5, 64.5);
   fH2[slot]["griffinGriffinTS"]   = new TH2F("griffinGriffinTS", "GRIFFIN crystal vs. GRIFFIN-GRIFFIN timestamp difference (suppressed addback);#DeltaTS_{GRIFFIN-GRIFFIN}", 2000, -1000., 1000., 64, 0.5, 64.5);
   fH2[slot]["griffinGriffinTime"] = new TH2F("griffinGriffinTime", "GRIFFIN crystal vs. GRIFFIN-GRIFFIN timing (suppressed addback);#Deltat_{GRIFFIN-GRIFFIN}", 2000, -1000., 1000., 64, 0.5, 64.5);

   // cycle spectra
   if(fCycleLength > 0.) {
      fH2[slot]["griffinCycle"] = new TH2F("griffinCycle", "GRIFFIN suppressed addback energy w/ #beta-tag vs. time in cycle;time in cycle [s];energy [keV]", 100 * fCycleLength / 1e6, 0., fCycleLength / 1e6, energyBins / 5, lowEnergy, highEnergy);
      fH1[slot]["zdsCycle"]     = new TH1F("zdsCycle", "ZDS hits in cycle;time in cycle [s]", 100 * fCycleLength / 1e6, 0., fCycleLength / 1e6);
   }
}

// Coincidences Gates
bool PromptCoincidence(TGriffinHit* h1, TGriffinHit* h2)
{   // Griffin-Griffin
   return -250. <= h2->GetTime() - h1->GetTime() && h2->GetTime() - h1->GetTime() <= 250.;
}
bool TimeRandom(TGriffinHit* h1, TGriffinHit* h2)
{
   return (-500. <= h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() <= -250.) || (250. <= h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() <= 500.);
}
bool PromptCoincidence(TGriffinHit* h1, TZeroDegreeHit* h2)
{   // Griffin-Zds
   return -200. <= h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() <= 20.;
}
bool TimeRandom(TGriffinHit* h1, TZeroDegreeHit* h2)
{
   return (-310. <= h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() <= -200.) || (20. <= h1->GetTime() - h2->GetTime() && h1->GetTime() - h2->GetTime() <= 130.);
}

// TODO: Change the function arguments to match the detectors you want to use and the declaration in the header file!
void ExampleEventHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo, TZeroDegree& zds)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   // multiplicities
   fH2[slot].at("zdsMultGriffinMult")->Fill(grif.GetMultiplicity(), zds.GetMultiplicity());

   // loop over unsuppressed griffin hits
   for(int g = 0; g < grif.GetMultiplicity(); ++g) {
      auto grif1 = grif.GetGriffinHit(g);
      fH1[slot].at("griffinE")->Fill(grif1->GetEnergy());
   }

   // loop over suppressed griffin hits
   for(int g = 0; g < grif.GetSuppressedMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedHit(g);
      fH1[slot].at("griffinESupp")->Fill(grif1->GetEnergy());
   }

   // loop over unsuppressed griffin addback hits
   for(int g = 0; g < grif.GetAddbackMultiplicity(); ++g) {
      auto grif1 = grif.GetAddbackHit(g);
      fH1[slot].at("griffinEAddback")->Fill(grif1->GetEnergy());
   }

   // loop over suppressed griffin addback hits
   for(int g = 0; g < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedAddbackHit(g);
      fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy());
      // we use a flag to check if there is any beta in coincidence with this gamma-ray and only fill afterwards
      // otherwise we could double-count gamma-rays that are in coincidence with more than one beta
      // e.g. because of double triggering of SCEPTAR/ZDS where we get a second trigger from the tail of the real signal
      bool promptBeta = false;
      for(int z = 0; z < zds.GetMultiplicity(); ++z) {
         auto zds1 = zds.GetZeroDegreeHit(z);
         fH2[slot].at("griffinZdsTS")->Fill(grif1->GetTimeStampNs() - zds1->GetTimeStampNs(), grif1->GetArrayNumber());
         fH2[slot].at("griffinZdsTime")->Fill(grif1->GetTime() - zds1->GetTime(), grif1->GetArrayNumber());
         if(PromptCoincidence(grif1, zds1)) promptBeta = true;
      }
      if(promptBeta) {
         fH1[slot].at("griffinESuppAddbackBeta")->Fill(grif1->GetEnergy());
         if(fCycleLength > 0.) {
            fH2[slot].at("griffinCycle")->Fill(std::fmod(grif1->GetTime(), fCycleLength), grif1->GetEnergy());
         }
         for(int g2 = g + 1; g2 < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g2) {
            auto grif2 = grif.GetSuppressedAddbackHit(g2);
            fH2[slot].at("griffinGriffinTS")->Fill(grif1->GetTimeStampNs() - grif2->GetTimeStampNs(), grif1->GetArrayNumber());
            fH2[slot].at("griffinGriffinTime")->Fill(grif1->GetTime() - grif2->GetTime(), grif1->GetArrayNumber());
            if(PromptCoincidence(grif1, grif2)) {
               // fill twice to get a symmetric matrix
               fH2[slot].at("griffinESuppAddbackMatrixBeta")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at("griffinESuppAddbackMatrixBeta")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
            } else if(TimeRandom(grif1, grif2)) {
               // fill twice to get a symmetric matrix
               fH2[slot].at("griffinESuppAddbackMatrixBetaBg")->Fill(grif1->GetEnergy(), grif2->GetEnergy());
               fH2[slot].at("griffinESuppAddbackMatrixBetaBg")->Fill(grif2->GetEnergy(), grif1->GetEnergy());
            }
         }
      }
   }
   if(fCycleLength > 0.) {
      for(int z = 0; z < zds.GetMultiplicity(); ++z) {
         auto zds1 = zds.GetZeroDegreeHit(z);
         fH1[slot].at("zdsCycle")->Fill(std::fmod(zds1->GetTime(), fCycleLength));
      }
   }
}

void ExampleEventHelper::EndOfSort(std::shared_ptr<std::map<std::string, TList>> list)
{
   auto coincident = static_cast<TH2*>(list->at("").FindObject(fH2[0].at("griffinESuppAddbackMatrixBeta")));
   if(coincident == nullptr) {
      std::cout << "Failed to find griffinESuppAddbackMatrixBeta histogram in list:" << std::endl;
      list->at("").Print();
      return;
   }
   auto timeRandom = static_cast<TH2*>(list->at("").FindObject(fH2[0].at("griffinESuppAddbackMatrixBetaBg")));
   if(timeRandom == nullptr) {
      std::cout << "Failed to find griffinESuppAddbackMatrixBetaBg histogram in list:" << std::endl;
      list->at("").Print();
      return;
   }

   auto corrected = static_cast<TH2*>(coincident->Clone("griffinESuppAddbackMatrixBetaCorr"));
   // coinc = -250 - 250 = 500 wide, bg = -500 - -250 plus 250 - 500 = 500 wide
   corrected->Add(timeRandom, -1.);
   list->at("").Add(corrected);
}
