#include "TCalManager.h"

#include <stdexcept>

/// \cond CLASSIMP
ClassImp(TCalManager)
/// \endcond

TCalManager::TCalManager()
{
   fClass = nullptr;   // fClass will point to a TClass which is made persistant through a root session within gROOT.
   // So we don't need to worry about allocating it.
}

TCalManager::TCalManager(const char* classname)
{
   fClass = nullptr;
   SetClass(classname);
}

TCalManager::~TCalManager()
{
   CalMap::iterator iter;
   for(iter = fCalMap.begin(); iter != fCalMap.end(); iter++) {
      if(iter->second != nullptr) {
         delete iter->second;
      }
      iter->second = nullptr;
   }
}

void TCalManager::RemoveCal(UInt_t channum, Option_t*)
{
   if(fCalMap.count(channum) == 1) {   // if this cal exists
      TCal* cal = GetCal(channum);
      delete cal;
      fCalMap.erase(channum);
   }
}

void TCalManager::SetClass(const char* className)
{
   /// Sets the Derived class of the TCal being held in the TCalManager
   SetClass(TClass::GetClass(className));
}

void TCalManager::SetClass(const TClass* cl)
{
   /// Sets the Derived class of the TCal being held in the TCalManager
   if(fClass != nullptr) {
      std::cout << "TCalManager type already set to " << fClass->ClassName() << std::endl;
      return;
   }

   fClass = const_cast<TClass*>(cl);
   if(fClass == nullptr) {
      MakeZombie();
      Error("SetClass", "called with a null pointer");
      return;
   }
   const char* className = fClass->GetName();
   if(!fClass->InheritsFrom(TCal::Class())) {
      MakeZombie();
      Error("SetClass", "%s does not inherit from TCal", className);
      return;
   }
   if(fClass->GetBaseClassOffset(TObject::Class()) != 0) {
      MakeZombie();
      Error("SetClass", "%s must inherit from TObject as the left most base class.", className);
      return;
   }
   std::cout << "Changing TCalManager to type: " << className << std::endl;
   Int_t nch  = strlen(className) + 2;
   auto* name = new char[nch];
   snprintf(name, nch, "%ss", className);
   SetName(name);
   delete[] name;
}

TCal* TCalManager::GetCal(UInt_t chanNum)
{
   /// Gets the TCal for the channel number chanNum
   TCal* cal = nullptr;
   try {
      cal = fCalMap.at(chanNum);
   } catch(const std::out_of_range& oor) {
      Error("GetCal", "Channel %u is empty", chanNum);
      return nullptr;
   }
   return cal;
}

Bool_t TCalManager::AddToManager(TCal* cal, Option_t* opt)
{
   /// Makes a Deep copy of cal and adds it to the CalManager Map.

   // Check to see if the channel number has been set. If not, the user must supply one in the function call.
   if(cal->GetChannel() != nullptr) {
      return AddToManager(cal, cal->GetChannel()->GetNumber(), opt);
   }
   Error("AddToManager", "Channel has not been set");
   return false;

   // Might have to do other checks
}

Bool_t TCalManager::AddToManager(TCal* cal, UInt_t chanNum, Option_t* opt)
{
   /// Makes a Deep copy of cal and adds it to the CalManager Map for channel number
   /// chanNum.
   if(cal == nullptr) {
      return false;
   }

   if(fClass == nullptr) {
      SetClass(cal->ClassName());
   } else if(fClass->GetName() != cal->ClassName()) {
      Error("AddToManager", "Trying to put a %s in a TCalManager of type %s", cal->ClassName(), fClass->GetName());
      return false;
   }

   if((cal->GetChannel()) == nullptr) {
      if(!(cal->SetChannel(chanNum))) {
         return false;   // TCal does the Error for us.
      }
   }

   if(fCalMap.count(cal->GetChannel()->GetNumber()) == 1) {   // if this cal already exists
      if(strcmp(opt, "overwrite") == 0) {
         TCal* oldCal = GetCal(chanNum);
         // delete the old calibration for this channel number
         delete oldCal;
         oldCal              = static_cast<TCal*>(cal->Clone(cal->GetName()));
         fCalMap.at(chanNum) = oldCal;
         return true;
      }
      Error("AddToManager", "Trying to add a channel that already exists!");
      return false;
   }
   TCal* newCal = static_cast<TCal*>(cal->Clone(cal->GetName()));
   // In order to construct a new derived class you need to know the type at compile time.
   // Clone lets us get around this. There are other ways to do this using "virtual constructor idioms"
   // but the basically do what clone does anyway.
   // Clone uses ROOT streamers. We have made the TChannel part of the TCal a TRef.
   // This has the effect of making it persistent as far as the ROOT streamer
   // facility is concerned. All of the other "pointer members" of the TCal
   // Get Deep copied into the TCal Manager.
   std::cout << "newCal: " << newCal->GetChannel() << ", cal: " << cal->GetChannel() << std::endl;
   ;
   fCalMap.insert(std::make_pair(chanNum, newCal));

   return true;
}

void TCalManager::WriteToChannel() const
{
   /// Writes all of the TCals to TChannel based on the method WriteToChannel
   /// defined in the TCal held by TCalManager.
   for(const auto& iter : fCalMap) {
      if(iter.second != nullptr) {
         iter.second->WriteToChannel();
      }
   }
}

void TCalManager::Clear(Option_t*)
{
   /// This deletes all of the current TCal's. It also resets the class
   /// type to 0.
   for(auto& iter : fCalMap) {
      if(iter.second != nullptr) {
         delete iter.second;
      }
      iter.second = nullptr;
   }
   fCalMap.clear();
   fClass = nullptr;
}

void TCalManager::Print(Option_t*) const
{
   if(fClass != nullptr) {
      std::cout << "Type: " << fClass->GetName() << std::endl;
   }
   std::cout << "Size: " << fCalMap.size() << std::endl;   // Printing this way due to size_type return
}
