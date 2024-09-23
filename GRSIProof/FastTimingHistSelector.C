#define FastTimingHistSelector_cxx
// The class definition in FastTimingHistSelector.h has been generated automatically
#include "FastTimingHistSelector.h"

void FastTimingHistSelector::CreateHistograms()
{
   // get settings from GValues
   fDrainingEnergy = GValue::Value("GRSISort.FastTiming.DrainingEnergy");
   fDrainingLow    = GValue::Value("GRSISort.FastTiming.DrainingLow");
   fDrainingHigh   = GValue::Value("GRSISort.FastTiming.DrainingHigh");
   fFeedingEnergy  = GValue::Value("GRSISort.FastTiming.FeedingEnergy");
   fFeedingLow     = GValue::Value("GRSISort.FastTiming.FeedingLow");
   fFeedingHigh    = GValue::Value("GRSISort.FastTiming.FeedingHigh");
   fGeEnergy       = GValue::Value("GRSISort.FastTiming.GeEnergy");
   fGeLow          = GValue::Value("GRSISort.FastTiming.GeLow");
   fGeHigh         = GValue::Value("GRSISort.FastTiming.GeHigh");

   // set new output prefix - doesn't work because this is a worker function and the master is the one writing the file
   SetOutputPrefix(Form("histoPRD_with_%.0f-%.0f_drain_%.0f-%.0f_feed_%.0f-%.0f_Ge_%s.root", fDrainingLow, fDrainingHigh, fFeedingLow, fFeedingHigh, fGeLow, fGeHigh, (fGeEnergy != 0) ? "true" : "false"));

   int    bins = 5000;
   double low  = -2500.;
   double high = 2500.;
   // Define Histograms
   fH1["laBrLaBrGate"]    = new TH1D("LaBrLaBrGate", Form("LaBr_3 E w/ %.1f as gate (Draining)", fDrainingEnergy), bins, low, high);
   fH1["laBrLaBrGate2"]   = new TH1D("LaBrLaBrGate2", Form("LaBr_3 E w/ %.1f as gate (Feeding)", fFeedingEnergy), bins, low, high);
   fH1["laBrLaBrGeGate"]  = new TH1D("LaBrLaBrGeGate", Form("LaBr_3 E w/ %.1f as gate (Draining), and %.1f as hpge gate", fDrainingEnergy, fGeEnergy), bins, low, high);
   fH1["laBrLaBrGeGate2"] = new TH1D("LaBrLaBrGeGate2", Form("LaBr_3 E w/ %.1f as gate (Feeding), and %.1f as hpge gate", fFeedingEnergy, fGeEnergy), bins, low, high);

   fH1["laBrFull"]      = new TH1D("LaBrFull", "LaBr_3 singles", bins, low, high);
   fH1["geFull"]        = new TH1D("GeFull", "Ge singles", bins, low, high);
   fH1["laBrGeGate"]    = new TH1D("LaBrGeGate", Form("LaBr_3 E w %.1f as HPGe gate", fGeEnergy), bins, low, high);
   fH1["geLaBrGate"]    = new TH1D("GeLaBrGate", Form("HPGe E w %.1f as gate (Draining)", fDrainingEnergy), bins, low, high);
   fH1["geLaBrGate2"]   = new TH1D("GeLaBrGate2", Form("HPGe E w %.1f as gate (Feeding)", fFeedingEnergy), bins, low, high);
   fH1["ge2LaBrGate"]   = new TH1D("Ge2LaBrGate", Form("HPGe E w %.1f as gate (Draining) and  %.1f as gate (Feeding)", fDrainingEnergy, fFeedingEnergy), bins, low, high);
   fH1["geGeLaBrGate"]  = new TH1D("GeGeLaBrGate", Form("HPGe E w %.1f as gate (Draining) and  %.1f as HPGe gate widened +/-5 but on LaBr_3", fDrainingEnergy, fGeEnergy), bins, low, high);
   fH1["geGeLaBr2Gate"] = new TH1D("GeGeLaBr2Gate", Form("HPGe E w %.1f as gate (Draining) and  %.1f as HPGe gate widened +/-5 but on LaBr_3", fFeedingEnergy, fGeEnergy), bins, low, high);
   fH1["delayed"]       = new TH1D("delayed", Form("delayed w %.1f as Draining, %.1f as Feeding, %s", fDrainingEnergy, fFeedingEnergy, ((fGeEnergy != 0.) ? Form("%.1f as HPGe gate", fGeEnergy) : "no HPGe gate")), 10 * bins, 10 * low, 10 * high);
   fH1["antidelayed"]   = new TH1D("antidelayed", Form("antidelayed w %.1f as Draining, %.1f as Feeding, %s", fDrainingEnergy, fFeedingEnergy, ((fGeEnergy != 0.) ? Form("%.1f as HPGe gate", fGeEnergy) : "no HPGe gate")), 10 * bins, 10 * low, 10 * high);

   // Send histograms to Output list to be added and written.
   for(auto it : fH1) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fH2) {
      GetOutputList()->Add(it.second);
   }
   for(auto it : fHSparse) {
      GetOutputList()->Add(it.second);
   }
}

