#ifndef TENERGYCAL_H
#define TENERGYCAL_H

/** \addtogroup Calibration
 *  @{
 */

#include "TCal.h"
#include "TPeak.h"
#include "TSpectrum.h"

////////////////////////////////////////////////////////////////
///
/// \class TEnergyCal
///
/// Class for performing energy calibrations using a single
/// nucleus.
///
////////////////////////////////////////////////////////////////

class TEnergyCal : public TCal {
public:
   TEnergyCal();
   TEnergyCal(const char* name, const char* title) : TCal(name, title) {}
   ~TEnergyCal() override = default;

   std::vector<Double_t> GetParameters() const override;
   Double_t              GetParameter(size_t parameter) const override;
   void                  WriteToChannel() const override;

#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 24, 0)
   using TGraph::AddPoint;
#endif
   void AddPoint(Double_t measured, Double_t accepted, Double_t measuredUncertainty = 0.0,
                 Double_t acceptedUncertainty = 0.0);
   using TGraphErrors::SetPoint;
   using TGraphErrors::SetPointError;
   Bool_t SetPoint(Int_t idx, Double_t measured);
   Bool_t SetPoint(Int_t idx, TPeak* peak);
   Bool_t SetPointError(Int_t idx, Double_t measuredUncertainty);

   void SetNucleus(TNucleus* nuc, Option_t* opt = "") override;

   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;
   void SetDefaultTitles();

   Bool_t IsGroupable() const override { return true; }

private:
   /// \cond CLASSIMP
   ClassDefOverride(TEnergyCal, 1);
   /// \endcond
};
/*! @} */
#endif
