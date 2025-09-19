#ifndef TRWPEAK_H
#define TRWPEAK_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <cstdarg>

#include "TF1.h"
#include "TGraph.h"

#include "TSinglePeak.h"

/////////////////////////////////////////////////////////////////
///
/// \class TRWPeak
///
///  This class is used to fit RadWare like peaks in data
///
/////////////////////////////////////////////////////////////////

class TRWPeak : public TSinglePeak {
public:
   // ctors and dtors
   TRWPeak() = default;
   explicit TRWPeak(Double_t centroid) { Centroid(centroid); }
   TRWPeak(const TRWPeak&)                = default;
   TRWPeak(TRWPeak&&) noexcept            = default;
   TRWPeak& operator=(const TRWPeak&)     = default;
   TRWPeak& operator=(TRWPeak&&) noexcept = default;
   ~TRWPeak()                             = default;

   void InitParNames() override;
   void InitializeParameters(TH1* hist, const double& rangeLow, const double& rangeHigh) override;

   void Centroid(const Double_t& centroid) override;

   Double_t Centroid() const override;
   Double_t CentroidErr() const override;
   Double_t Width() const override { return GetFitFunction()->GetParameter("sigma"); }
   Double_t Sigma() const override { return GetFitFunction()->GetParameter("sigma"); }
   Double_t SigmaErr() const override { return GetFitFunction()->GetParError(GetFitFunction()->GetParNumber("sigma")); }

protected:
   Double_t PeakFunction(Double_t* dim, Double_t* par) override;
   Double_t BackgroundFunction(Double_t* dim, Double_t* par) override;

public:
   /// \cond CLASSIMP
   ClassDefOverride(TRWPeak, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
