#ifndef TCALGRAPH_H__
#define TCALGRAPH_H__

/** \addtogroup Calibration
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TCalGraph
///
/// This is a class that contains the basic info
/// about a calibration. Calibrations here are TGraphErrors
/// that are fit, with the resulting fit function being the
/// calibrating function.
///
/////////////////////////////////////////////////////////////////

#include <map>
#include <vector>
#include <utility>

#include "TGraphErrors.h"
#include "TCalPoint.h"
#include "TCalList.h"
#include "TSourceList.h"

class TCalGraph : public TGraphErrors {
public:
   TCalGraph();
   TCalGraph(const char* name, const char* title) : TGraphErrors(name, title){};
   ~TCalGraph() override;

   TCalGraph(const TCalGraph& copy);

   void AddPoint(const TCalPoint& cal_point);
   Int_t FindClosestPointX(const Double_t& x_val);
   Double_t FindDistToClosestPointX(const Double_t& x_val);

   Int_t AddLists(const TCalList& cal_list, const TSourceList& src_list);
   void Draw(Option_t* opt = "") override
   {
      BuildGraph();
      TGraphErrors::Draw(opt);
   }
   void ClearAllPoints(Option_t* opt = "");

public:
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

protected:
   std::map<UInt_t, std::pair<TCalPoint, TCalPoint>> fCompareMap;

private:
   void CorrectMissingPoints(TCalList& cal_list, TCalList& src_list);

   virtual void BuildGraph() = 0;

   /// \cond CLASSIMP
   ClassDefOverride(TCalGraph, 1); // Graph Class for Calibrations
   /// \endcond
};
/*! @} */
#endif
