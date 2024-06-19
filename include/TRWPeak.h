#ifndef TRWPEAK_H
#define TRWPEAK_H

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
#include "TGraph.h"

#include "TGRSIFunctions.h"
#include "TGRSIFit.h"
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
   ~TRWPeak() override = default;

   void InitParNames() override;
   void InitializeParameters(TH1* hist, const double& rangeLow, const double& rangeHigh) override;

   void Centroid(const Double_t& centroid) override;

   Double_t Centroid() const override;
   Double_t CentroidErr() const override;
   Double_t Width() const override { return GetFitFunction()->GetParameter("sigma"); }
   Double_t Sigma() const override { return GetFitFunction()->GetParameter("sigma"); }

protected:
   Double_t PeakFunction(Double_t* dim, Double_t* par) override;
   Double_t BackgroundFunction(Double_t* dim, Double_t* par) override;

public:
   /// \cond CLASSIMP
   ClassDefOverride(TRWPeak, 2);
   /// \endcond
};
/*! @} */
#endif
