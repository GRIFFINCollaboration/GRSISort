#include <deque>

#include "TFile.h"
#include "TTree.h"
#include "TError.h"

#include "TDetector.h"

int main(int argc, char** argv)
{
   if(argc == 1) {
      std::cout << "Usage: " << argv[0] << " <analysis root-file>" << std::endl;
      return 1;
   }

   // suppress error messages
   gErrorIgnoreLevel = kFatal;

   for(int file = 1; file < argc; ++file) {
      TFile input(argv[file]);
      if(!input.IsOpen()) {
         std::cout << MAGENTA << "Failed to open " << argv[file] << RESET_COLOR << std::endl;
         return 1;
      }

      TTree* tree = static_cast<TTree*>(input.Get("AnalysisTree"));
      if(tree == nullptr) {
         std::cout << "No AnalysysTree in " << argv[file] << std::endl;
         continue;
      }

      // loop over all branches and add the detector to the list and set up the list of last time stamps and energies
      auto                                   branches = tree->GetListOfBranches();
      std::vector<TDetector*>                det(branches->GetEntries());
      std::map<TClass*, std::deque<int64_t>> lastTS;
      std::map<TClass*, std::deque<double>>  lastEn;

      size_t d = 0;
      for(auto branch : *(branches)) {
         auto cl = TClass::GetClass(branch->GetName());
         det[d]  = static_cast<TDetector*>(cl->New());
         tree->SetBranchAddress(branch->GetName(), &det[d]);
         lastTS[cl] = std::deque<int64_t>(3, 0);
         lastEn[cl] = std::deque<double>(3, 0.);
         ++d;
      }

      Long64_t nEntries = tree->GetEntries();
      Long64_t e        = 0;

      for(e = 0; e < nEntries; ++e) {
         // check if we can advance 100 events, otherwise read last event
         if(e + 100 < nEntries) e += 100;
         else e = nEntries - 1;

         tree->GetEntry(e);

         if(e % 1000 == 0) {
            std::cout << "\r" << (100 * e) / nEntries << "% done" << std::flush;
         }

         // re-using the d-variable from above
         for(d = 0; d < det.size(); ++d) {
            // skip empty detectors
            if(det[d]->GetMultiplicity() < 1) continue;
            auto          cl  = det[d]->IsA();
            TDetectorHit* hit = det[d]->GetHit(0);
            if(hit != nullptr) {
               bool sameE = true;
               for(auto energy : lastEn[cl]) {
                  if(hit->GetEnergy() != energy) {
                     sameE = false;
                     break;
                  }
               }
               bool sameTS = true;
               for(auto ts : lastTS[cl]) {
                  if(hit->GetTimeStamp() != ts) {
                     sameTS = false;
                     break;
                  }
               }
               if(sameE && sameTS) {
                  std::cerr << "\r" << RED << "Got the same timestamp and energy for entry " << e << ". File " << YELLOW << argv[file] << RED << " is bad!" << RESET_COLOR << std::endl;
                  break;
               }
               lastTS[cl].emplace_back(hit->GetTimeStamp());
               lastEn[cl].emplace_back(hit->GetEnergy());
               lastTS[cl].pop_front();
               lastEn[cl].pop_front();
            }
         }
         // if we broke out of the previous loop, break out of this one as well
         if(d != det.size()) break;
      }   // loop over entries
      if(e == nEntries) {
         std::cerr << "\r" << GREEN << "File " << BLUE << argv[file] << GREEN << " is good!" << RESET_COLOR << std::endl;
      }

      input.Close();
   }

   return 0;
}
