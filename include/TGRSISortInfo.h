#ifndef TGRSISORTINFO_H
#define TGRSISORTINFO_H

/** \addtogroup Sorting
 *  @{
 */

#include <map>

#include "TList.h"
#include "TString.h"
#include "TCollection.h"
#include "Globals.h"
#include "TGRSIRunInfo.h"

class TGRSISortInfo;

class TGRSISortList : public TObject {
   typedef std::map<Int_t, std::map<Int_t, TGRSISortInfo*>> info_map;

public:
   TGRSISortList(){};
   ~TGRSISortList() override= default;;

   Bool_t AddSortInfo(TGRSISortInfo* info, Option_t* opt = "");
   Bool_t AddSortList(TGRSISortList* rhsList, Option_t* opt = "");
   TGRSISortInfo* GetSortInfo(Int_t RunNumber, Int_t SubRunNumber);
   Long64_t Merge(TCollection* list);

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   info_map* GetMap() { return &fSortInfoList; };

private:
   info_map fSortInfoList;

   /// \cond CLASSIMP
   ClassDefOverride(TGRSISortList, 1);
   /// \endcond
};

class TGRSISortInfo : public TObject {
public:
   TGRSISortInfo();
   TGRSISortInfo(const TGRSIRunInfo* info);
   ~TGRSISortInfo() override;

public:
   void SetRunInfo(const TGRSIRunInfo* info);
   Int_t   RunNumber() const { return fRunNumber; }
   Int_t   SubRunNumber() const { return fSubRunNumber; }
   TString Comment() const { return fComment; }

   void SetComment(const char* comment) { fComment = comment; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   Int_t AddDuplicate() { return ++fDuplicates; }

private:
   Int_t   fRunNumber{};
   Int_t   fSubRunNumber{};
   UInt_t  fDuplicates{};
   TString fComment;

   /// \cond CLASSIMP
   ClassDefOverride(TGRSISortInfo, 1);
   /// \endcond
};
/*! @} */
#endif
