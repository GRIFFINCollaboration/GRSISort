#ifndef TPEAKFITTER_H
#define TPEAKFITTER_H

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
#include "TSinglePeak.h"

/////////////////////////////////////////////////////////////////
///
/// \class TPeakFitter
///
///  This class is used to fit things that resemble "peaks" in data
///
/////////////////////////////////////////////////////////////////
class TSinglePeak;
using MultiplePeak_t = std::list<TSinglePeak*>;

class TPeakFitter : public TObject {
public:
   // ctors and dtors
   TPeakFitter() : TPeakFitter(0., 0.) {}
   TPeakFitter(const Double_t& rangeLow, const Double_t& rangeHigh);
   TPeakFitter(const TPeakFitter&)                = default;
   TPeakFitter(TPeakFitter&&) noexcept            = default;
   TPeakFitter& operator=(const TPeakFitter&)     = default;
   TPeakFitter& operator=(TPeakFitter&&) noexcept = default;
   ~TPeakFitter()                                 = default;

   void AddPeak(TSinglePeak* peak)
   {
      fPeaksToFit.push_back(peak);
      ResetTotalFitFunction();
   }
   void RemovePeak(TSinglePeak* peak)
   {
      fPeaksToFit.remove(peak);
      ResetTotalFitFunction();
   }
   void RemoveAllPeaks()
   {
      fPeaksToFit.clear();
      ResetTotalFitFunction();
   }
   //   void SetPeakToFit(TMultiplePeak*  peaks_to_fit) { fPeaksToFit = peaks_to_fit; }
   void SetBackground(TF1* bg_to_fit) { fBGToFit = bg_to_fit; }
   void InitializeParameters(TH1* fit_hist);
   void InitializeBackgroundParameters(TH1* fit_hist);

   void Print(Option_t* opt = "") const override;
   void PrintParameters() const;

   TF1*          GetBackground() { return fBGToFit; }
   TF1*          GetFitFunction() { return fTotalFitFunction; }
   void          SetRange(const Double_t& low, const Double_t& high);
   Int_t         GetNParameters() const;
   TFitResultPtr Fit(TH1* fit_hist, Option_t* opt = "");
   void          DrawPeaks(Option_t* = "") const;

   void ResetInitFlag() { fInitFlag = false; }

   void SetColorIndex(const int& index) { fColorIndex = index; }

private:
   void     UpdateFitterParameters();
   void     UpdatePeakParameters(const TFitResultPtr& fit_res, TH1* fit_hist);
   Double_t DefaultBackgroundFunction(Double_t* dim, Double_t* par);
   void     ResetTotalFitFunction()
   {
      if(fTotalFitFunction != nullptr) {
         delete fTotalFitFunction;
         fTotalFitFunction = nullptr;
      }
   }

   MultiplePeak_t fPeaksToFit;
   TF1*           fBGToFit{nullptr};

   TF1* fTotalFitFunction{nullptr};

   Double_t fRangeLow{0.};
   Double_t fRangeHigh{0.};

   Double_t FitFunction(Double_t* dim, Double_t* par);
   Double_t BackgroundFunction(Double_t* dim, Double_t* par);

   bool fInitFlag{false};

   TH1* fLastHistFit{nullptr};

   int fColorIndex{0};   ///< this index is added to the colors kRed for the total function and kMagenta for the individual peaks

   /// \cond CLASSIMP
   ClassDefOverride(TPeakFitter, 2)   // NOLINT
   /// \endcond
};
/*! @} */
#endif
