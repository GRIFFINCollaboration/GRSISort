#ifndef GHSYM_H
#define GHSYM_H

#include "TH1.h"
#include "TH2.h"
#include "TArrayF.h"
#include "TArrayD.h"
#include "TProfile.h"
#include "TF1.h"
#include "TRandom.h"

class GHSym : public TH1 {
public:
   GHSym();
   GHSym(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
   GHSym(const char* name, const char* title, Int_t nbins, const Double_t* bins);
   GHSym(const char* name, const char* title, Int_t nbins, const Float_t* bins);
   GHSym(const GHSym&);
   GHSym(GHSym&&) noexcept;
   GHSym& operator=(const GHSym&);
   GHSym& operator=(GHSym&&) noexcept;

   ~GHSym() = default;

   Int_t         BufferEmpty(Int_t action = 0) override;
   Int_t         BufferFill(Double_t, Double_t) override { return -2; }   // MayNotUse
   virtual Int_t BufferFill(Double_t x, Double_t y, Double_t w);
   void          Copy(TObject& obj) const override;
   Int_t         Fill(Double_t) override;                                   // MayNotUse
   Int_t         Fill(const char*, Double_t) override { return Fill(0); }   // MayNotUse
   Int_t         Fill(Double_t x, Double_t y) override;
   virtual Int_t Fill(Double_t x, Double_t y, Double_t w);
   virtual Int_t Fill(const char* namex, const char* namey, Double_t w);
   void          FillN(Int_t, const Double_t*, const Double_t*, Int_t) override { ; }   // MayNotUse
   void          FillN(Int_t ntimes, const Double_t* x, const Double_t* y, const Double_t* w, Int_t stride = 1) override;
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 24, 0)
   void FillRandom(const char* fname, Int_t ntimes = 5000) override
   {
      FillRandom(fname, ntimes, nullptr);
   }
   void FillRandom(TH1* h, Int_t ntimes = 5000) override { FillRandom(h, ntimes, nullptr); }
   void FillRandom(const char* fname, Int_t ntimes = 5000, TRandom* rng = nullptr);
   void FillRandom(TH1* h, Int_t ntimes = 5000, TRandom* rng = nullptr);
#else
   void FillRandom(const char* fname, Int_t ntimes = 5000, TRandom* rng = nullptr) override;
   void FillRandom(TH1* h, Int_t ntimes = 5000, TRandom* rng = nullptr) override;
#endif
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 18, 0)
   Int_t FindFirstBinAbove(Double_t threshold = 0, Int_t axis = 1) const override
   {
      return FindFirstBinAbove(threshold, axis, 1, -1);
   }
   Int_t FindLastBinAbove(Double_t threshold = 0, Int_t axis = 1) const override { return FindLastBinAbove(threshold, axis, 1, -1); }
   Int_t FindFirstBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin = 1, Int_t lastBin = -1) const;
   Int_t FindLastBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin = 1, Int_t lastBin = -1) const;
#else
   Int_t FindFirstBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin = 1, Int_t lastBin = -1) const override;
   Int_t FindLastBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin = 1, Int_t lastBin = -1) const override;
#endif
   virtual void     FitSlices(TF1* f1 = nullptr, Int_t firstbin = 0, Int_t lastbin = -1, Int_t cut = 0,
                              Option_t* option = "QNR", TObjArray* arr = nullptr);
   Int_t            GetBin(Int_t binx, Int_t biny = 0, Int_t binz = 0) const override;
   virtual Double_t GetBinWithContent2(Double_t c, Int_t& binx, Int_t& biny, Int_t firstxbin = 1, Int_t lastxbin = -1,
                                       Int_t firstybin = 1, Int_t lastybin = -1, Double_t maxdiff = 0) const;
   Double_t         GetCellContent(Int_t binx, Int_t biny) const override;
   Double_t         GetCellError(Int_t binx, Int_t biny) const override;
   virtual Double_t GetCorrelationFactor(Int_t axis1 = 1, Int_t axis2 = 2) const;
   virtual Double_t GetCovariance(Int_t axis1 = 1, Int_t axis2 = 2) const;
   virtual void     GetRandom2(Double_t& x, Double_t& y);
   void             GetStats(Double_t* stats) const override;
   Double_t         Integral(Option_t* option = "") const override;
   using TH1::Integral;
   virtual Double_t Integral(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin,
                             Option_t* option = "") const;
   virtual Double_t Integral(Int_t, Int_t, Int_t, Int_t, Int_t, Int_t, Option_t* = "") const { return 0; }
   using TH1::IntegralAndError;
   virtual Double_t IntegralAndError(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Double_t& error,
                                     Option_t* option = "") const;
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
   Double_t Interpolate(Double_t) override;
   Double_t Interpolate(Double_t, Double_t) override;
   Double_t Interpolate(Double_t, Double_t, Double_t) override;
#else
   Double_t Interpolate(Double_t) const override;
   Double_t Interpolate(Double_t, Double_t) const override;
   Double_t Interpolate(Double_t, Double_t, Double_t) const override;
#endif
   Double_t          KolmogorovTest(const TH1* h2, Option_t* option = "") const override;
   Long64_t          Merge(TCollection* list) override;
   virtual TProfile* Profile(const char* name = "_pf", Int_t firstbin = 1, Int_t lastbin = -1,
                             Option_t* option = "") const;
   virtual TH1D*     Projection(const char* name = "_pr", Int_t firstBin = 0, Int_t lastBin = -1,
                                Option_t* option = "") const;
   void              PutStats(Double_t* stats) override;
   virtual GHSym*    Rebin2D(Int_t ngroup = 2, const char* newname = "");
   void              Reset(Option_t* option = "") override;
   void              SetCellContent(Int_t binx, Int_t biny, Double_t content) override;
   void              SetCellError(Int_t binx, Int_t biny, Double_t content) override;
   virtual void      SetShowProjectionX(Int_t nbins = 1);   // *MENU*
   virtual void      SetShowProjectionY(Int_t nbins = 1);   // *MENU*
   TH1*              ShowBackground(Int_t niter = 20, Option_t* option = "same") override;
   Int_t             ShowPeaks(Double_t sigma = 2, Option_t* option = "", Double_t threshold = 0.05) override;   // *MENU*
   void              Smooth(Int_t ntimes = 1, Option_t* option = "") override;                                   // *MENU*

protected:
   using TH1::DoIntegral;
   virtual Double_t DoIntegral(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Double_t& error, Option_t* option,
                               Bool_t doError = kFALSE) const;

   TH2* Matrix() { return fMatrix; }
   void Matrix(TH2* val) { fMatrix = val; }

private:
   Double_t fTsumwy{0.};        ///< Total Sum of weight*Y
   Double_t fTsumwy2{0.};       ///< Total Sum of weight*Y*Y
   Double_t fTsumwxy{0.};       ///< Total Sum of weight*X*Y
   TH2*     fMatrix{nullptr};   //!<! Transient pointer to the 2D-Matrix used in Draw() or GetMatrix()

   /// /cond CLASSIMP
   ClassDefOverride(GHSym, 1)   // NOLINT(readability-else-after-return)
                                /// /endcond
};

class GHSymF : public GHSym, public TArrayF {
public:
   GHSymF();
   GHSymF(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
   GHSymF(const char* name, const char* title, Int_t nbins, const Double_t* bins);
   GHSymF(const char* name, const char* title, Int_t nbins, const Float_t* bins);
   GHSymF(const GHSymF&);
   GHSymF(GHSymF&&) noexcept;
   ~GHSymF();

   TH2F* GetMatrix(bool force = false);

   void     AddBinContent(Int_t bin) override { ++fArray[bin]; }
   void     AddBinContent(Int_t bin, Double_t w) override { fArray[bin] += static_cast<Float_t>(w); }
   void     Copy(TObject& rh) const override;
   void     Draw(Option_t* option = "") override { GetMatrix()->Draw(option); }
   TH1*     DrawCopy(Option_t* option = "", const char* name_postfix = "_copy") const override;
   Double_t GetBinContent(Int_t bin) const override;
   Double_t GetBinContent(Int_t binx, Int_t biny) const override { return GetBinContent(GetBin(binx, biny)); }
   Double_t GetBinContent(Int_t binx, Int_t biny, Int_t) const override { return GetBinContent(GetBin(binx, biny)); }
   void     Reset(Option_t* option = "") override;
   Double_t RetrieveBinContent(Int_t bin) const override { return static_cast<Double_t>(fArray[bin]); }
   void     SetBinContent(Int_t bin, Double_t content) override;
   void     SetBinContent(Int_t binx, Int_t biny, Double_t content) override { SetBinContent(GetBin(binx, biny), content); }
   void     SetBinContent(Int_t binx, Int_t biny, Int_t, Double_t content) override
   {
      SetBinContent(GetBin(binx, biny), content);
   }
   void          SetBinsLength(Int_t n = -1) override;
   void          UpdateBinContent(Int_t bin, Double_t content) override { fArray[bin] = static_cast<Float_t>(content); }
   GHSymF&       operator=(const GHSymF& h1);
   GHSymF&       operator=(GHSymF&&) noexcept;
   friend GHSymF operator*(Float_t c1, GHSymF& h1);
   friend GHSymF operator*(GHSymF& h1, Float_t c1) { return operator*(c1, h1); }
   friend GHSymF operator+(GHSymF& h1, GHSymF& h2);
   friend GHSymF operator-(GHSymF& h1, GHSymF& h2);
   friend GHSymF operator*(GHSymF& h1, GHSymF& h2);
   friend GHSymF operator/(GHSymF& h1, GHSymF& h2);

   /// /cond CLASSIMP
   ClassDefOverride(GHSymF, 1)   // NOLINT(readability-else-after-return)
                                 /// /endcond
};

class GHSymD : public GHSym, public TArrayD {
public:
   GHSymD();
   GHSymD(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
   GHSymD(const char* name, const char* title, Int_t nbins, const Double_t* bins);
   GHSymD(const char* name, const char* title, Int_t nbins, const Float_t* bins);
   GHSymD(const GHSymD&);
   GHSymD(GHSymD&&) noexcept;
   ~GHSymD();

   TH2D* GetMatrix(bool force = false);

   void     AddBinContent(Int_t bin) override { ++fArray[bin]; }
   void     AddBinContent(Int_t bin, Double_t w) override { fArray[bin] += w; }
   void     Copy(TObject& rh) const override;
   TH1*     DrawCopy(Option_t* option = "", const char* name_postfix = "_copy") const override;
   void     Draw(Option_t* option = "") override { GetMatrix()->Draw(option); }
   Double_t GetBinContent(Int_t bin) const override;
   Double_t GetBinContent(Int_t binx, Int_t biny) const override { return GetBinContent(GetBin(binx, biny)); }
   Double_t GetBinContent(Int_t binx, Int_t biny, Int_t) const override { return GetBinContent(GetBin(binx, biny)); }
   void     Reset(Option_t* option = "") override;
   Double_t RetrieveBinContent(Int_t bin) const override { return static_cast<Double_t>(fArray[bin]); }
   void     SetBinContent(Int_t bin, Double_t content) override;
   void     SetBinContent(Int_t binx, Int_t biny, Double_t content) override { SetBinContent(GetBin(binx, biny), content); }
   void     SetBinContent(Int_t binx, Int_t biny, Int_t, Double_t content) override
   {
      SetBinContent(GetBin(binx, biny), content);
   }
   void          SetBinsLength(Int_t n = -1) override;
   void          UpdateBinContent(Int_t bin, Double_t content) override { fArray[bin] = content; }
   GHSymD&       operator=(const GHSymD& h1);
   GHSymD&       operator=(GHSymD&&) noexcept;
   friend GHSymD operator*(Float_t c1, GHSymD& h1);
   friend GHSymD operator*(GHSymD& h1, Float_t c1) { return operator*(c1, h1); }
   friend GHSymD operator+(GHSymD& h1, GHSymD& h2);
   friend GHSymD operator-(GHSymD& h1, GHSymD& h2);
   friend GHSymD operator*(GHSymD& h1, GHSymD& h2);
   friend GHSymD operator/(GHSymD& h1, GHSymD& h2);

   /// /cond CLASSIMP
   ClassDefOverride(GHSymD, 1)   // NOLINT(readability-else-after-return)
                                 /// /endcond
};
#endif
