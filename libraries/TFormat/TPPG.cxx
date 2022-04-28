#include "TPPG.h"

#include <iomanip>
#include "TDirectory.h"

#include "TRunInfo.h"

/// \cond CLASSIMP
ClassImp(TPPGData)
ClassImp(TPPG)
/// \endcond

short TPPGData::fTimestampUnits = 10;
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
   fTimeStamp = fTimestampUnits * time;
}

void TPPGData::Clear(Option_t*)
{
   /// Clears the TPPGData and leaves it a "junk" state. By junk, I just mean default
   /// so that we can tell that this PPG is no good.
   fTimeStamp       = 0;
   fOldPpg          = EPpgPattern::kJunk;
   fNewPpg          = EPpgPattern::kJunk;
   fNetworkPacketId = -1;
   fLowTimeStamp    = 0;
   fHighTimeStamp   = 0;
}

void TPPGData::Print(Option_t*) const
{
   std::cout<<"time: "<<std::setw(7)<<GetTimeStamp()<<"\t PPG Status: "<<hex(static_cast<std::underlying_type<EPpgPattern>::type>(fNewPpg),7)<<"\t Old: "<<hex(static_cast<std::underlying_type<EPpgPattern>::type>(fOldPpg),7)<<std::endl;
}

TPPG::TPPG()
{
   fPPGStatusMap = new PPGMap_t;
   Clear();
}

TPPG::TPPG(const TPPG& rhs) : TSingleton<TPPG>()
{
   fPPGStatusMap = new PPGMap_t;
   rhs.Copy(*this);
}

TPPG::~TPPG()
{
   Clear();
   PPGMap_t::iterator ppgit;
   if(fPPGStatusMap != nullptr) {
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++) {
         if(ppgit->second != nullptr) {
            delete(ppgit->second);
         }
         ppgit->second = nullptr;
      }
      delete fPPGStatusMap;
   }
}

void TPPG::Copy(TObject& obj) const
{
   static_cast<TPPG&>(obj).Clear();
   static_cast<TPPG&>(obj).fCycleLength          = fCycleLength;
   static_cast<TPPG&>(obj).fNumberOfCycleLengths = fNumberOfCycleLengths;

   // We want to provide a copy of each of the data in the PPG rather than a copy of the pointer
   if(static_cast<TPPG&>(obj).fPPGStatusMap != nullptr && fPPGStatusMap != nullptr) {
      for(auto& ppgit : *fPPGStatusMap) {
         if(ppgit.second != nullptr) {
            static_cast<TPPG&>(obj).AddData(ppgit.second);
         }
      }
      static_cast<TPPG&>(obj).fCurrIterator = static_cast<TPPG&>(obj).fPPGStatusMap->begin();
   }

   static_cast<TPPG&>(obj).fOdbPPGCodes     = fOdbPPGCodes;
   static_cast<TPPG&>(obj).fOdbDurations    = fOdbDurations;
}

Bool_t TPPG::MapIsEmpty() const
{
   /// Checks to see if the ppg map is empty. We need this because we need to put a default
   /// PPG in at time T=0 to prevent bad things from happening. This function says the map
   /// is empty when only the default is there, which it essentially is.
   if(fPPGStatusMap->size() == 1) { // We check for size 1 because we always start with a Junk event at time 0.
      return true;
   }
   return false;
}

void TPPG::AddData(TPPGData* pat)
{
   /// Adds a PPG status word at a given time in the current run. Makes a copy of the pointer to
   /// store in the map.
   fPPGStatusMap->insert(std::make_pair(pat->GetTimeStamp(), new TPPGData(*pat)));
   fCycleLength = 0;
	fCycleSet = false;
   fNumberOfCycleLengths.clear();
}

ULong64_t TPPG::GetLastStatusTime(ULong64_t time, EPpgPattern pat) const
{
   /// Gets the last time that a status was given. If the EPpgPattern kJunk is passed, the
   /// current status at the time "time" is looked for.
	/// "time" is in ns (since we store the PPG timestamp in ns as well)
   if(MapIsEmpty()) {
      std::cout<<"Empty"<<std::endl;
      return 0;
   }

   auto               curppg_it = --(fPPGStatusMap->upper_bound(time));
   PPGMap_t::iterator ppg_it;
   if(pat == EPpgPattern::kJunk) {
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it) {
         if(curppg_it->second->GetNewPPG() == ppg_it->second->GetNewPPG() && curppg_it != ppg_it) {
            return ppg_it->first;
         }
      }
   } else {
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it) {
			if(pat == ppg_it->second->GetNewPPG()) {
				return ppg_it->first;
			}
      }
   }
   return 0;
}

