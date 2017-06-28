#include "TPPG.h"

#include <iomanip>
#include "TDirectory.h"

#include "TGRSIRunInfo.h"

/// \cond CLASSIMP
ClassImp(TPPGData)
ClassImp(TPPG)
/// \endcond

TPPG* TPPG::fPPG = nullptr;

TPPGData::TPPGData()
{
   Clear();
}

TPPGData::TPPGData(const TPPGData& rhs) : TObject()
{
   rhs.Copy(*this);
}

void TPPGData::Copy(TObject& rhs) const
{
   static_cast<TPPGData&>(rhs).fTimeStamp       = fTimeStamp;
   static_cast<TPPGData&>(rhs).fOldPpg          = fOldPpg;
   static_cast<TPPGData&>(rhs).fNewPpg          = fNewPpg;
   static_cast<TPPGData&>(rhs).fNetworkPacketId = fNetworkPacketId;
   static_cast<TPPGData&>(rhs).fLowTimeStamp    = fLowTimeStamp;
   static_cast<TPPGData&>(rhs).fHighTimeStamp   = fHighTimeStamp;
}

void TPPGData::SetTimeStamp()
{
   Long64_t time = GetHighTimeStamp();
   time          = time<<28;
   time |= GetLowTimeStamp() & 0x0fffffff;
   fTimeStamp = time;
}

void TPPGData::Clear(Option_t*)
{
   /// Clears the TPPGData and leaves it a "junk" state. By junk, I just mean default
   /// so that we can tell that this PPG is no good.
   fTimeStamp       = 0;
   fOldPpg          = 0xFFFF;
   fNewPpg          = 0xFFFF;
   fNetworkPacketId = -1;
   fLowTimeStamp    = 0;
   fHighTimeStamp   = 0;
}

void TPPGData::Print(Option_t*) const
{
   printf("time: %7lld\t PPG Status: 0x%07x\t Old: 0x%07x\n", GetTimeStamp(), fNewPpg, fOldPpg);
}

TPPG::TPPG()
{
   fPPGStatusMap = new PPGMap_t;
   Clear();
   // std::cout<<"default constructor called on "<<this<<std::endl;
}

TPPG::TPPG(const TPPG& rhs) : TObject()
{
   fPPGStatusMap = new PPGMap_t;
   rhs.Copy(*this);
   // std::cout<<"copy constructor called on "<<this<<" from "<<&rhs<<std::endl;
}

TPPG::~TPPG()
{
   Clear();
   PPGMap_t::iterator ppgit;
   if(fPPGStatusMap != nullptr) {
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++) {
         if(ppgit->second) {
            delete(ppgit->second);
         }
         ppgit->second = nullptr;
      }
      delete fPPGStatusMap;
   }
   // std::cout<<"destructor called on "<<this<<std::endl;
}

TPPG* TPPG::Get()
{
   // The getter for the singleton TPPG. Unfortunately ROOT doesn't allow true
   // singletons, so one should take care to always use this method and not
   // the constructor.
   if(fPPG == nullptr) {
      fPPG = static_cast<TPPG*>(gDirectory->Get("TPPG"));
      if(fPPG == nullptr) {
         fPPG = new TPPG();
      }
   }
   return fPPG;
}

void TPPG::Copy(TObject& obj) const
{
   static_cast<TPPG&>(obj).Clear();
   static_cast<TPPG&>(obj).fCycleLength          = fCycleLength;
   static_cast<TPPG&>(obj).fNumberOfCycleLengths = fNumberOfCycleLengths;

   // We want to provide a copy of each of the data in the PPG rather than a copy of the pointer
   if(static_cast<TPPG&>(obj).fPPGStatusMap != nullptr && fPPGStatusMap != nullptr) {
      for(auto ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++) {
         if(ppgit->second != nullptr) {
            static_cast<TPPG&>(obj).AddData(ppgit->second);
         }
      }
      static_cast<TPPG&>(obj).fCurrIterator = static_cast<TPPG&>(obj).fPPGStatusMap->begin();
   }
}

Bool_t TPPG::MapIsEmpty() const
{
   /// Checks to see if the ppg map is empty. We need this because we need to put a default
   /// PPG in at time T=0 to prevent bad things from happening. This function says the map
   /// is empty when only the default is there, which it essentially is.
   if(fPPGStatusMap->size() == 1) { // We check for size 1 because we always start with a Junk event at time 0.
      return true;
   } else {
      return false;
   }
}

