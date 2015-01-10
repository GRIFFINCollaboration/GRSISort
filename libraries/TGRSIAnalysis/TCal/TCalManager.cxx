#include "TCalManager.h"

ClassImp(TCalManager)

TCalManager::TCalManager(){}

TCalManager::~TCalManager(){
   CalMap::iterator iter;
   for(iter = fcalmap.begin(); iter != fcalmap.end(); iter++)   {
		if(iter->second)
	      delete iter->second;
      iter->second = 0;
   }
}


TCal *TCalManager::GetCal(UInt_t channum) {
   TCal *cal  = 0;
   try {
	   cal = fcalmap.at(channum);
   } 
   catch(const std::out_of_range& oor) {
	   return 0;
   }
   return cal;
}

void TCalManager::AddToManager(TCal* cal, Option_t *opt) {
//Makes a Deep copy of cal and adds it to the CalManager Map. 

   //Check to see if the channel number has been set. If not, the user must supply one in the function call.
   if(cal->GetChannelNumber() != 9999){
      AddToManager(cal,cal->GetChannelNumber(),opt);
   }
   else{
      printf("Channel Number has not been set\n");
      return;
   }
   //Might have to do other checks

}
   
void TCalManager::AddToManager(TCal* cal, UInt_t channum, Option_t *opt) {
//Makes a Deep copy of cal and adds it to the CalManager Map for channel number
//channum.

   if(channum == 9999){
      printf("Channel Number has not been set\n");
      return;
   }
   
   if(!cal)
      return;
   if(fcalmap.count(channum)==1) {// if this cal already exists
	   if(strcmp(opt,"overwrite")==0) {
	      TCal *oldcal = GetCal(channum);
         //delete the old calibration for this channel number
         delete oldcal;
			oldcal = (TCal*)cal->Clone(cal->GetName());
         fcalmap.at(channum) = oldcal;
			return;
	   } 
      else {
	      printf("Trying to add a channel that already exists!\n");
			return;
	   }	
   } 
   else {
      TCal* newcal = (TCal*)cal->Clone(cal->GetName());
      fcalmap.insert(std::make_pair(channum,newcal));
    }
   
}