ULong64_t TPPG::GetNextStatusTime(ULong64_t time, EPpgPattern pat) const
{
   /// Gets the next time that a status was given. If the EPpgPattern kJunk is passed, the
   /// current status at the time "time" is looked for. 
	/// "time" is in ns (since we store the PPG timestamp in ns as well)
   if(MapIsEmpty()) {
      std::cout<<"Empty"<<std::endl;
      return 0;
   }

   auto               curppg_it = --(fPPGStatusMap->upper_bound(time));
   PPGMap_t::iterator ppg_it;
   if(pat == EPpgPattern::kJunk) {
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->end(); ++ppg_it) {
         if(curppg_it->second->GetNewPPG() == ppg_it->second->GetNewPPG() && curppg_it != ppg_it) {
            return ppg_it->first;
         }
      }
   } else {
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->end(); ++ppg_it) {
			if(pat == ppg_it->second->GetNewPPG()) {
				return ppg_it->first;
			}
      }
   }
   return 0;
}

EPpgPattern TPPG::GetStatus(ULong64_t time) const
{
   /// Returns the current status of the PPG at the time "time".
	/// "time" is in ns (since we store the PPG timestamp in ns as well)
   if(MapIsEmpty()) {
      std::cout<<"Empty"<<std::endl;
   }
   // The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one
   // because we want to know what the last PPG event was.
   return ((--(fPPGStatusMap->upper_bound(time)))->second->GetNewPPG());
}

EPpgPattern TPPG::GetNextStatus(ULong64_t time) const
{
   /// Returns the next status of the PPG at the time "time".
	/// "time" is in ns (since we store the PPG timestamp in ns as well)
   if(MapIsEmpty()) {
      std::cout<<"Empty"<<std::endl;
   }
   // The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one
   // because we want to know what the last PPG event was.
	if(fPPGStatusMap->upper_bound(time) == fPPGStatusMap->end()) {
		return EPpgPattern::kJunk;
	}
   return ((fPPGStatusMap->upper_bound(time))->second->GetNewPPG());
}

