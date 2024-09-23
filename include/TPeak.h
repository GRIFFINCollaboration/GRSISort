#ifndef TPEAK_H
#define TPEAK_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <string>
#include <algorithm>

#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TGraph.h"

#include "TGRSIFunctions.h"
#include "TGRSIFit.h"

/////////////////////////////////////////////////////////////////
///
/// \class TPeak
///
/// This Class is used to represent fitted data that is
/// Gaussian like in nature (ie centroid and area).
///
/////////////////////////////////////////////////////////////////

class TPeak : public TGRSIFit {
   friend class TMultiPeak;

public:
   // ctors and dtors
   TPeak(Double_t cent, Double_t xlow, Double_t xhigh, TF1* background = nullptr);
   TPeak();   // I might make it so if you call this ctor, the TPeak yells at you since it's a fairly useless call anyway
   TPeak(const TPeak& copy);
   TPeak(TPeak&&) noexcept            = default;
   TPeak& operator=(const TPeak&)     = default;
   TPeak& operator=(TPeak&&) noexcept = default;
   ~TPeak();

protected:
   void InitNames();

public:
   void Copy(TObject& obj) const override;
   void SetCentroid(Double_t cent) { SetParameter("centroid", cent); }

   Bool_t Fit(TH1* fitHist, Option_t* opt = "");   // Might switch this to TFitResultPtr
   // Bool_t Fit(TH1* fithist = 0);

   Double_t GetCentroid() const { return GetParameter("centroid"); }
   Double_t GetCentroidErr() const { return GetParError(GetParNumber("centroid")); }
   Double_t GetArea() const { return fArea; }
   Double_t GetAreaErr() const { return fDArea; }
   Double_t GetFWHM() const { return GetParameter("sigma") * 2.3548; }
   Double_t GetFWHMErr() const { return GetParError(GetParNumber("sigma")) * 2.3548; }
   Double_t GetIntegralArea();
   Double_t GetIntegralArea(Double_t int_low, Double_t int_high);
   Double_t GetIntegralAreaErr();
   Double_t GetIntegralAreaErr(Double_t int_low, Double_t int_high);
   /*
      Double_t Fit(Option_t* opt = "");
      Double_t Fit(TH1* hist, Option_t* opt = "");
      Double_t Fit(const char* histname, Option_t* opt);
   */
   const TF1* GetFitFunction() const
   {
      return static_cast<const TF1*>(this);
   }   // I might move the fit functions to TGRSIFit, it's just a little tricky to initilize the function

   Double_t   Centroid() const { return GetCentroid(); }
   Double_t   CentroidErr() const { return GetCentroidErr(); }
   Double_t   Area() const { return GetArea(); }
   Double_t   AreaErr() const { return GetAreaErr(); }
   Double_t   FWHM() const { return GetFWHM(); }
   Double_t   FWHMErr() const { return GetFWHMErr(); }
   Double_t   IntegralArea() { return GetIntegralArea(); }
   Double_t   IntegralArea(Double_t int_low, Double_t int_high) { return GetIntegralArea(int_low, int_high); }
   Double_t   IntegralAreaErr() { return GetIntegralAreaErr(); }
   Double_t   IntegralAreaErr(Double_t int_low, Double_t int_high) { return GetIntegralAreaErr(int_low, int_high); }
   const TF1* FitFunction() const { return GetFitFunction(); }

protected:
   void SetArea(Double_t area) { fArea = area; }
   void SetAreaErr(Double_t areaErr) { fDArea = areaErr; }
   void SetArea(Double_t area, Double_t areaErr)
   {
      SetArea(area);
      SetAreaErr(areaErr);
   }
   void SetChi2(Double_t chi2) { fChi2 = chi2; }
   void SetNdf(Double_t Ndf) { fNdf = Ndf; }

public:
   Bool_t InitParams(TH1* fitHist = nullptr) override;
   TF1*   Background() const { return fBackground; }
   void   DrawBackground(Option_t* opt = "SAME") const;   // *MENU*
   void   DrawResiduals();                                // *MENU*
   void   CheckArea();
   void   CheckArea(Double_t int_low, Double_t int_high);

   static void   SetLogLikelihoodFlag(Bool_t flag = true) { fLogLikelihoodFlag = flag; }
   static Bool_t GetLogLikelihoodFlag() { return fLogLikelihoodFlag; }

   static Bool_t CompareEnergy(const TPeak& lhs, const TPeak& rhs) { return lhs.GetCentroid() < rhs.GetCentroid(); }
   static Bool_t CompareArea(const TPeak& lhs, const TPeak& rhs) { return lhs.GetArea() < rhs.GetArea(); }
   static Bool_t CompareEnergy(const TPeak* lhs, const TPeak* rhs) { return lhs->GetCentroid() < rhs->GetCentroid(); }
   static Bool_t CompareArea(const TPeak* lhs, const TPeak* rhs) { return lhs->GetArea() < rhs->GetArea(); }

   static TPeak* GetLastFit() { return fLastFit; }

   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

private:
   bool GoodStatus();
   // Centroid will eventually be read from parameters
   Double_t fArea{0.};
   Double_t fDArea{0.};
   Double_t fChi2{0.};
   Double_t fNdf{0.};
   Bool_t   fOwnBgFlag{false};

   static bool   fLogLikelihoodFlag;   //!<!
   static TPeak* fLastFit;             //!<!

   TF1*    fBackground{nullptr};
   TGraph* fResiduals{nullptr};

   /// \cond CLASSIMP
   ClassDefOverride(TPeak, 2)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif
