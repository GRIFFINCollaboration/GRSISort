#ifndef TGRSISORTINFO_H
#define TGRSISORTINFO_H

#include "TList.h"
#include "TString.h"
#include "TCollection.h"
#include "Globals.h"
#include "TGRSIRunInfo.h"

#include <map>


class TGRSISortList;
class TGRSISortInfo;

class TGRSISortList: public TObject {
   public:
      TGRSISortList(){};
      virtual ~TGRSISortList(){};

      Bool_t AddSortInfo(TGRSISortInfo *info, Option_t *opt = "");
      TGRSISortInfo* GetSortInfo(Int_t RunNumber, Int_t SubRunNumber);
      Long64_t Merge(TCollection *list);
      
      void Print(Option_t* opt = "") const;
      void Clear(Option_t* opt = "");

   private:
      std::map<Int_t,std::map<Int_t,TGRSISortInfo*>> fSortInfoList;

      ClassDef(TGRSISortList,1);
};


class TGRSISortInfo : public TObject {
   public:
      TGRSISortInfo();
      TGRSISortInfo(const TGRSIRunInfo* info);
      virtual ~TGRSISortInfo();

   public:
      void SetRunInfo(const TGRSIRunInfo *info);
      Int_t RunNumber() const { return fRunNumber; }
      Int_t SubRunNumber() const { return fSubRunNumber; }

      void Print(Option_t* opt = "") const;
      void Clear(Option_t* opt = "");

      Int_t AddDuplicate() { return ++fDuplicates; }

   private:
      Int_t fRunNumber;
      Int_t fSubRunNumber;
      UInt_t fDuplicates;

   ClassDef(TGRSISortInfo,1);
};

#endif