void TPPG::AddData(TPPGData* pat)
{
   /// Adds a PPG status word at a given time in the current run. Makes a copy of the pointer to
   /// store in the map.
   fPPGStatusMap->insert(std::make_pair(pat->GetTimeStamp(), new TPPGData(*pat)));
   fCycleLength = 0;
   fNumberOfCycleLengths.clear();
}

ULong64_t TPPG::GetLastStatusTime(ULong64_t time, ppg_pattern pat, bool exact_flag) const
{
   /// Gets the last time that a status was given. If the ppg_pattern kJunk is passed, the
   /// current status at the time "time" is looked for. If exact_flag is false, the bits of "pat"
   /// are looked for and the rest of the bits in the sorted statuses ignored. If "exact_flag"
   /// is true, the entire ppg pattern "pat" must be met.
   if(MapIsEmpty()) {
      printf("Empty\n");
      return 0;
   }

   auto               curppg_it = --(fPPGStatusMap->upper_bound(time));
   PPGMap_t::iterator ppg_it;
   if(pat == kJunk) {
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it) {
         if(curppg_it->second->GetNewPPG() == ppg_it->second->GetNewPPG() && curppg_it != ppg_it) {
            return ppg_it->first;
         }
      }
   } else {
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it) {
         if(exact_flag) {
            if(pat == ppg_it->second->GetNewPPG()) {
               return ppg_it->first;
            }
         } else {
            //	printf("pat %x, ppg_it->first %lu, ppg_it->second->GetNewPPG()
            //%x\n",pat,ppg_it->first,ppg_it->second->GetNewPPG());
            if(pat & ppg_it->second->GetNewPPG()) {
               return ppg_it->first;
            }
         }
      }
   }
   // printf("No previous status\n");
   return 0;
}

uint16_t TPPG::GetStatus(ULong64_t time) const
{
   /// Returns the current status of the PPG at the time "time".
   if(MapIsEmpty()) {
      printf("Empty\n");
   }
   // The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one
   // because we want to know what the last PPG event was.
   return (uint16_t)((--(fPPGStatusMap->upper_bound(time)))->second->GetNewPPG());
}

