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
   ~TABPeak() override {};
   TABPeak();
   TABPeak(Double_t centroid);

   void InitParNames() override;
   void InitializeParameters(TH1* hist) override;

   Double_t Centroid() const override;
   Double_t CentroidErr() const override;

   void Print(Option_t *opt = "") const override;

protected:
   Double_t PeakFunction(Double_t *dim, Double_t *par) override;
   Double_t BackgroundFunction(Double_t *dim, Double_t *par) override;

public:
   /// \cond CLASSIMP
   ClassDefOverride(TABPeak, 1);
   /// \endcond
};
/*! @} */
#endif
