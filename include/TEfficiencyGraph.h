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
   TEfficiencyGraph(const char* name, const char* title) : TCalGraph(name,title) {};
   virtual ~TEfficiencyGraph(); 

   TEfficiencyGraph(const TEfficiencyGraph& copy);

 public:
   virtual void Print(Option_t* opt = "") const;
   virtual void Clear(Option_t* opt = "");

	void Scale(const Double_t &scale);

 protected:
	void BuildGraph();

/// \cond CLASSIMP
   ClassDef(TEfficiencyGraph,1); //Graph Class for Calibrations
/// \endcond
};
/*! @} */
#endif
