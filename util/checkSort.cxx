#include <deque>

#include "TFile.h"
#include "TTree.h"
#include "TError.h"

#include "TDetector.h"

////////////////////////////////////////////////////////////////////////////////
///
/// This program checks analysis file(s) provided on the command line for
/// repeated entries by checking if entries have the same time stamp and energy.
///
////////////////////////////////////////////////////////////////////////////////

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

      auto* tree = static_cast<TTree*>(input.Get("AnalysisTree"));
      if(tree == nullptr) {
         std::cout << "No AnalysysTree in " << argv[file] << std::endl;
         continue;
      }

      // loop over all branches and add the detector to the list and set up the list of last time stamps and energies
      auto*                                  branches = tree->GetListOfBranches();
      std::vector<TDetector*>                det(branches->GetEntries());
      std::map<TClass*, std::deque<int64_t>> lastTS;
      std::map<TClass*, std::deque<double>>  lastEn;

      size_t index = 0;
      for(auto* branch : *(branches)) {
         auto* cls  = TClass::GetClass(branch->GetName());
         det[index] = static_cast<TDetector*>(cls->New());
         tree->SetBranchAddress(branch->GetName(), &det[index]);
         lastTS[cls] = std::deque<int64_t>(3, 0);
         lastEn[cls] = std::deque<double>(3, 0.);
         ++index;
      }

      Long64_t nEntries = tree->GetEntries();
      Long64_t entry    = 0;

      for(entry = 0; entry < nEntries; ++entry) {
         // check if we can advance 100 events, otherwise read last event
         if(entry + 100 < nEntries) {
            entry += 100;
         } else {
            entry = nEntries - 1;
         }

         tree->GetEntry(entry);

         if(entry % 1000 == 0) {
            std::cout << "\r" << (100 * entry) / nEntries << "% done" << std::flush;
         }

         // re-using the d-variable from above
         for(index = 0; index < det.size(); ++index) {
            // skip empty detectors
            if(det[index]->GetMultiplicity() < 1) { continue; }
            auto*         cls = det[index]->IsA();
            TDetectorHit* hit = det[index]->GetHit(0);
            if(hit != nullptr) {
               bool sameE = true;
               for(auto energy : lastEn[cls]) {
                  if(hit->GetEnergy() != energy) {
                     sameE = false;
                     break;
                  }
               }
               bool sameTS = true;
               for(auto timestamp : lastTS[cls]) {
                  if(hit->GetTimeStamp() != timestamp) {
                     sameTS = false;
                     break;
                  }
               }
               if(sameE && sameTS) {
                  std::cerr << "\r" << RED << "Got the same timestamp and energy for entry " << entry << ". File " << YELLOW << argv[file] << RED << " is bad!" << RESET_COLOR << std::endl;
                  break;
               }
               lastTS[cls].emplace_back(hit->GetTimeStamp());
               lastEn[cls].emplace_back(hit->GetEnergy());
               lastTS[cls].pop_front();
               lastEn[cls].pop_front();
            }
         }
         // if we broke out of the previous loop, break out of this one as well
         if(index != det.size()) { break; }
      }   // loop over entries
      if(entry == nEntries) {
         std::cerr << "\r" << GREEN << "File " << BLUE << argv[file] << GREEN << " is good!" << RESET_COLOR << std::endl;
      }

      input.Close();
   }

   return 0;
}
