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
   ~TEfficiencyCalibration() override;

   TEfficiencyCalibration(const TEfficiencyCalibration& copy);

public:
   void AddEfficiencyGraph(const TEfficiencyGraph& graph);
   void AddEfficiencyGraph(const TEfficiencyGraph& graph, const char* name);
   void ScaleGuess();

public:
   void Copy(TObject& obj) const override;

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;
   TFitResultPtr Fit(Option_t* opt = "");

   void Draw(Option_t* opt = "") override;
   void DrawRelative(Option_t* opt = "");
   void DrawAbsolute(Option_t* opt = "");

   Double_t GetEfficiency(const Double_t& eng);
   Double_t GetEfficiencyErr(const Double_t& eng);

private:
   void     BuildMultiGraph();
   Double_t PhotoPeakEfficiency(Double_t* x, Double_t* par);
   Double_t AbsoluteEfficiency(Double_t* x, Double_t* par);

public:
   bool ScaleToAbsolute();

private:
   std::map<const char*, TEfficiencyGraph> fGraphMap;
   TMultiGraph* fRelativeEffGraph;
   TMultiGraph* fAbsEffGraph;
   mutable bool fFitting;
   TF1*         fRelativeFit;
   TF1*         fAbsoluteFunc;

   /// \cond CLASSIMP
   ClassDefOverride(TEfficiencyCalibration, 1);
   /// \endcond
};
/*! @} */
#endif