void TPPG::Print(Option_t* opt) const
{
   if(fOdbPPGCodes.size() != fOdbDurations.size()) {
      std::cout<<"Mismatch between number of ppg codes ("<<fOdbPPGCodes.size()<<") and durations ("
               <<fOdbDurations.size()<<")"<<std::endl;
   } else {
      std::cout<<"ODB cycle:"<<std::endl<<"Code       Duration"<<std::endl;
      for(size_t i = 0; i < fOdbPPGCodes.size(); ++i) {
         std::cout<<"0x"<<std::hex<<std::setw(8)<<fOdbPPGCodes[i]<<std::dec<<" "<<fOdbDurations[i]
                  <<std::endl;
      }
   }
   if(MapIsEmpty()) {
      printf("Empty\n");
      return;
   }
   if(TString(opt).Contains("all", TString::ECaseCompare::kIgnoreCase)) {
      // print all ppg data
      PPGMap_t::iterator ppgit;
      printf("*****************************\n");
      printf("           PPG STATUS        \n");
      printf("*****************************\n");
      for(ppgit = MapBegin(); ppgit != MapEnd(); ppgit++) {
         ppgit->second->Print();
      }
      return;
   }
   // print only an overview of the ppg
   // can't call non-const GetCycleLength here, so we do the calculation with local variables here
   PPGMap_t::iterator ppgIt;
   std::map<uint16_t, int>  status;               // to calculate how often each different status occured
   std::map<ULong64_t, int> numberOfCycleLengths; // to calculate the length of the whole cycle
   std::map<ULong64_t, int>
      numberOfStateLengths[4]; // to calculate the length of each state (tape move, background, beam on, and decay)
   std::map<int, int> numberOfOffsets; // to calculate the offset on each timestamp
   for(ppgIt = MapBegin(); ppgIt != MapEnd(); ++ppgIt) {
      status[ppgIt->second->GetNewPPG()]++;
      ULong64_t diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp());
      numberOfCycleLengths[diff]++;
      numberOfOffsets[(ppgIt->second->GetTimeStamp()) % 1000]++; // let's assume our offset is less than 10 us
      switch(ppgIt->second->GetNewPPG() & 0xf) {
      case kBackground:
         diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), kTapeMove);
         numberOfStateLengths[0][diff]++;
         break;
      case kBeamOn:
         diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), kBackground);
         numberOfStateLengths[1][diff]++;
         break;
      case kDecay:
         diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), kBeamOn);
         numberOfStateLengths[2][diff]++;
         break;
      case kTapeMove:
         diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), kDecay);
         numberOfStateLengths[3][diff]++;
         break;
      default: break;
      }
   }
   int       counter     = 0;
   ULong64_t cycleLength = 0;
   for(auto& numberOfCycleLength : numberOfCycleLengths) {
      if(numberOfCycleLength.second > counter) {
         counter     = numberOfCycleLength.second;
         cycleLength = numberOfCycleLength.first;
      }
   }
   ULong64_t stateLength[4] = {0, 0, 0, 0};
   for(int i = 0; i < 4; ++i) {
      counter = 0;
      for(auto it = numberOfStateLengths[i].begin(); it != numberOfStateLengths[i].end(); ++it) {
         if(it->second > counter) {
            counter        = it->second;
            stateLength[i] = it->first;
         }
      }
   }
   counter    = 0;
   int offset = 0;
   for(auto& numberOfOffset : numberOfOffsets) {
      if(numberOfOffset.second > counter) {
         counter = numberOfOffset.second;
         offset  = numberOfOffset.first;
      }
   }

   // the print statement itself
   printf("Cycle length is %lld in 10 ns units = %.3lf seconds.\n", cycleLength, cycleLength / 1e8);
   printf("Cycle: %.3lf s tape move, %.3lf background, %.3lf beam on, and %.3lf decay\n", stateLength[0] / 1e8,
          stateLength[1] / 1e8, stateLength[2] / 1e8, stateLength[3] / 1e8);
   printf("Offset is %d\n", offset);
   printf("Got %ld PPG words:\n", fPPGStatusMap->size() - 1);
   for(auto& statu : status) {
      printf("\tfound status 0x%04x %d times\n", statu.first, statu.second);
   }

   // go through all expected ppg words
   ULong64_t time = offset;
   auto      it   = MapEnd();
   --it;
   ULong64_t lastTimeStamp = it->second->GetTimeStamp();
   for(int cycle = 1; time < lastTimeStamp; ++cycle) {
      if(GetLastStatusTime(time, kTapeMove) != time) {
         printf("Missing tape move status at %12lld in %d. cycle, last tape move status came at %lld.\n", time, cycle,
                GetLastStatusTime(time, kTapeMove));
      }
      if(GetLastStatusTime(time + cycleLength, kBackground) != time + stateLength[0]) {
         printf("Missing background status at %12lld in %d. cycle, last background status came at %lld.\n",
                time + stateLength[0], cycle, GetLastStatusTime(time + cycleLength, kBackground));
      }
      if(GetLastStatusTime(time + cycleLength, kBeamOn) != time + stateLength[0] + stateLength[1]) {
         printf("Missing beam on status at %12lld in %d. cycle, last beam on status came at %lld.\n",
                time + stateLength[0] + stateLength[1], cycle, GetLastStatusTime(time + cycleLength, kBeamOn));
      }
      if(GetLastStatusTime(time + cycleLength, kDecay) != time + stateLength[0] + stateLength[1] + stateLength[2]) {
         printf("Missing decay status at %12lld in %d. cycle, last decay status came at %lld.\n",
                time + stateLength[0] + stateLength[1] + stateLength[2], cycle,
                GetLastStatusTime(time + cycleLength, kDecay));
      }
      time += cycleLength;
   }
}

void TPPG::Clear(Option_t*)
{
   if(fPPGStatusMap != nullptr) {
      PPGMap_t::iterator ppgit;
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++) {
         if(ppgit->second) {
            delete(ppgit->second);
         }
         ppgit->second = nullptr;
      }
      fPPGStatusMap->clear();
   }
   // We always add a junk event to keep the code from crashing if we ask for a PPG below the lowest PPG time.
   AddData(new TPPGData);
   fCurrIterator = fPPGStatusMap->begin();
   fCycleLength  = 0;
   fNumberOfCycleLengths.clear();
}

