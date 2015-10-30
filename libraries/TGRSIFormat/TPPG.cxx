
#include "TPPG.h"

ClassImp(TPPGData)
ClassImp(TPPG)

TPPGData::TPPGData(){
   Clear();
}

TPPGData::TPPGData(const TPPGData& rhs) : TObject() {
  ((TPPGData&)rhs).Copy(*this);
}

void TPPGData::Copy(TObject &rhs) const {
  ((TPPGData&)rhs).ftimestamp        =  ftimestamp;      
  ((TPPGData&)rhs).fold_ppg          =  fold_ppg;        
  ((TPPGData&)rhs).fnew_ppg          =  fnew_ppg;                
  ((TPPGData&)rhs).fNetworkPacketId  =  fNetworkPacketId;
  ((TPPGData&)rhs).flowtimestamp     =  flowtimestamp;   
  ((TPPGData&)rhs).fhightimestamp    =  fhightimestamp;  
}

void TPPGData::SetTimeStamp() {
   Long64_t time = GetHighTimeStamp();
   time  = time << 28;
   time |= GetLowTimeStamp() & 0x0fffffff;
   ftimestamp = time;
}

void TPPGData::Clear(Option_t* opt) {
//Clears the TPPGData and leaves it a "junk" state. By junk, I just mean default
//so that we can tell that this PPG is no good.
   ftimestamp        =  0;
   fold_ppg          =  0xFFFF;
   fnew_ppg          =  0xFFFF;         
   fNetworkPacketId  =  -1;
   flowtimestamp     =  0;
   fhightimestamp    =  0;
}

void TPPGData::Print(Option_t* opt) const{
   printf("time: %7lld\t PPG Status: 0x%07x\t Old: 0x%07x\n",GetTimeStamp(),fnew_ppg,fold_ppg); 
}

TPPG::TPPG() {
   fPPGStatusMap = new PPGMap_t;
   this->Clear();
}

TPPG::TPPG(const TPPG& rhs) : TObject() {
   fPPGStatusMap = new PPGMap_t;
   rhs.Copy(*this);
}

TPPG::~TPPG() {
   Clear();
   PPGMap_t::iterator ppgit;
   if(fPPGStatusMap){
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++){
         if(ppgit->second){
            delete (ppgit->second);
         }
         ppgit->second = 0;
      }
      delete fPPGStatusMap;
   }
   fPPGStatusMap = 0;
}

void TPPG::Copy(TObject &obj) const {
  ((TPPG&)obj).Clear();
   ((TPPG&)obj).fcurrIterator = ((TPPG&)obj).fPPGStatusMap->begin();//might not need this
   ((TPPG&)obj).fCycleLength =  fCycleLength;
   ((TPPG&)obj).fNumberOfCycleLengths = fNumberOfCycleLengths;

   //We want to provide a copy of each of the data in the PPG rather than a copy of th pointer
   if(((TPPG&)obj).fPPGStatusMap && fPPGStatusMap){
      PPGMap_t::iterator ppgit;
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++){
         if(ppgit->second){
            ((TPPG&)obj).AddData(ppgit->second);
         }
      }
   }

}

Bool_t TPPG::MapIsEmpty() const {
//Checks to see if the ppg map is empty. We need this because we need to put a default
//PPG in at time T=0 to prevent bad things from happening. This function says the map
//is empty when only the default is there, which it essentially is.
   if(fPPGStatusMap->size() ==1)//We check for size 1 because we always start with a Junk event at time 0.
      return true;
   else 
      return false;
}

void TPPG::AddData(TPPGData* pat){
//Adds a PPG status word at a given time in the current run. Makes a copy of the pointer to
//store in the map.
   fPPGStatusMap->insert(std::make_pair(pat->GetTimeStamp(),new TPPGData(*pat)));
   fCycleLength = 0;
   fNumberOfCycleLengths.clear();
}

ULong64_t TPPG::GetLastStatusTime(ULong64_t time,ppg_pattern pat,bool exact_flag){
//Gets the last time that a status was given. If the ppg_pattern kJunk is passed, the 
//current status at the time "time" is looked for. If exact_flag is false, the bits of "pat" 
//are looked for and ignore the rest of the bits in the sotred statuses. If "exact_flag" 
//is true, the entire ppg pattern "pat" must be met.
   if(MapIsEmpty()){
      printf("Empty\n");
      return 0;
   }

   PPGMap_t::iterator curppg_it = --(fPPGStatusMap->upper_bound(time));
   PPGMap_t::iterator ppg_it;
   if(pat == kJunk){
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it){
         if(curppg_it->second->GetNewPPG() == ppg_it->second->GetNewPPG() && curppg_it != ppg_it ){
            return ppg_it->first;
         }
      }
   }
   else{
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it){
         if(exact_flag){
            if(pat == ppg_it->second->GetNewPPG()){
               return ppg_it->first;
            }
         }
         else{
            printf("pat %x, ppg_it->first %lu, ppg_it->second->GetNewPPG() %x\n",pat,ppg_it->first,ppg_it->second->GetNewPPG());
            if(pat & ppg_it->second->GetNewPPG()){
               return ppg_it->first;
            }
         }
      }
   }
   //printf("No previous status\n");
   return 0;
}

