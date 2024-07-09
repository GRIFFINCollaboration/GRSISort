#include "TSourceList.h"
#include "TCalPoint.h"

#include <iostream>

TSourceList::TSourceList()
{
   Clear();
}

TSourceList::TSourceList(const char* name, const char* title) : TCalList(name, title)
{
   Clear();
   SetNucleus(TNucleus(name));
}

TSourceList::TSourceList(const TNucleus& nuc, const char* name, const char* title) : TCalList(name, title)
{
   Clear();
   SetNucleus(nuc);
}

TSourceList::TSourceList(const TSourceList& copy) : TCalList(copy)
{
   copy.Copy(*this);
}

TSourceList::TSourceList(const TNucleus& nuc)
{
   Clear();
   SetNucleus(nuc);
}

void TSourceList::Copy(TObject& obj) const
{
   TCalList::Copy(obj);
}

void TSourceList::Print(Option_t*) const
{
   std::cout << "Nucleus: " << fNucleusName << std::endl;
   TCalList::Print();
}

void TSourceList::Clear(Option_t*)
{
   TCalList::Clear();
   fNucleusName.clear();
}

Int_t TSourceList::SetNucleus(const TNucleus& nuc)
{
   fNucleusName = nuc.GetName();

   Int_t        good_counter    = 0;
   const TList* transition_list = nuc.GetTransitionList();
   TIter        next(transition_list);
   TObject*     transition = nullptr;
   std::cout << "Adding Transitions..." << std::endl;
   while((transition = next()) != nullptr) {
      transition->Print();
      if(AddTransition(static_cast<TTransition*>(transition))) {
         good_counter++;
      }
   }
   return good_counter;
}

bool TSourceList::AddTransition(TTransition* tran)
{
   if(tran == nullptr) {
      std::cout << "Trying to add a bad transition" << std::endl;
      return false;
   }

   AddPoint(TCalPoint(tran->GetEnergy(), tran->GetIntensity(), tran->GetEnergyUncertainty(),
                      tran->GetIntensityUncertainty()));

   return true;
}
