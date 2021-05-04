#ifndef GCUBE_H
#define GCUBE_H

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TArrayF.h"
#include "TArrayD.h"
#include "TProfile.h"
#include "TF1.h"

class GCube : public TH1 {
public:
   GCube();
   GCube(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
   GCube(const char* name, const char* title, Int_t nbins, const Double_t* bins);
   GCube(const char* name, const char* title, Int_t nbins, const Float_t* bins);
   ~GCube() override;

   Int_t BufferEmpty(Int_t action = 0) override;
   Int_t         BufferFill(Double_t, Double_t) override { return -2; } // MayNotUse
   virtual Int_t BufferFill(Double_t x, Double_t y, Double_t z, Double_t w);
   void Copy(TObject& obj) const override;
   Int_t         Fill(Double_t) override;                                     // MayNotUse
   Int_t         Fill(Double_t, Double_t) override { return Fill(0.); }       // MayNotUse
   Int_t         Fill(const char*, Double_t) override { return Fill(0); }     // MayNotUse
   virtual Int_t Fill(Double_t, const char*, Double_t) { return Fill(0); }    // MayNotUse
   virtual Int_t Fill(const char*, Double_t, Double_t) { return Fill(0); }    // MayNotUse
   virtual Int_t Fill(const char*, const char*, Double_t) { return Fill(0); } // MayNotUse
   virtual Int_t Fill(Double_t x, Double_t y, Double_t z);
   virtual Int_t Fill(Double_t x, Double_t y, Double_t z, Double_t w);
   virtual Int_t Fill(const char* namex, const char* namey, const char* namez, Double_t w);
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 24, 0)
   void FillRandom(const char* fname, Int_t ntimes = 5000) override { FillRandom(fname, ntimes, nullptr); }
   void FillRandom(TH1* h, Int_t ntimes = 5000) override { FillRandom(h, ntimes, nullptr); }
   void FillRandom(const char* fname, Int_t ntimes = 5000, TRandom* rng = nullptr);
   void FillRandom(TH1* h, Int_t ntimes = 5000, TRandom* rng = nullptr);
#else
   void FillRandom(const char* fname, Int_t ntimes = 5000, TRandom* rng = nullptr) override;
   void FillRandom(TH1* h, Int_t ntimes = 5000, TRandom* rng = nullptr) override;
#endif
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 18, 0)
   Int_t FindFirstBinAbove(Double_t threshold = 0, Int_t axis = 1) const override { return FindFirstBinAbove(threshold, axis, 1, -1); }
   Int_t FindLastBinAbove(Double_t threshold = 0, Int_t axis = 1) const override { return FindLastBinAbove(threshold, axis, 1, -1); }
   Int_t FindFirstBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin=1, Int_t lastBin=-1) const;
   Int_t FindLastBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin=1, Int_t lastBin=-1) const;
#else
   Int_t FindFirstBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin=1, Int_t lastBin=-1) const override;
   Int_t FindLastBinAbove(Double_t threshold = 0, Int_t axis = 1, Int_t firstBin=1, Int_t lastBin=-1) const override;
