#ifndef TABPEAK_H
#define TABPEAK_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <string>
#include <algorithm>
#include <vector>
#include <cstdarg>

#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#include "TSinglePeak.h"

/////////////////////////////////////////////////////////////////
///
/// \class TABPeak
///
///  This class is used to fit Addback peaks in data
///
/////////////////////////////////////////////////////////////////

class TABPeak : public TSinglePeak {
public:
   // ctors and dtors
   TABPeak() = default;
   explicit TABPeak(Double_t centroid) { Centroid(centroid); }
   TABPeak(const TABPeak&)                = default;
   TABPeak(TABPeak&&) noexcept            = default;
   TABPeak& operator=(const TABPeak&)     = default;
   TABPeak& operator=(TABPeak&&) noexcept = default;
   ~TABPeak()                             = default;

   void InitParNames() override;
   void InitializeParameters(TH1* hist, const double& rangeLow, const double& rangeHigh) override;

   void Centroid(const Double_t& centroid) override;

   Double_t Centroid() const override;
   Double_t CentroidErr() const override;
   Double_t Width() const override;
   Double_t Sigma() const override;
   Double_t SigmaErr() const override { return GetFitFunction()->GetParError(GetFitFunction()->GetParNumber("sigma")); }

   void DrawComponents(Option_t* opt = "") override;

protected:
   Double_t PeakFunction(Double_t* dim, Double_t* par) override;
   Double_t BackgroundFunction(Double_t* dim, Double_t* par) override;

private:
   static Double_t OneHitPeakFunction(Double_t* dim, Double_t* par);
   static Double_t TwoHitPeakFunction(Double_t* dim, Double_t* par);
   Double_t        OneHitPeakOnGlobalFunction(Double_t* dim, Double_t* par);
   Double_t        TwoHitPeakOnGlobalFunction(Double_t* dim, Double_t* par);

   TF1* fOneHitOnGlobal{nullptr};
   TF1* fTwoHitOnGlobal{nullptr};

public:
   /// \cond CLASSIMP
   ClassDefOverride(TABPeak, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
