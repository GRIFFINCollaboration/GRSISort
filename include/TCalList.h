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
   virtual ~TCalList();

   TCalList(const TCalList& copy);

public:
   void AddPoint(const TCalPoint& pt);
   void AddPoint(const UInt_t& idx, const TCalPoint& pt);
   Int_t NPoints() const { return fCalList.size(); }
   void FillGraph(TGraph* graph) const;
   bool SetPointIndex(const UInt_t& old_idx, const UInt_t& new_idx);

   virtual void Copy(TObject& obj) const;
   virtual void Clear(Option_t* opt = "");
   virtual void Print(Option_t* opt = "") const;

   const std::map<UInt_t, TCalPoint>& Map() const { return fCalList; }

private:
   std::map<UInt_t, TCalPoint> fCalList;

   /// \cond CLASSIMP
   ClassDef(TCalList, 1);
   /// \endcond
};
/*! @} */
#endif
