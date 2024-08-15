#ifndef TCALLIST_H
#define TCALLIST_H

/** \addtogroup Calibration
 *  @{
 */

#include "TNamed.h"
#include "TCalPoint.h"
#include "TGraphErrors.h"

#include <map>

class TCalList : public TNamed {
public:
   TCalList();
   explicit TCalList(const char* name, const char* title = "");
	TCalList(const TCalList&);
	TCalList(TCalList&&) noexcept = default;
	TCalList& operator=(const TCalList&) = default;
	TCalList& operator=(TCalList&&) noexcept = default;
   ~TCalList() = default;

   void  AddPoint(const TCalPoint& point);
   void  AddPoint(const UInt_t& idx, const TCalPoint& point);
   Int_t NPoints() const { return static_cast<Int_t>(fCalList.size()); }
   void  FillGraph(TGraph* graph) const;
   bool  SetPointIndex(const UInt_t& old_idx, const UInt_t& new_idx);

   void Copy(TObject& obj) const override;
   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   const std::map<UInt_t, TCalPoint>& Map() const { return fCalList; }

private:
   std::map<UInt_t, TCalPoint> fCalList;

   /// \cond CLASSIMP
   ClassDefOverride(TCalList, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
