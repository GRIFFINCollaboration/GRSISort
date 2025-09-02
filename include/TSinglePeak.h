#ifndef TSINGLEPEAK_H
#define TSINGLEPEAK_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <vector>
#include <cstdarg>

#include "TObject.h"
#include "TF1.h"
#include "TGraph.h"
#include "TPeakFitter.h"

/////////////////////////////////////////////////////////////////
///
/// \class TSinglePeak
///
/// This is the base class for anything that is used to fit
/// things that resemble "peaks" in data.
/// Any derived classes must implement functions to
/// - initialize parameters and parameter names,
/// - set the total function and peak function,
/// The centroid of the peak should always be parameter 1.
///
/////////////////////////////////////////////////////////////////
class TPeakFitter;

class TSinglePeak : public TObject {
public:
   friend class TPeakFitter;
   // ctors and dtors
   TSinglePeak()                                  = default;
   TSinglePeak(const TSinglePeak&)                = default;
   TSinglePeak(TSinglePeak&&) noexcept            = default;
   TSinglePeak& operator=(const TSinglePeak&)     = default;
   TSinglePeak& operator=(TSinglePeak&&) noexcept = default;
   ~TSinglePeak()                                 = default;

   virtual void InitParNames() {}
   virtual void InitializeParameters(TH1*, const double&, const double&) {}
   bool         IsBackgroundParameter(const Int_t& par) const;
   bool         IsPeakParameter(const Int_t& par) const;
   void         SetListOfBGPar(const std::vector<bool>& list_of_bg_par) { fListOfBGPars = list_of_bg_par; }
   Int_t        GetNParameters() const;

   void SetArea(const Double_t& area) { fArea = area; }
   void SetAreaErr(const Double_t& area_err) { fAreaErr = area_err; }

   Double_t Area() const { return fArea; }
   Double_t AreaErr() const { return fAreaErr; }

   virtual void     Centroid(const Double_t& centroid) = 0;
   virtual Double_t Centroid() const                   = 0;
   virtual Double_t CentroidErr() const                = 0;
   virtual Double_t Width() const                      = 0;
   virtual Double_t Sigma() const                      = 0;
   virtual Double_t SigmaErr() const                   = 0;
   virtual Double_t FWHM();      // not constant because we have to update the parameters of the peak function
   virtual Double_t FWHMErr();   // not constant because we have to update the parameters of the peak function

   void         Print(Option_t* = "") const override;
   void         Draw(Option_t* opt = "") override;
   virtual void DrawBackground(Option_t* opt = "")
   {
      if(fGlobalBackground != nullptr) { fGlobalBackground->Draw(opt); }
   }
   virtual void DrawComponents(Option_t* opt = "");
   virtual void PrintParameters() const;

   TF1* GetFitFunction() const { return fTotalFunction; }
   TF1* GetPeakFunction() const { return fPeakFunction; }
   TF1* GetGlobalBackground() const { return fGlobalBackground; }
   TF1* GetBackgroundFunction();
   void SetGlobalBackground(TF1* background)
   {
      fGlobalBackground = background;
      fGlobalBackground->SetLineStyle(kDashed);
   }

   void UpdatePeakParameters();
   void UpdateBackgroundParameters();

   Double_t GetChi2() const { return fChi2; }
   Double_t GetNDF() const { return fNDF; }
   Double_t GetReducedChi2() const { return fChi2 / fNDF; }

   bool ParameterSetByUser(int par);

   void SetLineColor(Color_t color) { fTotalFunction->SetLineColor(color); }
   void SetLineStyle(Style_t style) { fTotalFunction->SetLineStyle(style); }

   Color_t GetLineColor() { return fTotalFunction->GetLineColor(); }
   Style_t GetLineStyle() { return fTotalFunction->GetLineStyle(); }

protected:
   Double_t         TotalFunction(Double_t* dim, Double_t* par);
   virtual Double_t BackgroundFunction(Double_t*, Double_t*) { return 0.0; }
   virtual Double_t PeakFunction(Double_t*, Double_t*) { return 0.0; }
   virtual Double_t PeakOnGlobalFunction(Double_t* dim, Double_t* par);

   void SetChi2(const Double_t& chi2) { fChi2 = chi2; }
   void SetNDF(const Int_t& ndf) { fNDF = ndf; }

   void SetFitFunction(TF1* function) { fTotalFunction = function; }
   void SetPeakFunction(TF1* function) { fPeakFunction = function; }

private:
   TF1* fTotalFunction{nullptr};
   TF1* fBackgroundFunction{nullptr};
   TF1* fGlobalBackground{nullptr};
   TF1* fPeakOnGlobal{nullptr};
   TF1* fPeakFunction{nullptr};

   std::vector<bool> fListOfBGPars;
   Double_t          fArea{-0.1};
   Double_t          fAreaErr{0.0};
   Double_t          fChi2{std::numeric_limits<Double_t>::quiet_NaN()};
   Int_t             fNDF{0};

public:
   /// \cond CLASSIMP
   ClassDefOverride(TSinglePeak, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
