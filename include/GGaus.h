#ifndef GGAUS_H
#define GGAUS_H

#include <TF1.h>

#include <string>
#include <algorithm>

class GGaus : public TF1 {
public:
   GGaus();
   GGaus(Double_t xlow, Double_t xhigh, Option_t* opt = "gsc");
   GGaus(Double_t xlow, Double_t xhigh, TF1* bg, Option_t* opt = "gsc");
   GGaus(const GGaus&);
   GGaus(GGaus&&) noexcept            = default;
   GGaus& operator=(const GGaus&)     = default;
   GGaus& operator=(GGaus&&) noexcept = default;
   ~GGaus()                           = default;

   void Copy(TObject&) const override;
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   void InitNames();
   bool InitParams(TH1* fithist = nullptr);
   bool Fit(TH1*, Option_t* opt = "");
   void DrawResiduals(TH1*) const;
   // void DrawResiduals(); // *MENU*

   TF1* Background(Option_t* = "TF1") { return &fBGFit; }
   // void DrawBackground(Option_t* opt = "SAME") const; // *MENU*

   Double_t GetCentroid() const { return GetParameter("centroid"); }
   Double_t GetCentroidErr() const { return GetParError(GetParNumber("centroid")); }
   Double_t GetArea() const { return fArea; }
   Double_t GetAreaErr() const { return fDArea; }
   Double_t GetSum() const { return fSum; }
   Double_t GetSumErr() const { return fDSum; }
   Double_t GetFWHM() const { return GetParameter("sigma") * 2.3548; }
   Double_t GetFWHMErr() const { return GetParError(GetParNumber("sigma")) * 2.3548; }
   // Double_t GetIntegralArea();
   // Double_t GetIntegralArea(Double_t int_low, Double_t int_high);
   // Double_t GetIntegralAreaErr();
   // Double_t GetIntegralAreaErr(Double_t int_low, Double_t int_high);

   static Bool_t CompareEnergy(const GGaus& lhs, const GGaus& rhs) { return lhs.GetCentroid() < rhs.GetCentroid(); }
   static Bool_t CompareArea(const GGaus& lhs, const GGaus& rhs) { return lhs.GetArea() < rhs.GetArea(); }

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

private:
   double fArea{0.};
   double fDArea{0.};
   double fChi2{0.};
   double fNdf{0.};

   double fSum{0.};
   double fDSum{0.};

   Bool_t IsInitialized() const { return fInitFlag; }
   void   SetInitialized(Bool_t flag = true) { fInitFlag = flag; }
   bool   fInitFlag{false};

   TF1 fBGFit;
   TF1 fBGHist;

   /// /cond CLASSIMP
   ClassDefOverride(GGaus, 2)   // NOLINT(readability-else-after-return)
                                /// /endcond
};

#endif
