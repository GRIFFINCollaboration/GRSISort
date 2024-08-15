#ifndef TEFFICIENCYCALIBRATION_H
#define TEFFICIENCYCALIBRATION_H

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
	TEfficiencyCalibration(const TEfficiencyCalibration&);
	TEfficiencyCalibration(TEfficiencyCalibration&&) noexcept = default;
	TEfficiencyCalibration& operator=(const TEfficiencyCalibration&) = default;
	TEfficiencyCalibration& operator=(TEfficiencyCalibration&&) noexcept = default;
   ~TEfficiencyCalibration();

   void AddEfficiencyGraph(const TEfficiencyGraph& graph);
   void AddEfficiencyGraph(const TEfficiencyGraph& graph, const char* name);
   void ScaleGuess();

   void Copy(TObject& copy) const override;

   void          Clear(Option_t* opt = "") override;
   void          Print(Option_t* opt = "") const override;
   TFitResultPtr Fit(Option_t* opt = "");

   void Draw(Option_t* opt = "") override;
   void DrawRelative(Option_t* opt = "");
   void DrawAbsolute(Option_t* opt = "");

   Double_t GetEfficiency(const Double_t& eng);
   Double_t GetEfficiencyErr(const Double_t& eng);

   bool ScaleToAbsolute();

private:
   void     BuildMultiGraph();
   Double_t PhotoPeakEfficiency(Double_t* x, Double_t* par);
   Double_t AbsoluteEfficiency(Double_t* x, Double_t* par);

   std::map<const char*, TEfficiencyGraph> fGraphMap;
   TMultiGraph*                            fRelativeEffGraph{nullptr};
   TMultiGraph*                            fAbsEffGraph{nullptr};
   mutable bool                            fFitting{false};
   TF1*                                    fRelativeFit{nullptr};
   TF1*                                    fAbsoluteFunc{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TEfficiencyCalibration, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
