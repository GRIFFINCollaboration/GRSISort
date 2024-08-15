#ifndef TCALGRAPH_H
#define TCALGRAPH_H

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
	TCalGraph(const TCalGraph&);
	TCalGraph(TCalGraph&&) noexcept = default;
	TCalGraph& operator=(const TCalGraph&) = default;
	TCalGraph& operator=(TCalGraph&&) noexcept = default;
   ~TCalGraph() = default;

#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
   using TGraph::AddPoint;
#endif
   void     AddPoint(const TCalPoint& cal_point);
   Int_t    FindClosestPointX(const Double_t& x_val);
   Double_t FindDistToClosestPointX(const Double_t& x_val);

   Int_t AddLists(const TCalList& cal_list, const TSourceList& src_list);
   void  Draw(Option_t* opt = "") override
   {
      BuildGraph();
      TGraphErrors::Draw(opt);
   }
   void ClearAllPoints(Option_t* opt = "");

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

protected:
	std::map<UInt_t, std::pair<TCalPoint, TCalPoint>>& CompareMap() { return fCompareMap; }

private:
   void CorrectMissingPoints(TCalList& cal_list, TCalList& src_list);

   virtual void BuildGraph() = 0;

   std::map<UInt_t, std::pair<TCalPoint, TCalPoint>> fCompareMap;

   /// \cond CLASSIMP
   ClassDefOverride(TCalGraph, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
