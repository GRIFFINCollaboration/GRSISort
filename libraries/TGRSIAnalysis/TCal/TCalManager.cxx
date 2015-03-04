#include "TCalManager.h"

#include <stdexcept>
   
ClassImp(TCalManager)

TCalManager::TCalManager(){
   fClass = 0; //fClass will point to a TClass which is made persistant through a root session within gROOT.
               //So we don't need to worry about allocating it.
}

TCalManager::TCalManager(const char* classname){
   fClass = 0;
   this->SetClass(classname);
}

TCalManager::~TCalManager(){
   CalMap::iterator iter;
   for(iter = fcalmap.begin(); iter != fcalmap.end(); iter++)   {
		if(iter->second)
	      delete iter->second;
      iter->second = 0;
   }
}

void TCalManager::RemoveCal(UInt_t channum, Option_t *opt){
   if(fcalmap.count(channum)==1) {// if this cal exists
      TCal* cal = GetCal(channum);
      delete cal;
      fcalmap.erase(channum);
   }
}

void TCalManager::SetClass(const char* classname){
//Sets the Derived class of the TCal being held in the TCalManager
   SetClass(TClass::GetClass(classname));
}

void TCalManager::SetClass(const TClass* cl){
//Sets the Derived class of the TCal being held in the TCalManager
  if(fClass) {
     printf("TCalManager type already set to %s\n",fClass->ClassName());
     return;
  }

  fClass = (TClass*)cl;
  //fClass = cl;
  if(!fClass) {
     MakeZombie();
     Error("SetClass", "called with a null pointer");
     return;
  }
  const char *classname = fClass->GetName();
  if(!fClass->InheritsFrom(TCal::Class())) {
      MakeZombie();
      Error("SetClass", "%s does not inherit from TCal", classname);
      return;
  }
  if(fClass->GetBaseClassOffset(TObject::Class())!=0) {
      MakeZombie();
      Error("SetClass", "%s must inherit from TObject as the left most base class.", classname);
      return;      
  }
  printf("Changing TCalManager to type: %s\n",classname);
  Int_t nch = strlen(classname)+2;
  char *name = new char[nch];
  snprintf(name,nch, "%ss", classname);
  SetName(name);
  delete [] name;

}

TCal *TCalManager::GetCal(UInt_t channum) {
//Gets the TCal for the channel number channum
   TCal *cal  = 0;
   try {
	   cal = fcalmap.at(channum);
   } 
   catch(const std::out_of_range& oor) {
      Error("GetCal","Channel %u is empty",channum);
	   return 0;
   }
   return cal;
}

Bool_t TCalManager::AddToManager(TCal* cal, Option_t *opt) {
//Makes a Deep copy of cal and adds it to the CalManager Map. 

   //Check to see if the channel number has been set. If not, the user must supply one in the function call.
   if(cal->GetChannel() != 0){
      return AddToManager(cal,cal->GetChannel()->GetNumber(),opt);
   }
   else{
      Error("AddToManager","Channel has not been set");
      return false;
   }
   //Might have to do other checks

}
   
Bool_t TCalManager::AddToManager(TCal* cal, UInt_t channum, Option_t *opt) {
//Makes a Deep copy of cal and adds it to the CalManager Map for channel number
//channum.
   if(!cal)
      return false;

   if(!fClass)
      SetClass(cal->ClassName());
   else if(fClass->GetName() != cal->ClassName()){
     Error("AddToManager","Trying to put a %s in a TCalManager of type %s",cal->ClassName(),fClass->GetName());
     return false;
   }
   
   if(!(cal->GetChannel()))
      if(!(cal->SetChannel(channum)))
         return false; //TCal does the Error for us.
   
   if(fcalmap.count(cal->GetChannel()->GetNumber())==1) {// if this cal already exists
	   if(strcmp(opt,"overwrite")==0) {
	      TCal *oldcal = GetCal(channum);
         //delete the old calibration for this channel number
         delete oldcal;
			oldcal = (TCal*)cal->Clone(cal->GetName());
         fcalmap.at(channum) = oldcal;
			return true;
	   } 
      else {
	      Error("AddToManager","Trying to add a channel that already exists!");
			return false;
	   }	
   } 
   else {
      TCal* newcal = (TCal*)cal->Clone(cal->GetName());
      //In order to construct a new derived class you need to know the type at compile time.
      //Clone lets us get around this. There are other ways to do this using "virtual constructor idioms"
      //but the basically do what clone does anyway.
      //Clone uses ROOT streamers. We have made the TChannel part of the TCal a TRef.
      //This has the effect of making it persistent as far as the ROOT streamer
      //facility is concerned. All of the other "pointer members" of the TCal
      //Get Deep copied into the TCal Manager.
      printf("newcal: %p, cal: %p\n",newcal->GetChannel(),cal->GetChannel());
      fcalmap.insert(std::make_pair(channum,newcal));
    }
   return true;
   
}

void TCalManager::WriteToChannel() const {
//Writes all of the TCals to TChannel based on the method WriteToChannel 
//defined in the TCal held by TCalManager.
   CalMap::const_iterator iter;
   for(iter = fcalmap.begin(); iter != fcalmap.end(); iter++)   {
		if(iter->second)
         iter->second->WriteToChannel();
   }
}

void TCalManager::Clear(Option_t *opt) {
//This deletes all of the current TCal's. It also resets the class
//type to 0.
   CalMap::iterator iter;
   for(iter = fcalmap.begin(); iter != fcalmap.end(); iter++)   {
		if(iter->second)
	      delete iter->second;
      iter->second = 0;
   }
   fcalmap.clear();
   fClass = 0;
}

void TCalManager::Print(Option_t *opt) const{
   if(fClass)
      printf("Type: %s\n", fClass->GetName());
      std::cout << "Size: " << fcalmap.size() << std::endl; //Printing this way due to size_type return
}



