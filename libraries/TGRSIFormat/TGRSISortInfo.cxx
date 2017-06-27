#include "TGRSISortInfo.h"

#include <iostream>

/// \cond CLASSIMP
ClassImp(TGRSISortInfo)
   /// \endcond
   /// \cond CLASSIMP
   ClassImp(TGRSISortList)
   /// \endcond

   Bool_t TGRSISortList::AddSortInfo(TGRSISortInfo* info, Option_t* opt)
{
   TString opt1 = opt;
   opt1.ToUpper();

   if(fSortInfoList.count(info->RunNumber()) && fSortInfoList[info->RunNumber()].count(info->SubRunNumber())) {
      if(opt1 == "SAFE") {
         std::cout<<"Can't add Run: "<<info->RunNumber()<<"\tSubRun "<<info->SubRunNumber()<<std::endl;
         std::cout<<"Already in Sort Info List"<<std::endl;
         return false;
      }
      info->AddDuplicate();
      std::cout<<"Adding Duplicate  Run: "<<info->RunNumber()<<"\tSubRun "<<info->SubRunNumber()<<std::endl;
      return true;
   }
   std::cout<<"Adding Run: "<<info->RunNumber()<<"\tSubRun "<<info->SubRunNumber()<<std::endl;
   fSortInfoList[info->RunNumber()].insert(std::make_pair(info->SubRunNumber(), info));
   return true;
}

TGRSISortInfo* TGRSISortList::GetSortInfo(Int_t RunNumber, Int_t SubRunNumber)
{
   return (fSortInfoList.find(RunNumber)->second.find(SubRunNumber)->second);
}

void TGRSISortList::Print(Option_t*) const
{
   info_map::const_iterator it;
   std::map<Int_t, TGRSISortInfo*>::const_iterator lil_it;
   for(it = fSortInfoList.begin(); it != fSortInfoList.end(); it++) {
      for(lil_it = it->second.begin(); lil_it != it->second.end(); lil_it++) {
         lil_it->second->Print();
      }
   }
}

void TGRSISortList::Clear(Option_t*)
{
   fSortInfoList.clear();
}

Bool_t TGRSISortList::AddSortList(TGRSISortList* rhslist, Option_t*)
{
   // Adds another TGRSISortList to the current Sort list.
   info_map::iterator it;
   std::map<Int_t, TGRSISortInfo*>::iterator lil_it;
   info_map* rhsmap = rhslist->GetMap();
   for(it = rhsmap->begin(); it != rhsmap->end(); it++) {
      for(lil_it = it->second.begin(); lil_it != it->second.end(); lil_it++) {
         // We need to clone the TGRSISortInfo so that we have ownership in the new list
         AddSortInfo(static_cast<TGRSISortInfo*>(lil_it->second->Clone()));
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

   while((sortlist = dynamic_cast<TGRSISortList*>(it.Next()))) {
      // Now we want to loop through each TGRSISortList and find the TGRSISortInfo's stored in there.
      this->AddSortList(sortlist);
   }
   return 0;

   //  fRunInfoList.AddAll(list);
}

TGRSISortInfo::TGRSISortInfo(const TGRSIRunInfo* info)
{
   Clear();
   SetRunInfo(info);
}

TGRSISortInfo::TGRSISortInfo()
{
   Clear();
}

TGRSISortInfo::~TGRSISortInfo() = default;

void TGRSISortInfo::SetRunInfo(const TGRSIRunInfo* info)
{
   fRunNumber    = info->RunNumber();
   fSubRunNumber = info->SubRunNumber();
}

void TGRSISortInfo::Print(Option_t*) const
{
   std::cout<<"Run Number:\t "<<RunNumber();
   std::cout<<"\tSub Run:\t "<<SubRunNumber();
   std::cout<<"\tComment:\t "<<Comment();
   if(fDuplicates) {
      std::cout<<"\tDuplicates:\t"<<fDuplicates;
   }
   std::cout<<std::endl;
}

void TGRSISortInfo::Clear(Option_t*)
{
   fRunNumber    = 0;
   fSubRunNumber = 0;
   fDuplicates   = 0;
   fComment      = "";
}
