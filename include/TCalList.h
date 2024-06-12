#ifndef TCALLIST_H__
#define TCALLIST_H__

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
   TCalList(const char* name, const char* title = "");
   ~TCalList() override;

   TCalList(const TCalList& copy);

public:
   void  AddPoint(const TCalPoint& point);
   void  AddPoint(const UInt_t& idx, const TCalPoint& point);
   Int_t NPoints() const { return fCalList.size(); }
   void  FillGraph(TGraph* graph) const;
   bool  SetPointIndex(const UInt_t& old_idx, const UInt_t& new_idx);

   void Copy(TObject& obj) const override;
   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   const std::map<UInt_t, TCalPoint>& Map() const { return fCalList; }

private:
   std::map<UInt_t, TCalPoint> fCalList;

   /// \cond CLASSIMP
   ClassDefOverride(TCalList, 1);
   /// \endcond
};
/*! @} */
#endif
