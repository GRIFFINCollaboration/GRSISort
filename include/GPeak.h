#ifndef GPEAK_H
#define GPEAK_H

#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"

#include <string>
#include <algorithm>

class GPeak : public TF1 {
public:
   GPeak(Double_t cent, Double_t xlow, Double_t xhigh, Option_t* opt = "gsc");
   GPeak(Double_t cent, Double_t xlow, Double_t xhigh, TF1* bg, Option_t* opt = "gsc");
   GPeak(const GPeak&);
   GPeak();
   ~GPeak() override;

   void Copy(TObject&) const override;
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   void InitNames();
   bool InitParams(TH1* fithist = nullptr);
   bool Fit(TH1*, Option_t* opt = "");
   void DrawResiduals(TH1*) const;

   TF1* Background(Option_t* = "TF1") { return &fBGFit; }

   Double_t GetCentroid() const { return GetParameter("centroid"); }
   Double_t GetCentroidErr() const { return GetParError(GetParNumber("centroid")); }
   Double_t GetArea() const { return fArea; }
   Double_t GetAreaErr() const { return fDArea; }
   Double_t GetSum() const { return fSum; }
   Double_t GetSumErr() const { return fDSum; }
   Double_t GetFWHM() const { return GetParameter("sigma") * 2.3548; }
   Double_t GetFWHMErr() const { return GetParError(GetParNumber("sigma")) * 2.3548; }

   Double_t Centroid() const { return GetCentroid(); }
   Double_t CentroidErr() const { return GetCentroidErr(); }
   Double_t Area() const { return GetArea(); }
   Double_t AreaErr() const { return GetAreaErr(); }
   Double_t Sum() const { return GetSum(); }
   Double_t SumErr() const { return GetSumErr(); }
   Double_t FWHM() const { return GetFWHM(); }
   Double_t FWHMErr() const { return GetFWHMErr(); }

protected:
   void SetArea(Double_t a) { fArea = a; }
   void SetAreaErr(Double_t d_a) { fDArea = d_a; }
   void SetSum(Double_t a) { fSum = a; }
   void SetSumErr(Double_t d_a) { fDSum = d_a; }
   void SetArea(Double_t a, Double_t dA)
   {
      SetArea(a);
      SetAreaErr(dA);
   }
   void SetChi2(Double_t chi2) { fChi2 = chi2; }
   void SetNdf(Double_t Ndf) { fNdf = Ndf; }

public:
   static Bool_t CompareEnergy(const GPeak& lhs, const GPeak& rhs) { return lhs.GetCentroid() < rhs.GetCentroid(); }
   static Bool_t CompareArea(const GPeak& lhs, const GPeak& rhs) { return lhs.GetArea() < rhs.GetArea(); }

   static GPeak* GetLastFit() { return fLastFit; }

private:
   double fArea{0.};
   double fDArea{0.};
   double fSum{0.};
   double fDSum{0.};
   double fChi2{0.};
   double fNdf{0.};

   Bool_t IsInitialized() const { return init_flag; }
   void   SetInitialized(Bool_t flag = true) { init_flag = flag; }
   bool   init_flag{false};

   static GPeak* fLastFit;

   TF1 fBGFit;

   ClassDefOverride(GPeak, 3)
};

#endif
