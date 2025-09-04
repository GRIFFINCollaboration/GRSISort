#include "TacOffsetHelper.hh"

void TacOffsetHelper::CreateHistograms(unsigned int slot)
{
   // find the offsets from the current calibration
   int channelNumber = 0;
   int currentIndex  = 0;
   while(currentIndex < 8) {
      auto* channel = TChannel::GetChannelByNumber(++channelNumber);
      if(channel == nullptr) {
         break;
      }
      if(channel->GetClassType() != TTAC::Class()) {
         continue;
      }
      fOffset[currentIndex] = static_cast<double>(channel->GetTimeOffset());
      if(slot == 0) {
         std::cout << "Current TAC offset in the calfile:  " << fOffset[currentIndex] << " for channel #" << channelNumber << std::endl;
      }
      ++currentIndex;
   }

   //TAC offset histograms
   for(int i = 0; i < fOffset.size(); ++i) {
      fH1[slot][Form("TacOffset_%d", i)]          = new TH1D(Form("TacOffset_%d", i), Form("Time difference between TAC and LaBr %d; time (ns); counts/ns", i), 10000, -5000., 5000.);
      fH1[slot][Form("TacOffsetCorrected_%d", i)] = new TH1D(Form("TacOffsetCorrected_%d", i), Form("Time difference between TAC and LaBr %d, corrected by TAC offset; time (ns); counts/ns", i), 10000, -5000., 5000.);
      fH1[slot][Form("TimeDiff_%d", i)]           = new TH1D(Form("TimeDiff_%d", i), Form("Time difference for LaBr %d - LaBr with TAC coincidence; time (ns); counts/ns", i), 10000, -5000., 5000.);
      fH1[slot][Form("TimeDiffNoTac_%d", i)]      = new TH1D(Form("TimeDiffNoTac_%d", i), Form("Time difference for LaBr %d - LaBr without TAC coincidence; time (ns); counts/ns", i), 10000, -5000., 5000.);
      fH1[slot][Form("TimeStampDiff_%d", i)]      = new TH1D(Form("TimeStampDiff_%d", i), Form("Timestamp difference for LaBr %d - LaBr with TAC coincidence; time (ns); counts/ns", i), 10000, -5000., 5000.);
      fH1[slot][Form("TimeStampDiffNoTac_%d", i)] = new TH1D(Form("TimeStampDiffNoTac_%d", i), Form("Timestamp difference for LaBr %d - LaBr without TAC coincidence; time (ns); counts/ns", i), 10000, -5000., 5000.);
   }
   fH1[slot]["TimeStampDiffGriffin"] = new TH1D("TimeStampDiffGriffin", "Timestamp difference for HPGe - LaBr, with TAC coincidence; time (ns); counts/ns", 10000, -5000., 5000.);
   fH1[slot]["TimeDiffGriffin"]      = new TH1D("TimeDiffGriffin", "Time difference for HPGe - LaBr, with TAC coincidence; time (ns); counts/ns", 10000, -5000., 5000.);
}

void TacOffsetHelper::Exec(unsigned int slot, TGriffin& grif, TTAC& tac, TLaBr& labr)   // NOLINT
{
   // we use .at() here instead of [] so that we get meaningful error message if a histogram we try to fill wasn't created
   // e.g. because of a typo

   Short_t nofLaBr = labr.GetMultiplicity();
   Short_t nofTac  = tac.GetMultiplicity();
   Short_t nofGrif = grif.GetMultiplicity();

   // ignoring any events without exactly two LaBr
   if(nofLaBr != 2) { return; }

   auto* labr0 = labr.GetLaBrHit(0);
   auto* labr1 = labr.GetLaBrHit(1);

   // ignoring events where both LaBr hits are from the same detector
   if(labr0->GetDetector() == labr1->GetDetector()) { return; }

   // these histograms get filled with labr0 refering to the earlier hit and labr1 to the later hit,
   // not to the lower and higher detector number like the other histograms!
   fH1[slot].at(Form("TimeDiffNoTac_%d", labr1->GetDetector() - 1))->Fill(labr1->GetTime() - labr0->GetTime());
   fH1[slot].at(Form("TimeStampDiffNoTac_%d", labr1->GetDetector() - 1))->Fill(static_cast<Double_t>(labr1->GetTimeStampNs() - labr0->GetTimeStampNs()));

   // ensure that labr0 always refers to the LaBr with the lower detector number
   if(labr0->GetDetector() > labr1->GetDetector()) {
      std::swap(labr0, labr1);
   }

   if(nofTac == 1) {
      auto* tac0 = tac.GetTACHit(0);
      if(labr0->GetDetector() == tac0->GetDetector()) {
         fH1[slot].at(Form("TacOffset_%d", tac0->GetDetector() - 1))->Fill(labr0->GetTime() - tac0->GetTime() + fOffset[tac0->GetDetector() - 1] * 10.);
         fH1[slot].at(Form("TacOffsetCorrected_%d", tac0->GetDetector() - 1))->Fill(labr0->GetTime() - tac0->GetTime());
         fH1[slot].at(Form("TimeDiff_%d", labr1->GetDetector() - 1))->Fill(labr1->GetTime() - labr1->GetTime());
         fH1[slot].at(Form("TimeStampDiff_%d", labr1->GetDetector() - 1))->Fill(static_cast<Double_t>(labr1->GetTimeStampNs() - labr0->GetTimeStampNs()));
         for(int i = 0; i < nofGrif; ++i) {
            auto* grif1 = grif.GetGriffinHit(i);
            fH1[slot].at("TimeDiffGriffin")->Fill(grif1->GetTime() - labr0->GetTime());
            fH1[slot].at("TimeStampDiffGriffin")->Fill(static_cast<Double_t>(grif1->GetTimeStampNs() - labr0->GetTimeStampNs()));
         }
      }
   }
}

void TacOffsetHelper::EndOfSort(std::shared_ptr<std::map<std::string, TList>>& list)
{
}