uint16_t TPPG::GetStatus(ULong64_t time) const {
//Returns the current status of the PPG at the time "time".
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   //The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one because we want to know what the last PPG event was.
   return (uint16_t)((--(fPPGStatusMap->upper_bound(time)))->second->GetNewPPG());
}

void TPPG::Print(Option_t *opt) {
   if(MapIsEmpty()) {
      printf("Empty\n");
   } else {
		if(TString(opt).Contains("all",TString::ECaseCompare::kIgnoreCase)) {
			//print all ppg data
			PPGMap_t::iterator ppgit;
			printf("*****************************\n");
			printf("           PPG STATUS        \n");
			printf("*****************************\n");
			for(ppgit = MapBegin(); ppgit != MapEnd(); ppgit++) {
				ppgit->second->Print();
			}
		} else {
			//print only an overview of the ppg
			//first calculate how often each different status occured
			std::map<uint16_t, int> status;
			for(auto it = MapBegin(); it != MapEnd(); ++it) {
				status[it->second->GetNewPPG()]++;
			}
			printf("Cycle length is %lld in 10 ns units = %.3lf seconds.\n", GetCycleLength(), GetCycleLength()/1e8);
			printf("Got %ld PPG words:\n", fPPGStatusMap->size() - 1);
			for(auto it = status.begin(); it != status.end(); ++it) {
				printf("\tfound status 0x%04x %d times\n", it->first, it->second);
			}
		}
   }
}

void TPPG::Clear(Option_t *opt){
   if(fPPGStatusMap){
      PPGMap_t::iterator ppgit;
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++){
         if(ppgit->second){
            delete (ppgit->second);
         }
         ppgit->second = 0;
      }
   }
   fPPGStatusMap->clear();
   //We always add a junk event to keep the code from crashing if we ask for a PPG below the lowest PPG time.
   AddData(new TPPGData);
   fcurrIterator = fPPGStatusMap->begin();
   fCycleLength = 0;
   fNumberOfCycleLengths.clear();
}

bool TPPG::Correct(bool verbose) {
   //Correct the map of PPG events by checking that we always have a PPG event with the same status that occured fCycleLength ago.

   //we use the cycle time to correct our ppg, so make sure it's set (GetCycleLength calculates and sets fCycleLength only if it's not already set)
   GetCycleLength();
   
   if(verbose) {
      //we can now use fNumberOfCycleLengths to see how many cycle lengths we have that are wrong
      for(auto it = fNumberOfCycleLengths.begin(); it != fNumberOfCycleLengths.end(); ++it) {
         if((*it).first < fCycleLength) {
            continue;
         }
         if((*it).first != fCycleLength) {
            printf("Found %d wrong cycle length(s) of %lld (correct is %lld).\n", (*it).second, (*it).first, fCycleLength);
         }
      }
   }
   
   //loop over all ppg data, check that the last ppg data of the same type was fCycleLength ago
   //if it's not, enter ppg data at that time (multiple times?)
   for(auto it = MapBegin(); it != MapEnd(); it++){
      //skip the first cycle
      if((*it).first < fCycleLength) {
         continue;
      }
      //get time difference to previous ppg with same status
      ULong64_t diff = (*it).second->GetTimeStamp() - GetLastStatusTime((*it).second->GetTimeStamp());
      if(diff != fCycleLength) {
         if(verbose) {
            printf("%ld: found missing ppg at time %lu (%lld != %lld)\n", std::distance(MapBegin(),it),(*it).first, diff, fCycleLength);
         }
         //check that the previous ppg with the same status is a multiple of fCycleLength ago and that no other ppg is in the map that was fCycleLength ago
         if(diff%fCycleLength != 0) {
            if(verbose) {
               printf(DRED "PPG is messed up, cycle length is %lld, but the previous event with the same status was %lld ago!" RESET_COLOR "\n", fCycleLength, diff);
            }
            continue;
            return false;
         }
         auto prev = fPPGStatusMap->find((*it).first - fCycleLength);
         if(prev != fPPGStatusMap->end()) {
            //check if the status of the current ppg and the old status of the ppg following the one fCycleLength ago are the same
            //if they are, we change the status of the one fCycleLength ago to match the current status
            if(it->second->GetNewPPG() == (++prev)->second->GetOldPPG()) {
               (--prev)->second->SetNewPPG(it->second->GetNewPPG());
            } else if(verbose) {
               printf(DBLUE "PPG at %lld already exist with status 0x%x (current status is 0x%x)." RESET_COLOR "\n", (*it).first - fCycleLength, prev->second->GetNewPPG(), it->second->GetNewPPG());
            }
            continue;
         }
         //copy the current ppg data and correct it's time before inserting it into the map
         TPPGData* new_data = new TPPGData(*((*it).second));
         ULong64_t new_ts = (*it).first - fCycleLength;
         new_data->SetHighTimeStamp(new_ts>>28);
         new_data->SetLowTimeStamp(new_ts&0x0fffffff);
         if(verbose) {
            printf("inserting new ppg data at %lld\n",new_data->GetTimeStamp());
         }
         it = fPPGStatusMap->insert(std::make_pair(new_data->GetTimeStamp(),new_data)).first;
         --it;
      }
   }
      
   //now we reset fCycleLength and calculate it again
   fCycleLength = 0;
   fNumberOfCycleLengths.clear();
   GetCycleLength();
   
   if(verbose) {
      //we can now use fNumberOfCycleLengths to see how many cycle lengths we have that are wrong
      for(auto it = fNumberOfCycleLengths.begin(); it != fNumberOfCycleLengths.end(); ++it) {
         if((*it).first < fCycleLength) {
            continue;
         }
         if((*it).first != fCycleLength) {
            printf("Found %d wrong cycle length(s) of %lld (correct is %lld).\n", (*it).second, (*it).first, fCycleLength);
         }
      }
   }
   
   return true;
}