void FastTimingHistSelector::FillHistograms()
{
   // singles spectra
   fH1.at("laBrFull")->Fill(fLaBrEnergy[0]);
   for(auto g : *fGeEnergies) {
      fH1.at("geFull")->Fill(g);
   }

   // loop over the starting and stopping LaBr for the gates and use the other one to fill histograms
   // i = 0 => (i+1)%2 = 1%2 = 1, i = 1 => (i+1)%2 = 2%2 = 0
   for(int i = 0; i < 2; ++i) {
      // draining gate
      if(fDrainingLow < fLaBrEnergy[i] && fLaBrEnergy[i] < fDrainingHigh) {
         fH1.at("laBrLaBrGate")->Fill(fLaBrEnergy[(i + 1) % 2]);
      }

      // feeding gate
      if(fFeedingLow < fLaBrEnergy[i] && fLaBrEnergy[i] < fFeedingHigh) {
         fH1.at("laBrLaBrGate2")->Fill(fLaBrEnergy[(i + 1) % 2]);
      }

      // germanium spectra
      for(auto g : *fGeEnergies) {
         // draining gate
         if(fDrainingLow < fLaBrEnergy[i] && fLaBrEnergy[i] < fDrainingHigh) {
            fH1.at("geLaBrGate")->Fill(g);
            // draining & feeding gate
            if(fFeedingLow < fLaBrEnergy[(i + 1) % 2] && fLaBrEnergy[(i + 1) % 2] < fFeedingHigh) {
               fH1.at("ge2LaBrGate")->Fill(g);
            }
            // draining & HPGe gate imposed on first LaBr
            if(fGeLow - 5. < fLaBrEnergy[(i + 1) % 2] && fLaBrEnergy[(i + 1) % 2] < fGeHigh + 5.) {
               fH1.at("geGeLaBrGate")->Fill(g);
            }
         }
         // feeding gate
         if(fFeedingLow < fLaBrEnergy[i] && fLaBrEnergy[i] < fFeedingHigh) {
            fH1.at("geLaBrGate2")->Fill(g);
            // feeding & HPGe gate imposed on first LaBr
            if(fGeLow - 5. < fLaBrEnergy[(i + 1) % 2] && fLaBrEnergy[(i + 1) % 2] < fGeHigh + 5.) {
               fH1.at("geGeLaBr2Gate")->Fill(g);
            }
         }
      }

      // germanium gated spectra
      if(fGeEnergy != 0.) {
         for(auto g : *fGeEnergies) {
            if(fGeLow < g && g < fGeHigh) {
               fH1.at("laBrGeGate")->Fill(fLaBrEnergy[(i + 1) % 2]);
               fH1.at("laBrGeGate")->Fill(fLaBrEnergy[i]);
               // draining gate
               if(fDrainingLow < fLaBrEnergy[i] && fLaBrEnergy[i] < fDrainingHigh) {
                  fH1.at("laBrLaBrGeGate")->Fill(fLaBrEnergy[(i + 1) % 2]);
               }
               // feeding gate
               if(fFeedingLow < fLaBrEnergy[i] && fLaBrEnergy[i] < fFeedingHigh) {
                  fH1.at("laBrLaBrGeGate2")->Fill(fLaBrEnergy[(i + 1) % 2]);
               }
               break;   // we found a matching germanium energy, so we stop here
            }
         }
      }
   }   // loop over LaBr
   // delayed and anti-delayed spectra
   // with germanium gate
   if(fGeEnergy != 0.) {
      for(auto g : *fGeEnergies) {
         if(fGeLow < g && g < fGeHigh) {
            // if the start feeds and the stop drains we have the delayed situation
            if(fFeedingLow < fLaBrEnergy[0] && fLaBrEnergy[0] < fFeedingHigh && fDrainingLow < fLaBrEnergy[1] && fLaBrEnergy[1] < fDrainingHigh) {
               fH1.at("delayed")->Fill(fTac);
            }
            // if the start drains and the stop feeds we have the anti-delayed situation
            if(fFeedingLow < fLaBrEnergy[1] && fLaBrEnergy[1] < fFeedingHigh && fDrainingLow < fLaBrEnergy[0] && fLaBrEnergy[0] < fDrainingHigh) {
               fH1.at("antidelayed")->Fill(fTac);
            }
            break;   // we found a matching germanium energy, so we stop here
         }
      }
   } else if(fGeLow == fGeHigh || (fGeEnergies->size() == 1 && fGeEnergies->at(0) == -2000.)) {
      // if the start feeds and the stop drains we have the delayed situation
      if(fFeedingLow < fLaBrEnergy[0] && fLaBrEnergy[0] < fFeedingHigh && fDrainingLow < fLaBrEnergy[1] && fLaBrEnergy[1] < fDrainingHigh) {
         fH1.at("delayed")->Fill(fTac);
      }
      // if the start drains and the stop feeds we have the anti-delayed situation
      if(fFeedingLow < fLaBrEnergy[1] && fLaBrEnergy[1] < fFeedingHigh && fDrainingLow < fLaBrEnergy[0] && fLaBrEnergy[0] < fDrainingHigh) {
         fH1.at("antidelayed")->Fill(fTac);
      }
   }
}
