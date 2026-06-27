#include "CrossTalkCheckHelper.hh"

void CrossTalkCheckHelper::CreateHistograms(unsigned int slot)
{
   // total addback spectrum
   fH1[slot]["griffinESuppAddback"] = new TH1F("griffinESuppAddback", Form("Suppressed griffin addback energy;energy [keV];counts/%.1f keV", (fHighEnergy - fLowEnergy) / fEnergyBins), fEnergyBins, fLowEnergy, fHighEnergy);

   // addback spectrum with 1-4 cyrstals/fragments involved
   for(int nCry = 1; nCry <= 4; ++nCry) {
      fH1[slot][Form("griffinESuppAddback%d", nCry)] = new TH1F(Form("griffinESuppAddback%d", nCry), Form("Suppressed griffin addback energy with %d crystals/fragments;energy [keV];counts/%.1f keV", nCry, (fHighEnergy - fLowEnergy) / fEnergyBins), fEnergyBins, fLowEnergy, fHighEnergy);
   }
}

// TODO: Change the function arguments to match the detectors you want to use and the declaration in the header file!
void CrossTalkCheckHelper::Exec(unsigned int slot, TGriffin& grif, TGriffinBgo& grifBgo)   // NOLINT
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   // loop over suppressed griffin addback hits
   for(int g = 0; g < grif.GetSuppressedAddbackMultiplicity(&grifBgo); ++g) {
      auto* grif1 = grif.GetSuppressedAddbackHit(g);
      fH1[slot].at("griffinESuppAddback")->Fill(grif1->GetEnergy());
      if(1 <= grif.GetNSuppressedAddbackFrags(g) && grif.GetNSuppressedAddbackFrags(g) <= 4) {
         fH1[slot].at(Form("griffinESuppAddback%d", grif.GetNSuppressedAddbackFrags(g)))->Fill(grif1->GetEnergy());
      }
   }
}

void CrossTalkCheckHelper::EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list)
{
}