const TPPGData* TPPG::Next() {
   if(++fcurrIterator != MapEnd()){
      return fcurrIterator->second;
   }
   else{
      printf("Already at last PPG\n");
      return 0;
   }
}

const TPPGData* TPPG::Previous() {
   if(fcurrIterator != MapBegin()){
      return (--fcurrIterator)->second;
   }
   else{
      printf("Already at first PPG\n");
      return 0;
   }
}

const TPPGData* TPPG::Last(){
   fcurrIterator = MapEnd();
   --fcurrIterator;
   return fcurrIterator->second;
}

const TPPGData* TPPG::First(){
   fcurrIterator = MapBegin();
   return fcurrIterator->second;
}

void TPPG::Streamer(TBuffer &R__b)
{
   // Stream an object of class TPPG.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TPPG::Class(),this);
      fcurrIterator = fPPGStatusMap->begin();
   } else {
      R__b.WriteClassBuffer(TPPG::Class(),this);
   }
}

/*const char* TPPG::ConvertStatus(ppg_pattern pattern){
   switch(pattern){
      case kBeamOn:        { return "Beam On";}
      case kDecay:         { return "Decay";}
      case kBackground:    { return "Background";}
      case kTapeMove:      { return "Tape Move";}
      default:             { return "Junk";}
   };


}*/

ULong64_t TPPG::GetTimeInCycle(ULong64_t real_time){
   return real_time%GetCycleLength();
}

ULong64_t TPPG::GetCycleNumber(ULong64_t real_time){
   return real_time/GetCycleLength();
}

ULong64_t TPPG::GetCycleLength() {
   if(fCycleLength == 0) {
      PPGMap_t::iterator ppgit;
      for(ppgit = MapBegin(); ppgit != MapEnd(); ++ppgit){
         ULong64_t diff = ppgit->second->GetTimeStamp() - GetLastStatusTime(ppgit->second->GetTimeStamp());
         fNumberOfCycleLengths[diff]++;
      }
      int counter =0;
      for(auto it=fNumberOfCycleLengths.begin(); it!=fNumberOfCycleLengths.end();++it){
         if(it->second > counter) {
            counter = it->second;
            fCycleLength = it->first;
         }
      }
   }

   return fCycleLength;
}

ULong64_t TPPG::GetNumberOfCycles() {
   return Last()->GetTimeStamp()/GetCycleLength();
}

Long64_t TPPG::Merge(TCollection *list){
   TIter it(list);
   TPPG *ppg = 0;

   while ((ppg = (TPPG *)it.Next())){
      *this += *ppg;
   }
   
   return 0;

}

ULong64_t TPPG::GetStatusStart(ppg_pattern pat) {
  //return the time in the cycle when pat starts
  return GetTimeInCycle(GetLastStatusTime(Last()->GetTimeStamp(), pat));
}

void TPPG::operator+=(const TPPG& rhs){                           
   this->Add(&rhs);    
}

void TPPG::Add(const TPPG* ppg){
   PPGMap_t::iterator ppgit;
   for(ppgit = ppg->MapBegin(); ppgit != ppg->MapEnd(); ++ppgit){
      if(ppgit->second){
         AddData(ppgit->second);
      }
   }
   //We want to rebuild our cycle length map and this is the easiest way to do it
   fNumberOfCycleLengths.clear();
   fCycleLength = 0;
   GetCycleLength();

}
