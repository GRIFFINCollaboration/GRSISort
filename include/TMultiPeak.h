#ifndef TMULTIPEAK_H
#define TMULTIPEAK_H

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
#include "TPeak.h"

/////////////////////////////////////////////////////////////////
///
/// \class TMultiPeak
///
/// This Class is used to represent fitted data that is
/// Gaussian like in nature (ie centroid and area).
///
/////////////////////////////////////////////////////////////////
class TPeak;

class TMultiPeak : public TGRSIFit {
public:
   // ctors and dtors
   ~TMultiPeak() override;
   // TMultiPeak(int n, ...);
   // TMultiPeak(double xlow, double xhigh, int n, ...);
   TMultiPeak(Double_t xlow, Double_t xhigh, const std::vector<Double_t>& centroids, Option_t* type = "gsc");
   TMultiPeak(const TMultiPeak& copy);
   TMultiPeak();   // I might make it so if you call this ctor, the TPeak yells at you since it's a fairly useless call
                   // anyway

protected:
   void InitNames();

public:
   Bool_t Fit(TH1* fithist, Option_t* opt = "");
   bool   InitParams(TH1* fithist) override;
   void   SortPeaks(Bool_t (*SortFunction)(const TPeak*, const TPeak*) = TPeak::CompareEnergy);
   TPeak* GetPeak(UInt_t idx);
   TPeak* GetPeakClosestTo(Double_t energy);
   void   DrawPeaks();
   TF1*   Background() const { return fBackground; }

   static void SetLogLikelihoodFlag(bool flag) { fLogLikelihoodFlag = flag; }
   static bool GetLogLikelihoodFlag() { return fLogLikelihoodFlag; }

   void Copy(TObject& obj) const override;
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   static bool         fLogLikelihoodFlag;   //!<!
   std::vector<TPeak*> fPeakVec;
   TF1*                fBackground;
   bool                fConstrainWidths;

   Double_t MultiPhotoPeakBG(Double_t* dim, Double_t* par);
   Double_t MultiStepBG(Double_t* dim, Double_t* par);
   Double_t SinglePeakBG(Double_t* dim, Double_t* par);

   /// \cond CLASSIMP
   ClassDefOverride(TMultiPeak, 2);
   /// \endcond
};
/*! @} */
#endif
