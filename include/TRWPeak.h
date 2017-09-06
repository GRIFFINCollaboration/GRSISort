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
   ~TRWPeak() override {};
   TRWPeak();
   TRWPeak(Double_t centroid);

   void InitParNames() override;
   void InitializeParameters(TH1* hist) override;

   Double_t Centroid() const override;
   Double_t CentroidErr() const override;

   void Print(Option_t *opt = "") const override;

protected:
   Double_t FitFunction(Double_t *dim, Double_t *par) override;
   

public:
   /// \cond CLASSIMP
   ClassDefOverride(TRWPeak, 1);
   /// \endcond
};
/*! @} */
#endif
