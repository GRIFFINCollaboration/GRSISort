#ifndef TEFFICIENCYCALIBRATION_H__
#define TEFFICIENCYCALIBRATION_H__

/** \addtogroup Calibration
 *  @{
 */

#include <map>

#include "TNamed.h"
#include "TMultiGraph.h"
#include "TF1.h"

#include "TEfficiencyGraph.h"

class TEfficiencyCalibration : public TNamed {
 public: 
   TEfficiencyCalibration();
   TEfficiencyCalibration(const char* name, const char* title);
   virtual ~TEfficiencyCalibration(); 

   TEfficiencyCalibration(const TEfficiencyCalibration &copy);

 public:
	void AddEfficiencyGraph(const TEfficiencyGraph & graph);
	void AddEfficiencyGraph(const TEfficiencyGraph & graph, const char* name);
	void ScaleGuess();

 public:
   void Copy(TObject &obj) const;

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;
	TFitResultPtr Fit(Option_t *opt="");

	void Draw(Option_t* opt="");

 private:
	void BuildMultiGraph();
	Double_t PhotoPeakEfficiency( Double_t *x, Double_t *par);

 private:
	std::map<const char*,TEfficiencyGraph> fGraphMap;
	TMultiGraph * fMultiGraph;
	mutable bool fFitting;

/// \cond CLASSIMP
   ClassDef(TEfficiencyCalibration,1);
/// \endcond

};
/*! @} */
#endif
