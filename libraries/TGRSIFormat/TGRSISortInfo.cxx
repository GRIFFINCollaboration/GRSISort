#include "TGRSISortInfo.h"

#include <iostream>

ClassImp(TGRSISortInfo)
ClassImp(TGRSISortList)

Bool_t TGRSISortList::AddSortInfo(TGRSISortInfo *info, Option_t *opt){
   TString opt1 = opt;
   opt1.ToUpper();
      
   if(fSortInfoList.count(info->RunNumber()) && fSortInfoList[info->RunNumber()].count(info->SubRunNumber())){
      if(opt1 == "FORCE"){
         info->AddDuplicate();
         std::cout << "Adding Duplicate  Run: " << info->RunNumber() << "\tSubRun " << info->SubRunNumber() << std::endl;
         return true;
      }
      std::cout << "Can't add Run: " << info->RunNumber() << "\tSubRun " << info->SubRunNumber() << std::endl;
      std::cout << "Already in Sort Info List" << std::endl;
      return false;
   }
   std::cout << "Adding Run: " << info->RunNumber() << "\tSubRun " << info->SubRunNumber() << std::endl;
   fSortInfoList[info->RunNumber()].insert(std::make_pair(info->SubRunNumber(), info));
   return true;
}

TGRSISortInfo* TGRSISortList::GetSortInfo(Int_t RunNumber, Int_t SubRunNumber){
   return (fSortInfoList.find(RunNumber)->second.find(SubRunNumber)->second);
}

void TGRSISortList::Print(Option_t* opt) const{
   std::map<Int_t,std::map<Int_t,TGRSISortInfo*>>::const_iterator it;
   std::map<Int_t,TGRSISortInfo*>::const_iterator lil_it;
   for(it = fSortInfoList.begin(); it != fSortInfoList.end(); it++){
      for(lil_it = it->second.begin(); lil_it != it->second.end(); lil_it++){
         lil_it->second->Print();
      }
   }
}
     
void TGRSISortList::Clear(Option_t* opt){
   fSortInfoList.clear();
}

Long64_t TGRSISortList::Merge(TCollection *list){
   return 0;

//  fRunInfoList.AddAll(list);
}

TGRSISortInfo::TGRSISortInfo(const TGRSIRunInfo* info){
   Clear();
   SetRunInfo(info);
}

TGRSISortInfo::TGRSISortInfo(){
   Clear();
}

TGRSISortInfo::~TGRSISortInfo(){}

void TGRSISortInfo::SetRunInfo(const TGRSIRunInfo *info){
   fRunNumber = info->RunNumber();
   fSubRunNumber = info->SubRunNumber();
}

void TGRSISortInfo::Print(Option_t *opt) const{
   std::cout << "Run Number:\t " << RunNumber();
   std::cout << "\tSub Run:\t " << SubRunNumber();
   if(fDuplicates)
      std::cout << "\tDuplicates:\t" << fDuplicates;
   std::cout << std::endl;
}

void TGRSISortInfo::Clear(Option_t *opt){
   fRunNumber     = 0;
   fSubRunNumber  = 0;
   fDuplicates = 0;
}

