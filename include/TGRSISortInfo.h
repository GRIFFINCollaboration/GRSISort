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
#include "TRunInfo.h"

class TGRSISortInfo;

class TGRSISortList : public TObject {
   using infoMap = std::map<Int_t, std::map<Int_t, TGRSISortInfo*>>;

public:
   TGRSISortList()           = default;
   ~TGRSISortList() override = default;

   Bool_t         AddSortInfo(TGRSISortInfo* info, Option_t* opt = "");
   Bool_t         AddSortList(TGRSISortList* rhslist, Option_t* opt = "");
   TGRSISortInfo* GetSortInfo(Int_t RunNumber, Int_t SubRunNumber);
   Long64_t       Merge(TCollection* list);

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   infoMap fSortInfoList;

   /// \cond CLASSIMP
   ClassDefOverride(TGRSISortList, 2) // NOLINT
   /// \endcond
};

class TGRSISortInfo : public TObject {
public:
   TGRSISortInfo();
   ~TGRSISortInfo() override;

   void    SetRunInfo();
   Int_t   RunNumber() const { return fRunNumber; }
   Int_t   SubRunNumber() const { return fSubRunNumber; }
   TString Comment() const { return fComment; }

   void SetComment(const char* comment) { fComment = comment; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   UInt_t AddDuplicate() { return ++fDuplicates; }

private:
   Int_t   fRunNumber{0};
   Int_t   fSubRunNumber{0};
   UInt_t  fDuplicates{0};
   TString fComment;

   /// \cond CLASSIMP
   ClassDefOverride(TGRSISortInfo, 2) // NOLINT
   /// \endcond
};
/*! @} */
#endif
