#ifndef TGAUSS_H
#define TGAUSS_H

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
/// \class TGauss
///
///  This class is used to fit simple gaussian peaks in data
///
/////////////////////////////////////////////////////////////////

class TGauss : public TSinglePeak {
public:
   // ctors and dtors
   ~TGauss() override {};
   TGauss();
   TGauss(Double_t centroid, Double_t relativeLimit = -1.);

   void InitParNames() override;
   void InitializeParameters(TH1* hist) override;

   Double_t Centroid() const override;
   Double_t CentroidErr() const override;
   Double_t Width() const override { return fTotalFunction->GetParameter("sigma"); }

   void Print(Option_t *opt = "") const override;

protected:
   Double_t PeakFunction(Double_t *dim, Double_t *par) override;

public:
   /// \cond CLASSIMP
   ClassDefOverride(TGauss, 1);
   /// \endcond
};
/*! @} */
#endif
