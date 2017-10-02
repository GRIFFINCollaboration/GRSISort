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
typedef std::list<TSinglePeak*> MultiplePeak_t;

class TPeakFitter : public TObject {
public:
   // ctors and dtors
   ~TPeakFitter() override {};
   TPeakFitter();
   TPeakFitter(const Double_t& range_low, const Double_t& range_high);

public:
   void AddPeak(TSinglePeak *p)     { fPeaksToFit.push_back(p); }
   void RemovePeak(TSinglePeak *p)  { fPeaksToFit.remove(p); }
//   void SetPeakToFit(TMultiplePeak * peaks_to_fit) { fPeaksToFit = peaks_to_fit; }
   void SetBackground(TF1* bg_to_fit)                 { fBGToFit = bg_to_fit; }
   void InitializeParameters(TH1* fit_hist);
   void InitializeBackgroundParameters(TH1* fit_hist);

   virtual void Print(Option_t * opt = "") const override;

   TF1 * GetBackground() { return fBGToFit; }
   void SetRange(const Double_t &low, const Double_t &high);
   Int_t GetNParameters() const;
   void Fit(TH1* fit_hist,Option_t *opt="");
   void DrawPeaks(Option_t * = "") const;

private:
   void UpdateFitterParameters();
   void UpdatePeakParameters(TFitResultPtr fit_res,TH1* fit_hist);
   Double_t DefaultBackgroundFunction(Double_t *dim, Double_t *par);


private:
//   TMultiplePeak *fPeaksToFit{nullptr};
   MultiplePeak_t fPeaksToFit;
   TF1 *fBGToFit{nullptr};

   TF1* fTotalFitFunction;

   Double_t fRangeLow;
   Double_t fRangeHigh;
   
   Double_t FitFunction(Double_t *dim, Double_t *par);
   Double_t BackgroundFunction(Double_t *dim, Double_t *par);

   bool fInitFlag{false};

   /// \cond CLASSIMP
   ClassDefOverride(TPeakFitter, 1);
   /// \endcond
};
/*! @} */
#endif