void TPPG::Print(Option_t* opt) const
{
   if(fOdbPPGCodes.size() != fOdbDurations.size()) {
      std::cout<<"Mismatch between number of ppg codes ("<<fOdbPPGCodes.size()<<") and durations ("
               <<fOdbDurations.size()<<")"<<std::endl;
   } else {
		if(fOdbPPGCodes.empty()) {
			std::cout<<"No ODB cycle read!"<<std::endl;
		} else {
			std::cout<<"ODB cycle:"<<std::endl<<"Code   Duration"<<std::endl;
			for(size_t i = 0; i < fOdbPPGCodes.size(); ++i) {
				std::cout<<hex(fOdbPPGCodes[i],4)<<" "<<fOdbDurations[i]<<std::endl;
			}
		}
   }
   if(MapIsEmpty()) {
      std::cout<<"Empty"<<std::endl;
      return;
   }
   if(TString(opt).Contains("all", TString::ECaseCompare::kIgnoreCase)) {
      // print all ppg data
      PPGMap_t::iterator ppgit;
      std::cout<<"*****************************"<<std::endl;
      std::cout<<"           PPG STATUS        "<<std::endl;
      std::cout<<"*****************************"<<std::endl;
      for(ppgit = MapBegin(); ppgit != MapEnd(); ppgit++) {
         ppgit->second->Print();
      }
      return;
   }
   // print only an overview of the ppg
   // can't call non-const GetCycleLength here, so we do the calculation with local variables here
   std::map<EPpgPattern, int>  status;               // to calculate how often each different status occured
   std::map<ULong64_t, int> numberOfCycleLengths;    // to calculate the length of the whole cycle
   std::map<ULong64_t, int> numberOfStateLengths[4]; // to calculate the length of each state (tape move, background, beam on, and decay)
   std::map<int, int> numberOfOffsets; // to calculate the offset on each timestamp
   for(PPGMap_t::iterator ppgIt = MapBegin(); ppgIt != MapEnd(); ++ppgIt) {
      status[ppgIt->second->GetNewPPG()]++;
      ULong64_t diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp());
      numberOfCycleLengths[diff]++;
      numberOfOffsets[(ppgIt->second->GetTimeStamp()) % 1000]++; // let's assume our offset is less than 10 us
      switch(ppgIt->second->GetNewPPG()) {
			case EPpgPattern::kBackground:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kTapeMove);
				numberOfStateLengths[0][diff]++;
				break;
			case EPpgPattern::kBeamOn:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kBackground);
				numberOfStateLengths[1][diff]++;
				break;
			case EPpgPattern::kDecay:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kBeamOn);
				numberOfStateLengths[2][diff]++;
				break;
			case EPpgPattern::kTapeMove:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kDecay);
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
   std::cout<<"Cycle length is "<<cycleLength<<" in ns = "<<cycleLength/1e9<<" seconds."<<std::endl;
   std::cout<<"Cycle: "<<stateLength[0]/1e9<<" s tape move, "<<stateLength[1]/1e9<<" s background, "<<stateLength[2]/1e9<<" s beam on, and "<<stateLength[3]/1e9<<" s decay"<<std::endl;
   std::cout<<"Offset is "<<offset<<" [ns]"<<std::endl;
   std::cout<<"Got "<<fPPGStatusMap->size() - 1<<" PPG words:"<<std::endl;
   for(auto& statu : status) {
      std::cout<<"\tfound status "<<hex(static_cast<std::underlying_type<EPpgPattern>::type>(statu.first),4)<<" "<<statu.second<<" times"<<std::endl;
   }

   if(TString(opt).Contains("missing", TString::ECaseCompare::kIgnoreCase)) {
		// go through all expected ppg words
		ULong64_t time = offset;
		auto      it   = MapEnd();
		--it;
		ULong64_t lastTimeStamp = it->second->GetTimeStamp();
		for(int cycle = 1; time < lastTimeStamp; ++cycle) {
			if(GetLastStatusTime(time, EPpgPattern::kTapeMove) != time) {
				std::cout<<"Missing tape move status at "<<std::setw(12)<<time<<" in "<<cycle<<". cycle, last tape move status came at "<<GetLastStatusTime(time, EPpgPattern::kTapeMove)<<"."<<std::endl;
			}
			if(GetLastStatusTime(time + cycleLength, EPpgPattern::kBackground) != time + stateLength[0]) {
				std::cout<<"Missing background status at "<<std::setw(12)<<time + stateLength[0]<<" in "<<cycle<<". cycle, last background status came at "<<GetLastStatusTime(time + cycleLength, EPpgPattern::kBackground)<<"."<<std::endl;
			}
			if(GetLastStatusTime(time + cycleLength, EPpgPattern::kBeamOn) != time + stateLength[0] + stateLength[1]) {
				std::cout<<"Missing beam on status at "<<std::setw(12)<<time + stateLength[0] + stateLength[1]<<" in "<<cycle<<". cycle, last beam on status came at "<<GetLastStatusTime(time + cycleLength, EPpgPattern::kBeamOn)<<"."<<std::endl;
			}
			if(GetLastStatusTime(time + cycleLength, EPpgPattern::kDecay) != time + stateLength[0] + stateLength[1] + stateLength[2]) {
				std::cout<<"Missing decay status at "<<std::setw(12)<<time + stateLength[0] + stateLength[1] + stateLength[2]<<" in "<<cycle<<". cycle, last decay status came at "<<GetLastStatusTime(time + cycleLength, EPpgPattern::kDecay)<<"."<<std::endl;
			}
			time += cycleLength;
		}
	}
}

bool TPPG::OdbMatchesData(bool verbose)
{
	if(!CalculateCycleFromData(verbose)) return false;
	
	// we got a cycle from the data, so we can compare it
	if(fPPGCodes.size() != fOdbPPGCodes.size()) {
		if(verbose) {
			std::cout<<"Got: "<<fPPGCodes.size()<<" states from data, and "<<fOdbPPGCodes.size()<<" states from ODB"<<std::endl;
		}
		return false;
	}
	for(size_t i = 0; i < fPPGCodes.size(); ++i) {
		// we only check the lowest nibble of the code, as the ODB code quite often is of the form 0xc00x (x = 1, 2, 4, or 8)
		// and we need to convert the data durations (in ns) to the ODB duration (in microseconds)
		if((fPPGCodes.at(i)&0xf) != (fOdbPPGCodes.at(i)&0xf) || static_cast<int>(fDurations.at(i)/1000) != fOdbDurations.at(i)) {
			if(verbose) {
				std::cout<<i<<": Mismatch between lowest nibble of codes ("<<hex(fPPGCodes.at(i),4)<<", "<<hex(fOdbPPGCodes.at(i),4)<<") or durations ("<<fDurations.at(i)<<", "<<fOdbDurations.at(i)<<")"<<std::endl;
			}
			return false;
		}
	}
	// if we got here without finding anything wrong the odb matches the data
	return true;
}

