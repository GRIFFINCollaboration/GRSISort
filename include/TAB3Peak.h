#ifndef TAB3PEAK_H
#define TAB3PEAK_H

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
/// \class TAB3Peak
///
///  This class is used to fit Addback peaks in data
///
/////////////////////////////////////////////////////////////////

class TAB3Peak : public TSinglePeak {
public:
   // ctors and dtors
   TAB3Peak() = default;
   explicit TAB3Peak(Double_t centroid) { Centroid(centroid); }
   ~TAB3Peak() = default;

   void InitParNames() override;
   void InitializeParameters(TH1* hist, const double& rangeLow, const double& rangeHigh) override;

   void Centroid(const Double_t& centroid) override;

   Double_t Centroid() const override;
   Double_t CentroidErr() const override;
   Double_t Width() const override;
   Double_t Sigma() const override;

   void DrawComponents(Option_t* opt = "") override;

protected:
   Double_t PeakFunction(Double_t* dim, Double_t* par) override;
   Double_t BackgroundFunction(Double_t* dim, Double_t* par) override;

private:
   static Double_t OneHitPeakFunction(Double_t* dim, Double_t* par);
   static Double_t TwoHitPeakFunction(Double_t* dim, Double_t* par);
   static Double_t ThreeHitPeakFunction(Double_t* dim, Double_t* par);
   Double_t        OneHitPeakOnGlobalFunction(Double_t* dim, Double_t* par);
   Double_t        TwoHitPeakOnGlobalFunction(Double_t* dim, Double_t* par);
   Double_t        ThreeHitPeakOnGlobalFunction(Double_t* dim, Double_t* par);

   TF1* fOneHitOnGlobal{nullptr};
   TF1* fTwoHitOnGlobal{nullptr};
   TF1* fThreeHitOnGlobal{nullptr};

public:
   /// \cond CLASSIMP
   ClassDefOverride(TAB3Peak, 2);
   /// \endcond
};
/*! @} */
#endif
