#include "TGRSISortInfo.h"

#include <iostream>

Bool_t TGRSISortList::AddSortInfo(TGRSISortInfo* info, Option_t* opt)
{
   TString opt1 = opt;
   opt1.ToUpper();

   if((fSortInfoList.count(info->RunNumber()) != 0u) &&
      (fSortInfoList[info->RunNumber()].count(info->SubRunNumber()) != 0u)) {
      if(opt1 == "SAFE") {
         std::cout << "Can't add Run: " << info->RunNumber() << "\tSubRun " << info->SubRunNumber() << std::endl;
         std::cout << "Already in Sort Info List" << std::endl;
         return false;
      }
      info->AddDuplicate();
      std::cout << "Adding Duplicate  Run: " << info->RunNumber() << "\tSubRun " << info->SubRunNumber() << std::endl;
      return true;
   }
   std::cout << "Adding Run: " << info->RunNumber() << "\tSubRun " << info->SubRunNumber() << std::endl;
   fSortInfoList[info->RunNumber()].insert(std::make_pair(info->SubRunNumber(), info));
   return true;
}

TGRSISortInfo* TGRSISortList::GetSortInfo(Int_t RunNumber, Int_t SubRunNumber)
{
   return (fSortInfoList.find(RunNumber)->second.find(SubRunNumber)->second);
}

void TGRSISortList::Print(Option_t*) const
{
   for(auto map : fSortInfoList) {
      for(auto item : map.second) {
         item.second->Print();
      }
   }
}

void TGRSISortList::Clear(Option_t*)
{
   fSortInfoList.clear();
}

Bool_t TGRSISortList::AddSortList(TGRSISortList* rhslist, Option_t*)
{
   /// Adds another TGRSISortList to the current Sort list.
   for(auto map : rhslist->fSortInfoList) {
      for(auto item : map.second) {
         // We need to clone the TGRSISortInfo so that we have ownership in the new list
         AddSortInfo(static_cast<TGRSISortInfo*>(item.second->Clone()));
         // We might not need the clone, but that will take some checking.
      }
   }
   return true;
}

Long64_t TGRSISortList::Merge(TCollection* list)
{
   // Loop through the TCollection of TGRSISortLists, and add each entry to the original TGRSISort List
   TIter it(list);
   // The TCollection will be filled by something like hadd. Each element in the list will be a TGRSISortList from
   // An individual file that was submitted to hadd.
   TGRSISortList* sortlist = nullptr;

   while((sortlist = static_cast<TGRSISortList*>(it.Next())) != nullptr) {
      // Now we want to loop through each TGRSISortList and find the TGRSISortInfo's stored in there.
      AddSortList(sortlist);
   }
   return 0;

   //  fRunInfoList.AddAll(list);
}

TGRSISortInfo::TGRSISortInfo()
{
   Clear();
   SetRunInfo();
}

TGRSISortInfo::~TGRSISortInfo() = default;

void TGRSISortInfo::SetRunInfo()
{
   fRunNumber    = TRunInfo::RunNumber();
   fSubRunNumber = TRunInfo::SubRunNumber();
}

void TGRSISortInfo::Print(Option_t*) const
{
   std::cout << "Run Number:\t " << RunNumber();
   std::cout << "\tSub Run:\t " << SubRunNumber();
   std::cout << "\tComment:\t " << Comment();
   if(fDuplicates != 0u) {
      std::cout << "\tDuplicates:\t" << fDuplicates;
   }
   std::cout << std::endl;
}

void TGRSISortInfo::Clear(Option_t*)
{
   fRunNumber    = 0;
   fSubRunNumber = 0;
   fDuplicates   = 0;
   fComment      = "";
}
