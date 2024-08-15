#include "DirectoryHelper.hh"

void DirectoryHelper::CreateHistograms(unsigned int slot)
{
   // some variables to easily change range and binning for multiple histograms at once
   int    energyBins = 10000;
   double lowEnergy  = 0.;
   double highEnergy = 2000.;

   // unsuppressed spectra
   H1()[slot]["unsuppressed/singles/griffinE"] = new TH1F("griffinE", Form("Unsuppressed griffin energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   // suppressed spectra
   H1()[slot]["suppressed/singles/griffinESupp"] = new TH1F("griffinESupp", Form("Suppressed griffin energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   // unsuppressed addback spectra
   H1()[slot]["unsuppressed/addback/griffinEAddback"] = new TH1F("griffinEAddback", Form("Unsuppressed griffin addback energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);
   // suppressed addback spectra
   H1()[slot]["suppressed/addback/griffinESuppAddback"] = new TH1F("griffinESuppAddback", Form("Suppressed griffin addback energy;energy [keV];counts/%.1f keV", (highEnergy - lowEnergy) / energyBins), energyBins, lowEnergy, highEnergy);

   // initialize the two arrays to keep time
   fLastTS[slot].resize(64, 0.);
   fLastSuppressedTS[slot].resize(64, 0.);
   fLastTime[slot].resize(64, 0.);
   fLastSuppressedTime[slot].resize(64, 0.);
   fLastTSNoPileup[slot].resize(64, 0.);
   fLastSuppressedTSNoPileup[slot].resize(64, 0.);
   fLastTimeNoPileup[slot].resize(64, 0.);
   fLastSuppressedTimeNoPileup[slot].resize(64, 0.);

   // timing spectra
   H2()[slot]["griffinDeadTS"]                     = new TH2F("griffinDeadTS", "timestamp difference between consecutive hits in a griffin channel", 200, 0., 2000., 64, 0.5, 64.5);
   H2()[slot]["griffinSuppressedDeadTS"]           = new TH2F("griffinSuppressedDeadTS", "timestamp difference between consecutive suppressed hits in a griffin channel", 200, 0., 2000., 64, 0.5, 64.5);
   H2()[slot]["griffinDeadTime"]                   = new TH2F("griffinDeadTime", "time difference between consecutive hits in a griffin channel", 2000, 0., 2000., 64, 0.5, 64.5);
   H2()[slot]["griffinSuppressedDeadTime"]         = new TH2F("griffinSuppressedDeadTime", "time difference between consecutive suppressed hits in a griffin channel", 2000, 0., 2000., 64, 0.5, 64.5);
   H2()[slot]["griffinDeadTSNoPileup"]             = new TH2F("griffinDeadTSNoPileup", "timestamp difference between consecutive hits in a griffin channel w/o pileups", 200, 0., 2000., 64, 0.5, 64.5);
   H2()[slot]["griffinSuppressedDeadTSNoPileup"]   = new TH2F("griffinSuppressedDeadTSNoPileup", "timestamp difference between consecutive suppressed hits in a griffin channel w/o pileups", 200, 0., 2000., 64, 0.5, 64.5);
   H2()[slot]["griffinDeadTimeNoPileup"]           = new TH2F("griffinDeadTimeNoPileup", "time difference between consecutive hits in a griffin channel w/o pileups", 2000, 0., 2000., 64, 0.5, 64.5);
   H2()[slot]["griffinSuppressedDeadTimeNoPileup"] = new TH2F("griffinSuppressedDeadTimeNoPileup", "time difference between consecutive suppressed hits in a griffin channel w/o pileups", 2000, 0., 2000., 64, 0.5, 64.5);
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

void DirectoryHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   // loop over unsuppressed griffin hits
   for(int g = 0; g < grif.GetMultiplicity(); ++g) {
      auto grif1 = grif.GetGriffinHit(g);
      H1()[slot].at("unsuppressed/singles/griffinE")->Fill(grif1->GetEnergy());
      if(grif1->GetArrayNumber() <= 64) {
         if(fLastTS[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinDeadTS")->Fill(grif1->GetTimeStampNs() - fLastTS[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
         fLastTS[slot][grif1->GetArrayNumber()] = grif1->GetTimeStampNs();
         if(fLastTime[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinDeadTime")->Fill(grif1->GetTime() - fLastTime[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
         fLastTime[slot][grif1->GetArrayNumber()] = grif1->GetTime();
         if(grif1->GetKValue() != 379) {
            if(fLastTSNoPileup[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinDeadTSNoPileup")->Fill(grif1->GetTimeStampNs() - fLastTSNoPileup[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
            fLastTSNoPileup[slot][grif1->GetArrayNumber()] = grif1->GetTimeStampNs();
            if(fLastTimeNoPileup[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinDeadTimeNoPileup")->Fill(grif1->GetTime() - fLastTimeNoPileup[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
            fLastTimeNoPileup[slot][grif1->GetArrayNumber()] = grif1->GetTime();
         }
      }
   }

   // loop over suppressed griffin hits
   for(int g = 0; g < grif.GetSuppressedMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedHit(g);
      H1()[slot].at("suppressed/singles/griffinESupp")->Fill(grif1->GetEnergy());
      if(grif1->GetArrayNumber() <= 64) {
         if(fLastSuppressedTS[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinSuppressedDeadTS")->Fill(grif1->GetTimeStampNs() - fLastSuppressedTS[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
         fLastSuppressedTS[slot][grif1->GetArrayNumber()] = grif1->GetTimeStampNs();
         if(fLastSuppressedTime[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinSuppressedDeadTime")->Fill(grif1->GetTime() - fLastSuppressedTime[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
         fLastSuppressedTime[slot][grif1->GetArrayNumber()] = grif1->GetTime();
         if(grif1->GetKValue() != 379) {
            if(fLastSuppressedTSNoPileup[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinSuppressedDeadTSNoPileup")->Fill(grif1->GetTimeStampNs() - fLastSuppressedTSNoPileup[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
            fLastSuppressedTSNoPileup[slot][grif1->GetArrayNumber()] = grif1->GetTimeStampNs();
            if(fLastSuppressedTimeNoPileup[slot][grif1->GetArrayNumber()] != 0) H2()[slot].at("griffinSuppressedDeadTimeNoPileup")->Fill(grif1->GetTime() - fLastSuppressedTimeNoPileup[slot][grif1->GetArrayNumber()], grif1->GetArrayNumber());
            fLastSuppressedTimeNoPileup[slot][grif1->GetArrayNumber()] = grif1->GetTime();
         }
      }
   }

   // loop over unsuppressed griffin addback hits
   for(int g = 0; g < grif.GetAddbackMultiplicity(); ++g) {
      auto grif1 = grif.GetAddbackHit(g);
      H1()[slot].at("unsuppressed/addback/griffinEAddback")->Fill(grif1->GetEnergy());
   }

   // loop over suppressed griffin addback hits
   for(int g = 0; g < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g) {
      auto grif1 = grif.GetSuppressedAddbackHit(g);
      H1()[slot].at("suppressed/addback/griffinESuppAddback")->Fill(grif1->GetEnergy());
   }
}

void DirectoryHelper::EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list)
{
}
