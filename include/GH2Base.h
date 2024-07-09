#ifndef GH2BASE__H
#define GH2BASE__H

#include <cstdio>
#include <iostream>
#include <map>

#include <TNamed.h>
#include <TH2.h>
#include <TList.h>

class GH1D;

enum class EBackgroundSubtraction { kNoBackground,
                                    kRegionBackground,
                                    kMatchedLowerMarker,
                                    kSplitTwoMarker,
                                    kTotalFraction };

enum class EDirection { kXDirection,
                        kYDirection };

class GH2Base {

public:
   GH2Base() { Init(); }

   virtual ~GH2Base();

   virtual void GH2Clear(Option_t* opt = "");

   virtual TH2* GetTH2() = 0;

   GH1D* Projection_Background(int axis = 0, int firstbin = 0, int lastbin = -1, int first_bg_bin = 0,
                               int last_bg_bin = -1, EBackgroundSubtraction mode = EBackgroundSubtraction::kRegionBackground);

   // GH1D* SummaryProject(int binnum);

   GH1D* ProjectionX_Background(int firstbin = 0, int lastbin = -1, int first_bg_bin = 0, int last_bg_bin = -1,
                                EBackgroundSubtraction mode = EBackgroundSubtraction::kRegionBackground);   // *MENU*

   GH1D* GH2ProjectionX(const char* name = "_px", int firstbin = 0, int lastbin = -1, Option_t* option = "",
                        bool KeepEmpty = false);   // *MENU*

   GH1D* GH2ProjectionY(const char* name = "_py", int firstbin = 0, int lastbin = -1, Option_t* option = "",
                        bool KeepEmpty = false);   // *MENU*

   GH1D* ProjectionY_Background(int firstbin = 0, int lastbin = -1, int first_bg_bin = 0, int last_bg_bin = -1,
                                EBackgroundSubtraction mode = EBackgroundSubtraction::kRegionBackground);   // *MENU*

   GH1D* GetPrevious(const GH1D* curr, bool DrawEmpty = true);
   GH1D* GetPrevSummary(const GH1D* curr, bool DrawEmpty = false);
   GH1D* GetNext(const GH1D* curr, bool DrawEmpty = true);
   GH1D* GetNextSummary(const GH1D* curr, bool DrawEmpty = false);

   TList* GetProjections() { return fProjections; }
   TList* GetSummaryProjections() { return fSummaryProjections; }

   void SetSummary(bool is_summary = true) { fIsSummary = is_summary; }
   bool GetSummary() const { return fIsSummary; }

   void       SetSummaryDirection(EDirection dir) { fSummaryDirection = dir; }
   EDirection GetSummaryDirection() const { return fSummaryDirection; }

   class iterator {
   public:
      iterator(GH2Base* mat, bool at_end = false)
         : fMat(mat), fFirst(mat->GetNext(nullptr)), fCurr(at_end ? nullptr : fFirst)
      {
      }

      GH1D& operator*() const { return *fCurr; }
      GH1D* operator->() const { return fCurr; }

      // prefix increment
      iterator& operator++()
      {
         fCurr = fMat->GetNext(fCurr);
         if(fCurr == fFirst) {
            fCurr = nullptr;
         }
         return *this;
      }

      // postfix increment
      iterator operator++(int)
      {
         iterator current = *this;
         ++(*this);
         return current;
      }

      bool operator==(const iterator& rhs) const { return (fMat == rhs.fMat && fFirst == rhs.fFirst && fCurr == rhs.fCurr); }
      bool operator!=(const iterator& rhs) const { return !(*this == rhs); }

   private:
      GH2Base* fMat;
      GH1D*    fFirst;
      GH1D*    fCurr;
   };

   iterator begin() { return {this, false}; }
   iterator end() { return {this, true}; }

private:
   void   Init();
   TList* fProjections{nullptr};

   TList*     fSummaryProjections{nullptr};   //!
   bool       fIsSummary{false};
   EDirection fSummaryDirection{EDirection::kXDirection};

   ClassDef(GH2Base, 1);
};

#endif
