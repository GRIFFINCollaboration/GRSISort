#ifndef TEFFICIENCYCAL_H
#define TEFFICIENCYCAL_H

/** \addtogroup Calibration
 *  @{
 */

#include "TCal.h"
#include "TPeak.h"
#include "TSpectrum.h"

class TEfficiencyCal : public TCal {
public:
   TEfficiencyCal() = default;
   TEfficiencyCal(const char* name, const char* title) : TCal(name, title) {}
   TEfficiencyCal(const TEfficiencyCal&);
   TEfficiencyCal(TEfficiencyCal&&) noexcept = default;
   TEfficiencyCal& operator=(const TEfficiencyCal&) = default;
   TEfficiencyCal& operator=(TEfficiencyCal&&) noexcept = default;
   ~TEfficiencyCal() = default;

   void Copy(TObject& obj) const override;
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
   using TGraph::AddPoint;
#endif
   void AddPoint(Double_t energy, Double_t area, Double_t dEnergy = 0.0, Double_t dArea = 0.0);
   void AddPoint(TPeak* peak);

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   Bool_t IsGroupable() const override { return true; }

   void ScaleGraph(Double_t scaleFactor);

private:
   Double_t fScaleFactor{1.};

   /// \cond CLASSIMP
   ClassDefOverride(TEfficiencyCal, 1) // NOLINT
   /// \endcond
};
/*! @} */
#endif