#endif
   virtual void FitSlicesZ(TF1* f1 = nullptr, Int_t binminx = 0, Int_t binmaxx = -1, Int_t binminy = 0,
                           Int_t binmaxy = -1, Int_t cut = 0, Option_t* option = "QNR");
   Int_t GetBin(Int_t binx, Int_t biny = 0, Int_t binz = 0) const override;
   virtual Double_t GetBinWithContent2(Double_t c, Int_t& binx, Int_t& biny, Int_t& binz, Int_t firstxbin = 1,
                                       Int_t lastxbin = -1, Int_t firstybin = 1, Int_t lastybin = -1,
                                       Int_t firstzbin = 1, Int_t lastzbin = -1, Double_t maxdiff = 0) const;
   virtual Double_t GetCorrelationFactor(Int_t axis1 = 1, Int_t axis2 = 2) const;
   virtual Double_t GetCovariance(Int_t axis1 = 1, Int_t axis2 = 2) const;
   virtual void GetRandom3(Double_t& x, Double_t& y, Double_t& z);
   void GetStats(Double_t* stats) const override;
   Double_t Integral(Option_t* option = "") const override;
   using TH1::Integral;
   virtual Double_t Integral(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Int_t firstzbin,
                             Int_t lastzbin, Option_t* option = "") const;
   using TH1::IntegralAndError;
   virtual Double_t IntegralAndError(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Int_t firstzbin,
                                     Int_t lastzbin, Double_t& error, Option_t* option = "") const;
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
   Double_t Interpolate(Double_t) override;
   Double_t Interpolate(Double_t, Double_t) override;
   Double_t Interpolate(Double_t, Double_t, Double_t) override;
#else
   Double_t Interpolate(Double_t) const override;
   Double_t Interpolate(Double_t, Double_t) const override;
   Double_t Interpolate(Double_t, Double_t, Double_t) const override;
#endif
   Double_t KolmogorovTest(const TH1* h2, Option_t* option = "") const override;
   Long64_t Merge(TCollection* list) override;
   virtual TH1D* Projection(const char* name = "_pr", Int_t firstBiny = 0, Int_t lastBiny = -1, Int_t firstBinz = 0,
                            Int_t lastBinz = -1, Option_t* option = "") const;
   void PutStats(Double_t* stats) override;
   virtual GCube* Rebin3D(Int_t ngroup = 2, const char* newname = "");
   void Reset(Option_t* option = "") override;
   virtual void SetShowProjection(const char* option = "xy", Int_t nbins = 1); // *MENU*
   TH1* ShowBackground(Int_t niter = 20, Option_t* option = "same") override;
   Int_t ShowPeaks(Double_t sigma = 2, Option_t* option = "", Double_t threshold = 0.05) override; // *MENU*
   void Smooth(Int_t ntimes = 1, Option_t* option = "") override;                                  // *MENU*

protected:
   using TH1::DoIntegral;
   Double_t DoIntegral(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2, Double_t& error,
                       Option_t* option, Bool_t doError = kFALSE) const override;
   Double_t fTsumwy{0};  // Total Sum of weight*Y
   Double_t fTsumwy2{0}; // Total Sum of weight*Y*Y
   Double_t fTsumwxy{0}; // Total Sum of weight*X*Y
   Double_t fTsumwz{0};  // Total Sum of weight*Z
   Double_t fTsumwz2{0}; // Total Sum of weight*Z*Z
   Double_t fTsumwxz{0}; // Total Sum of weight*X*Z
   Double_t fTsumwyz{0}; // Total Sum of weight*Y*Z
   TH2*     fMatrix{0};  //!<! Transient pointer to the 2D-Matrix used in Draw() or GetMatrix()

private:
   GCube(const GCube&);
   GCube& operator=(const GCube&);

   ClassDefOverride(GCube, 1);
};