Int_t TPPG::Write(const char*, Int_t, Int_t) const
{
   if(PPGSize() > 0 || OdbPPGSize() > 0) {
      printf("Writing PPG with %lu events and %lu ODB settings\n", PPGSize(), OdbPPGSize());
      return TObject::Write("TPPG", TObject::kSingleKey);
   }

   return 0;
}

void TPPG::Setup()
{
   if(TGRSIRunInfo::Tigress() || TGRSIRunInfo::Sharc()) {
      return;
   }

   if(TGRSIRunInfo::SubRunNumber() > 0) {
      auto* prevSubRun =
         new TFile(Form("fragment%05d_%03d.root", TGRSIRunInfo::RunNumber(), TGRSIRunInfo::SubRunNumber() - 1));
      if(prevSubRun->IsOpen()) {
         TPPG* prev_ppg = static_cast<TPPG*>(prevSubRun->Get("TPPG"));
         if(prev_ppg) {
            prev_ppg->Copy(*this);
            printf("Found previous PPG data from run %s\n", prevSubRun->GetName());
         } else {
            printf("Error, could not find PPG in file fragment%05d_%03d.root, not adding previous PPG data\n",
                   TGRSIRunInfo::RunNumber(), TGRSIRunInfo::SubRunNumber() - 1);
            printf("PPG set up.\n");
         }
         prevSubRun->Close();
      } else {
         printf("Error, could not find file fragment%05d_%03d.root, not adding previous PPG data\n",
                TGRSIRunInfo::RunNumber(), TGRSIRunInfo::SubRunNumber() - 1);
         printf("PPG set up.\n");
      }
   }
}

bool TPPG::Correct(bool verbose)
{
   /// Correct the map of PPG events by checking that we always have a PPG event with the same status that occured
   /// fCycleLength ago.

   // we use the cycle time to correct our ppg, so make sure it's set (GetCycleLength calculates and sets fCycleLength
   // only if it's not already set)
   GetCycleLength();

   if(verbose) {
      // we can now use fNumberOfCycleLengths to see how many cycle lengths we have that are wrong
      for(auto& fNumberOfCycleLength : fNumberOfCycleLengths) {
         if(fNumberOfCycleLength.first < fCycleLength) {
            continue;
         }
         if(fNumberOfCycleLength.first != fCycleLength) {
            printf("Found %d wrong cycle length(s) of %lld (correct is %lld).\n", fNumberOfCycleLength.second,
                   fNumberOfCycleLength.first, fCycleLength);
         }
      }
   }

   // loop over all ppg data, check that the last ppg data of the same type was fCycleLength ago
   // if it's not, enter ppg data at that time (multiple times?)
   for(auto it = MapBegin(); it != MapEnd(); it++) {
      // skip the first cycle
      if((*it).first < fCycleLength) {
         continue;
      }
      // get time difference to previous ppg with same status
      ULong64_t diff = (*it).second->GetTimeStamp() - GetLastStatusTime((*it).second->GetTimeStamp());
      if(diff != fCycleLength) {
         if(verbose) {
            printf("%ld: found missing ppg at time %lu (%lld != %lld)\n", std::distance(MapBegin(), it), (*it).first,
                   diff, fCycleLength);
         }
         // check that the previous ppg with the same status is a multiple of fCycleLength ago and that no other ppg is
         // in the map that was fCycleLength ago
         if(diff % fCycleLength != 0) {
            if(verbose) {
               printf(DRED "PPG is messed up, cycle length is %lld, but the previous event with the same status was "
                           "%lld ago!" RESET_COLOR "\n",
                      fCycleLength, diff);
            }
            continue;
            return false;
         }
         auto prev = fPPGStatusMap->find((*it).first - fCycleLength);
         if(prev != fPPGStatusMap->end()) {
            // check if the status of the current ppg and the old status of the ppg following the one fCycleLength ago
            // are the same
            // if they are, we change the status of the one fCycleLength ago to match the current status
            if(it->second->GetNewPPG() == (++prev)->second->GetOldPPG()) {
               (--prev)->second->SetNewPPG(it->second->GetNewPPG());
            } else if(verbose) {
               printf(DBLUE "PPG at %lld already exist with status 0x%x (current status is 0x%x)." RESET_COLOR "\n",
                      (*it).first - fCycleLength, prev->second->GetNewPPG(), it->second->GetNewPPG());
            }
            continue;
         }
         // copy the current ppg data and correct it's time before inserting it into the map
         auto*     new_data = new TPPGData(*((*it).second));
         ULong64_t new_ts   = (*it).first - fCycleLength;
         new_data->SetHighTimeStamp(new_ts >> 28);
         new_data->SetLowTimeStamp(new_ts & 0x0fffffff);
         if(verbose) {
            printf("inserting new ppg data at %lld\n", new_data->GetTimeStamp());
         }
         it = fPPGStatusMap->insert(std::make_pair(new_data->GetTimeStamp(), new_data)).first;
         --it;
      }
   }

   // now we reset fCycleLength and calculate it again
   fCycleLength = 0;
   fNumberOfCycleLengths.clear();
   GetCycleLength();

   if(verbose) {
      // we can now use fNumberOfCycleLengths to see how many cycle lengths we have that are wrong
      for(auto& fNumberOfCycleLength : fNumberOfCycleLengths) {
         if(fNumberOfCycleLength.first < fCycleLength) {
            continue;
         }
         if(fNumberOfCycleLength.first != fCycleLength) {
            printf("Found %d wrong cycle length(s) of %lld (correct is %lld).\n", fNumberOfCycleLength.second,
                   fNumberOfCycleLength.first, fCycleLength);
         }
      }
   }

   return true;
}