void TPPG::SetOdbFromData(bool verbose)
{
	if(!CalculateCycleFromData(verbose)) {
		std::cerr<<RED<<"Can't set ODB from data as we failed to calculate the cycle from data!"<<RESET_COLOR<<std::endl;
		return;
	}
	// we got a cycle, so we can set the ODB to it
	fOdbPPGCodes  = fPPGCodes;
	// need to convert the durations from ns to microseconds
	fOdbDurations.resize(fDurations.size());
	for(size_t i = 0; i < fDurations.size(); ++i) {
		fOdbDurations[i] = fDurations[i]/1000;
	}
}

bool TPPG::CalculateCycleFromData(bool verbose)
{
	/// Calculate the cycle from the data and store it.
	if(fCycleSet) return true;

	// loop over all data and count how often we get each time difference between one state and the previous state
   std::map<ULong64_t, int> numberOfStateLengths[4]; // to calculate the length of each state (tape move, background, beam on, and decay)
	ULong64_t diff;
   for(PPGMap_t::iterator ppgIt = MapBegin(); ppgIt != MapEnd(); ++ppgIt) {
      switch(ppgIt->second->GetNewPPG()) {
			case EPpgPattern::kBackground:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kTapeMove);
				numberOfStateLengths[0][diff]++;
				break;
			case EPpgPattern::kBeamOn:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kBackground);
				numberOfStateLengths[1][diff]++;
				break;
			case EPpgPattern::kDecay:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kBeamOn);
				numberOfStateLengths[2][diff]++;
				break;
			case EPpgPattern::kTapeMove:
				diff = ppgIt->second->GetTimeStamp() - GetLastStatusTime(ppgIt->second->GetTimeStamp(), EPpgPattern::kDecay);
				numberOfStateLengths[3][diff]++;
				break;
			default: break;
      }
   }
	// set the durations to the one that occured the most often
	// (codes are already set)
   for(int i = 0; i < 4; ++i) {
      int counter = 0;
		if(verbose) std::cout<<"state "<<hex(fPPGCodes[i],4)<<" checking "<<numberOfStateLengths[i].size()<<" lengths:";
      for(auto it : numberOfStateLengths[i]) {
         if(it.second > counter) {
				if(verbose) std::cout<<" "<<it.second<<">"<<counter<<" => fDurations["<<i<<"] = "<<it.first;
            counter       = it.second;
            fDurations[i] = it.first;
         } else if(verbose) {
				std::cout<<" "<<it.second<<"<"<<counter<<" => fDurations["<<i<<"] != "<<it.first;
			}
      }
		if(verbose) std::cout<<std::endl;
   }
 
	fCycleSet = true;

	return true;
}

