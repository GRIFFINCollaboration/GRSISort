#ifndef TEFFICIENCYGRAPH_H__
#define TEFFICIENCYGRAPH_H__

/** \addtogroup Calibration
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TEfficiencyGraph
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

#include "TCalGraph.h"
#include "TCalPoint.h"
#include "TCalList.h"
#include "TSourceList.h"

#include "TMath.h"

class TEfficiencyGraph : public TCalGraph {
public:
   TEfficiencyGraph();
   TEfficiencyGraph(const char* name, const char* title) : TCalGraph(name, title), fIsAbsolute(false) {};
   ~TEfficiencyGraph() override;

   TEfficiencyGraph(const TEfficiencyGraph& copy);

	TEfficiencyGraph& operator=(const TEfficiencyGraph&) = default; // use default to stop gcc 9.1 warning

public:
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   void Scale(const Double_t& scale);
   void SetAbsolute(const bool& flag) { fIsAbsolute = flag; }
   bool                         IsAbsolute() const { return fIsAbsolute; }

protected:
   void BuildGraph() override;

private:
   bool fIsAbsolute;

   /// \cond CLASSIMP
   ClassDefOverride(TEfficiencyGraph, 1); // Graph Class for Calibrations
   /// \endcond
};
/*! @} */
#endif