class GCubeF : public GCube, public TArrayF {
public:
   GCubeF();
   GCubeF(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
   GCubeF(const char* name, const char* title, Int_t nbins, const Double_t* bins);
   GCubeF(const char* name, const char* title, Int_t nbins, const Float_t* bins);
   GCubeF(const GCubeF&);
   ~GCubeF() override;

   TH2F* GetMatrix(bool force = false);

   void AddBinContent(Int_t bin) override { ++fArray[bin]; }
   void AddBinContent(Int_t bin, Double_t w) override { fArray[bin] += Float_t(w); }
   void Copy(TObject& rh) const override;
   void Draw(Option_t* option = "") override { GetMatrix()->Draw(option); }
   TH1* DrawCopy(Option_t* option = "", const char* name_postfix = "_copy") const override;
   Double_t GetBinContent(Int_t bin) const override;
   Double_t GetBinContent(Int_t bin, Int_t) const override { return GetBinContent(bin); }
   Double_t GetBinContent(Int_t binx, Int_t biny, Int_t binz) const override
   {
      return GetBinContent(GetBin(binx, biny, binz));
   }
   void Reset(Option_t* option = "") override;
   Double_t RetrieveBinContent(Int_t bin) const override { return Double_t(fArray[bin]); }
   void SetBinContent(Int_t bin, Double_t content) override;
   void SetBinContent(Int_t bin, Int_t, Double_t content) override { SetBinContent(bin, content); }
   void SetBinContent(Int_t binx, Int_t biny, Int_t binz, Double_t content) override
   {
      SetBinContent(GetBin(binx, biny, binz), content);
   }
   void SetBinsLength(Int_t n = -1) override;
   void UpdateBinContent(Int_t bin, Double_t content) override { fArray[bin] = static_cast<Float_t>(content); }
   GCubeF& operator=(const GCubeF& h1);
   friend GCubeF operator*(Float_t c1, GCubeF& h1);
   friend GCubeF operator*(GCubeF& h1, Float_t c1) { return operator*(c1, h1); }
   friend GCubeF operator+(GCubeF& h1, GCubeF& h2);
   friend GCubeF operator-(GCubeF& h1, GCubeF& h2);
   friend GCubeF operator*(GCubeF& h1, GCubeF& h2);
   friend GCubeF operator/(GCubeF& h1, GCubeF& h2);

   ClassDefOverride(GCubeF, 1);
};

class GCubeD : public GCube, public TArrayD {
public:
   GCubeD();
   GCubeD(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
   GCubeD(const char* name, const char* title, Int_t nbins, const Double_t* bins);
   GCubeD(const char* name, const char* title, Int_t nbins, const Float_t* bins);
   GCubeD(const GCubeD&);
   ~GCubeD() override;

   TH2D* GetMatrix(bool force = false);

   void AddBinContent(Int_t bin) override { ++fArray[bin]; }
   void AddBinContent(Int_t bin, Double_t w) override { fArray[bin] += w; }
   void Copy(TObject& rh) const override;
   TH1* DrawCopy(Option_t* option = "", const char* name_postfix = "_copy") const override;
   void Draw(Option_t* option = "") override { GetMatrix()->Draw(option); }
   Double_t GetBinContent(Int_t bin) const override;
   Double_t GetBinContent(Int_t bin, Int_t) const override { return GetBinContent(bin); }
   Double_t GetBinContent(Int_t binx, Int_t biny, Int_t binz) const override
   {
      return GetBinContent(GetBin(binx, biny, binz));
   }
   void Reset(Option_t* option = "") override;
   Double_t RetrieveBinContent(Int_t bin) const override { return Double_t(fArray[bin]); }
   void SetBinContent(Int_t bin, Double_t content) override;
   void SetBinContent(Int_t bin, Int_t, Double_t content) override { SetBinContent(bin, content); }
   void SetBinContent(Int_t binx, Int_t biny, Int_t binz, Double_t content) override
   {
      SetBinContent(GetBin(binx, biny, binz), content);
   }
   void SetBinsLength(Int_t n = -1) override;
   void UpdateBinContent(Int_t bin, Double_t content) override { fArray[bin] = content; }
   GCubeD& operator=(const GCubeD& h1);
   friend GCubeD operator*(Float_t c1, GCubeD& h1);
   friend GCubeD operator*(GCubeD& h1, Float_t c1) { return operator*(c1, h1); }
   friend GCubeD operator+(GCubeD& h1, GCubeD& h2);
   friend GCubeD operator-(GCubeD& h1, GCubeD& h2);
   friend GCubeD operator*(GCubeD& h1, GCubeD& h2);
   friend GCubeD operator/(GCubeD& h1, GCubeD& h2);

   ClassDefOverride(GCubeD, 1);
};
#endif