void TPPG::Clear(Option_t*)
{
   if(fPPGStatusMap != nullptr) {
      PPGMap_t::iterator ppgit;
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++) {
         if(ppgit->second != nullptr) {
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
   fOdbPPGCodes.clear();
   fOdbDurations.clear();
	fCycleSet = false;
}

Int_t TPPG::Write(const char*, Int_t, Int_t) const
{
   if(PPGSize() > 0 || OdbPPGSize() > 0) {
      std::cout<<"Writing PPG with "<<PPGSize()<<" events and "<<OdbPPGSize()<<" ODB settings"<<std::endl;
      return TObject::Write("TPPG", TObject::kSingleKey);
   }

   return 0;
}

void TPPG::Setup()
{
   if(TRunInfo::SubRunNumber() > 0) {
      auto* prevSubRun =
         new TFile(Form("fragment%05d_%03d.root", TRunInfo::RunNumber(), TRunInfo::SubRunNumber() - 1));
      if(prevSubRun != nullptr && prevSubRun->IsOpen()) {
         TPPG* prev_ppg = static_cast<TPPG*>(prevSubRun->Get("TPPG"));
         if(prev_ppg != nullptr) {
            prev_ppg->Copy(*this);
            std::cout<<"Found previous PPG data from run "<<prevSubRun->GetName()<<std::endl;
         } else {
            std::cout<<"Error, could not find PPG in file fragment"<<std::setfill('0')<<std::setw(5)<<TRunInfo::RunNumber()<<"_"<<std::setw(3)<<TRunInfo::SubRunNumber() - 1<<std::setfill(' ')<<".root, not adding previous PPG data"<<std::endl;
            std::cout<<"PPG set up."<<std::endl;
         }
         prevSubRun->Close();
      } else {
         std::cout<<"Error, could not find file fragment"<<std::setfill('0')<<std::setw(5)<<TRunInfo::RunNumber()<<"_"<<std::setw(3)<<TRunInfo::SubRunNumber() - 1<<std::setfill(' ')<<".root, not adding previous PPG data"<<std::endl;
         std::cout<<"PPG set up."<<std::endl;
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
            std::cout<<"Found "<<fNumberOfCycleLength.second<<" wrong cycle length(s) of "<<fNumberOfCycleLength.first<<" (correct is "<<fCycleLength<<")."<<std::endl;
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
            std::cout<<std::distance(MapBegin(), it)<<": found missing ppg at time "<<(*it).first<<" ("<<diff<<" != "<<fCycleLength<<")"<<std::endl;
         }
         // check that the previous ppg with the same status is a multiple of fCycleLength ago and that no other ppg is
         // in the map that was fCycleLength ago
         if(diff % fCycleLength != 0) {
            if(verbose) {
               std::cout<<DRED<<"PPG is messed up, cycle length is "<<fCycleLength<<", but the previous event with the same status was "<<diff<<" ago!"<<RESET_COLOR<<std::endl;
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
               std::cout<<DBLUE<<"PPG at "<<(*it).first - fCycleLength<<" already exist with status "<<hex(static_cast<std::underlying_type<EPpgPattern>::type>(prev->second->GetNewPPG()))<<" (current status is "<<hex(static_cast<std::underlying_type<EPpgPattern>::type>(it->second->GetNewPPG()))<<")."<<RESET_COLOR<<std::endl;
            }
            continue;
         }
         // copy the current ppg data and correct it's time before inserting it into the map
         auto*     new_data = new TPPGData(*((*it).second));
         ULong64_t new_ts   = (*it).first - fCycleLength;
         new_data->SetHighTimeStamp(new_ts >> 28);
         new_data->SetLowTimeStamp(new_ts & 0x0fffffff);
         if(verbose) {
            std::cout<<"inserting new ppg data at "<<new_data->GetTimeStamp()<<std::endl;
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
            std::cout<<"Found "<<fNumberOfCycleLength.second<<" wrong cycle length(s) of "<<fNumberOfCycleLength.first<<" (correct is "<<fCycleLength<<")."<<std::endl;
         }
      }
   }

   return true;
}

const TPPGData* TPPG::Next()
{
   if(++fCurrIterator != MapEnd()) {
      return fCurrIterator->second;
   }
   std::cout<<"Already at last PPG"<<std::endl;
   return nullptr;
}

const TPPGData* TPPG::Previous()
{
   if(fCurrIterator != MapBegin()) {
      return (--fCurrIterator)->second;
   }
   std::cout<<"Already at first PPG"<<std::endl;
   return nullptr;
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
   /// Stream an object of class TPPG.

   if(R__b.IsReading()) {
      R__b.ReadClassBuffer(TPPG::Class(), this);
      fCurrIterator = fPPGStatusMap->begin();
   } else {
      R__b.WriteClassBuffer(TPPG::Class(), this);
   }
}

ULong64_t TPPG::GetTimeInCycle(ULong64_t real_time)
{
	/// Returns the time in the cycle based on "real_time", i.e. the modulus of the 
	/// time and the cycle length.
	/// "real_time" is in ns since that's how we store the PPG timestamps as well.
   return real_time % GetCycleLength();
}

ULong64_t TPPG::GetCycleNumber(ULong64_t real_time)
{
	/// Returns the cycle number based on "real_time", i.e. the time divided by the
	/// cycle length.
	/// "real_time" is in ns since that's how we store the PPG timestamps as well.
   return real_time / GetCycleLength();
}

ULong64_t TPPG::GetCycleLength()
{
	/// Calculates cycle length from the most often occuring time difference between
	/// two PPG events with the same code. The result is also stored in fCycleLength
	/// (which prevents a second calculation of it the next time this function is called)
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

   while((ppg = static_cast<TPPG*>(it.Next())) != nullptr) {
      *this += *ppg;
   }

   return 0;
}

ULong64_t TPPG::GetStatusStart(EPpgPattern pat)
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
      if(ppgit->second != nullptr) {
         AddData(ppgit->second);
      }
   }
   // We want to rebuild our cycle length map and this is the easiest way to do it
   fNumberOfCycleLengths.clear();
   fCycleLength = 0;
   GetCycleLength();
	if(fOdbPPGCodes.empty()) {
		fOdbPPGCodes = ppg->fOdbPPGCodes;
		fOdbDurations = ppg->fOdbDurations;
	}
}
