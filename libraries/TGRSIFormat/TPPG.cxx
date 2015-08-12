
#include "TPPG.h"

ClassImp(TPPGData)
ClassImp(TPPG)

TPPGData::TPPGData(){
   Clear();
}

TPPGData::TPPGData(const TPPGData& rhs) {
  ((TPPGData&)rhs).Copy(*this);
}

void TPPGData::Copy(TObject &rhs) const {
  ((TPPGData&)rhs)     = *this;
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

TPPG::TPPG(){
   fPPGStatusMap = new PPGMap_t;
   this->Clear();
}

TPPG::~TPPG(){
   if(fPPGStatusMap){
      PPGMap_t::iterator ppgit;
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++){
         if(ppgit->second){
            delete ppgit->second;
         }
         ppgit->second = 0;
      }
      delete fPPGStatusMap;
   }
   fPPGStatusMap = 0;
}

Bool_t TPPG::MapIsEmpty() const {
   if(fPPGStatusMap->size() ==1)//We check for size 1 because we always start with a Junk event at time 0.
      return true;
   else 
      return false;
}

void TPPG::AddData(TPPGData* pat){
//Adds a PPG status word at a given time in the current run. Makes a copy of the pointer to
//store in the map.
   fPPGStatusMap->insert(std::make_pair(pat->GetTimeStamp(),new TPPGData(*pat)));
}

ULong64_t TPPG::GetLastStatusTime(ULong64_t time,ppg_pattern pat,bool exact_flag){
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
            if(pat | ppg_it->second->GetNewPPG()){
               return ppg_it->first;
            }
         }
      }
   }
   printf("No previous status\n");
   return 0;
}

uint16_t TPPG::GetStatus(ULong64_t time) const {
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   //The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one because we want to know what the last PPG event was.
   return (uint16_t)((--(fPPGStatusMap->upper_bound(time)))->second->GetNewPPG());
}

void TPPG::Print(Option_t *opt) const{
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   else{
      PPGMap_t::iterator ppgit;
      printf("*****************************\n");
      printf("           PPG STATUS        \n");
      printf("*****************************\n");
      for(ppgit = MapBegin(); ppgit != MapEnd(); ppgit++){
         ppgit->second->Print();
      }
   }
}

void TPPG::Clear(Option_t *opt){
   fPPGStatusMap->clear();
   //We always add a junk event to keep the code from crashing if we ask for a PPG below the lowest PPG time.
   AddData(new TPPGData);
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