const TPPGData* TPPG::Next()
{
   if(++fCurrIterator != MapEnd()) {
      return fCurrIterator->second;
   } else {
      printf("Already at last PPG\n");
      return nullptr;
   }
}

const TPPGData* TPPG::Previous()
{
   if(fCurrIterator != MapBegin()) {
      return (--fCurrIterator)->second;
   } else {
      printf("Already at first PPG\n");
      return nullptr;
   }
}

const TPPGData* TPPG::Last()
{
   fCurrIterator = MapEnd();
   --fCurrIterator;
   return fCurrIterator->second;
}

const TPPGData* TPPG::First()
{
   fCurrIterator = MapBegin();
   return fCurrIterator->second;
}

void TPPG::Streamer(TBuffer& R__b)
{
   // Stream an object of class TPPG.

   if(R__b.IsReading()) {
      R__b.ReadClassBuffer(TPPG::Class(), this);
      fCurrIterator = fPPGStatusMap->begin();
   } else {
      R__b.WriteClassBuffer(TPPG::Class(), this);
   }
}

ULong64_t TPPG::GetTimeInCycle(ULong64_t real_time)
{
   return real_time % GetCycleLength();
}

ULong64_t TPPG::GetCycleNumber(ULong64_t real_time)
{
   return real_time / GetCycleLength();
}

ULong64_t TPPG::GetCycleLength()
{
   if(fCycleLength == 0) {
      PPGMap_t::iterator ppgit;
      for(ppgit = MapBegin(); ppgit != MapEnd(); ++ppgit) {
         ULong64_t diff = ppgit->second->GetTimeStamp() - GetLastStatusTime(ppgit->second->GetTimeStamp());
         fNumberOfCycleLengths[diff]++;
      }
      int counter = 0;
      for(auto& fNumberOfCycleLength : fNumberOfCycleLengths) {
         if(fNumberOfCycleLength.second > counter) {
            counter      = fNumberOfCycleLength.second;
            fCycleLength = fNumberOfCycleLength.first;
         }
      }
   }

   return fCycleLength;
}

ULong64_t TPPG::GetNumberOfCycles()
{
   return Last()->GetTimeStamp() / GetCycleLength();
}

Long64_t TPPG::Merge(TCollection* list)
{
   TIter it(list);
   TPPG* ppg = nullptr;

   while((ppg = static_cast<TPPG*>(it.Next()))) {
      *this += *ppg;
   }

   return 0;
}

ULong64_t TPPG::GetStatusStart(ppg_pattern pat)
{
   /// return the time in the cycle when pat starts
   return GetTimeInCycle(GetLastStatusTime(Last()->GetTimeStamp(), pat));
}

void TPPG::operator+=(const TPPG& rhs)
{
   Add(&rhs);
}

void TPPG::Add(const TPPG* ppg)
{
   PPGMap_t::iterator ppgit;
   for(ppgit = ppg->MapBegin(); ppgit != ppg->MapEnd(); ++ppgit) {
      if(ppgit->second) {
         AddData(ppgit->second);
      }
   }
   // We want to rebuild our cycle length map and this is the easiest way to do it
   fNumberOfCycleLengths.clear();
   fCycleLength = 0;
   GetCycleLength();
}
