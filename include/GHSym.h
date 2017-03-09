#include "TH1.h"
#include "TH2.h"
#include "TArrayF.h"
#include "TArrayD.h"
#include "TProfile.h"

class GHSym : public TH1 {
	public:
		GHSym();
		GHSym(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
		GHSym(const char* name, const char* title, Int_t nbins, const Double_t* bins);
		GHSym(const char* name, const char* title, Int_t nbins, const Float_t* bins);
		~GHSym();
	
		virtual Int_t     BufferEmpty(Int_t action = 0);
		virtual Int_t     BufferFill(Double_t x, Double_t y, Double_t w);
		virtual void      Copy(TObject& hnew) const;
		virtual Int_t     Fill(Double_t);
		virtual Int_t     Fill(Double_t x, Double_t y);
		virtual Int_t     Fill(Double_t x, Double_t y, Double_t w);
		virtual Int_t     Fill(const char* namex, const char* namey, Double_t w);
		virtual void      FillN(Int_t, const Double_t *, const Double_t *, Int_t) {;} //MayNotUse
   	virtual void      FillN(Int_t ntimes, const Double_t *x, const Double_t *y, const Double_t *w, Int_t stride = 1);
   	virtual void      FillRandom(const char *fname, Int_t ntimes = 5000);
   	virtual void      FillRandom(TH1 *h, Int_t ntimes = 5000);
		virtual Int_t     FindFirstBinAbove(Double_t threshold = 0, Int_t axis = 1) const;
		virtual Int_t     FindLastBinAbove (Double_t threshold = 0, Int_t axis = 1) const;
		virtual void      FitSlices(TF1* f1 = nullptr, Int_t firstbin = 0, Int_t lastbin = -1, Int_t cut = 0, Option_t* option = "QNR", TObjArray* arr = nullptr);
		virtual Int_t     GetBin(Int_t binx, Int_t biny) const;
		virtual Double_t  GetBinWithContent2(Double_t c, Int_t& binx, Int_t& biny, Int_t firstxbin = 1, Int_t lastxbin = -1, Int_t firstybin = 1, Int_t lastybin = -1, Double_t maxdiff = 0) const;
		virtual Double_t  GetCellContent(Int_t binx, Int_t biny) const;
		virtual Double_t  GetCellError(Int_t binx, Int_t biny) const;
		virtual Double_t  GetCorrelationFactor(Int_t axis1 = 1, Int_t axis2 = 2) const;
		virtual Double_t  GetCovariance(Int_t axis1 = 1, Int_t axis2 = 2) const;
		virtual void      GetRandom2(Double_t& x, Double_t& y);
		virtual void      GetStats(Double_t* stats) const;
		virtual Double_t  Integral(Option_t* option = "") const;
		using TH1::Integral;
		virtual Double_t  Integral(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Option_t* option = "") const;
		virtual Double_t  Integral(Int_t, Int_t, Int_t, Int_t, Int_t, Int_t, Option_t * ="") const { return 0; }
		using TH1::IntegralAndError;
		virtual Double_t  IntegralAndError(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Double_t & error, Option_t* option = "") const;
		virtual Double_t  Interpolate(Double_t);
		virtual Double_t  Interpolate(Double_t, Double_t);
		virtual Double_t  Interpolate(Double_t, Double_t, Double_t);
		virtual Double_t  KolmogorovTest(const TH1* h2, Option_t* option = "") const;
		virtual Long64_t  Merge(TCollection* list);
		virtual TProfile* Profile(const char *name = "_pf", Int_t firstbin = 1, Int_t lastbin = -1, Option_t* option = "") const;
		virtual TH1D*     Projection(const char* name = "_pr", Int_t firstBin = 0, Int_t lastBin = -1, Option_t* opt = "") const;
		virtual void      PutStats(Double_t* stats);
		virtual GHSym*    Rebin2D(Int_t ngroup = 2, const char* newname = "");
		virtual void      Reset(Option_t* option="");
		virtual void      SetCellContent(Int_t binx, Int_t biny, Double_t content);
		virtual void      SetCellError(Int_t binx, Int_t biny, Double_t error);
		virtual void      SetShowProjectionX(Int_t nbins = 1);  // *MENU*
		virtual void      SetShowProjectionY(Int_t nbins = 1);  // *MENU*
		virtual TH1*      ShowBackground(Int_t niter = 20, Option_t* option = "same");
		virtual Int_t     ShowPeaks(Double_t sigma = 2, Option_t* option = "", Double_t threshold = 0.05); // *MENU*
		virtual void      Smooth(Int_t ntimes = 1, Option_t* option = ""); // *MENU*

	protected:
		virtual Double_t DoIntegral(Int_t ix1, Int_t ix2, Int_t iy1, Int_t iy2, Double_t& err, Option_t* opt, Bool_t doerr = kFALSE) const;
		Double_t fTsumwy;		//Total Sum of weight*Y
		Double_t fTsumwy2;	//Total Sum of weight*Y*Y
		Double_t fTsumwxy;	//Total Sum of weight*X*Y

	ClassDef(GHSym, 1);
};

class GHSymF : public GHSym, public TArrayF {
	public:
		GHSymF();
		GHSymF(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
		GHSymF(const char* name, const char* title, Int_t nbins, const Double_t* bins);
		GHSymF(const char* name, const char* title, Int_t nbins, const Float_t* bins);
		~GHSymF();

		TH2F* GetMatrix();

		virtual void     AddBinContent(Int_t bin) { ++fArray[bin]; }
		virtual void     AddBinContent(Int_t bin, Double_t w)	{ fArray[bin] += Float_t(w); }
		virtual void     Copy(TObject& hnew) const;
		virtual void     Draw(Option_t* option="") { GetMatrix()->Draw(option); }
		virtual TH1*     DrawCopy(Option_t *option = "") const;
		virtual Double_t GetBinContent(Int_t bin) const;
		virtual Double_t GetBinContent(Int_t binx, Int_t biny) const { return GetBinContent(GetBin(binx,biny)); }
		virtual Double_t GetBinContent(Int_t binx, Int_t biny, Int_t) const { return GetBinContent(GetBin(binx,biny)); }
		virtual void     Reset(Option_t* option = "");
		virtual void     SetBinContent(Int_t bin, Double_t content);
		virtual void     SetBinContent(Int_t binx, Int_t biny, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinContent(Int_t binx, Int_t biny, Int_t, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinsLength(Int_t n = -1);
		GHSymF&    operator=(const GHSymF& h1);
		friend  GHSymF     operator*(Float_t c1, GHSymF &h1);
		friend  GHSymF     operator*(GHSymF& h1, Float_t c1) { return operator*(c1,h1); }
		friend  GHSymF     operator+(GHSymF& h1, GHSymF& h2);
		friend  GHSymF     operator-(GHSymF& h1, GHSymF& h2);
		friend  GHSymF     operator*(GHSymF& h1, GHSymF& h2);
		friend  GHSymF     operator/(GHSymF& h1, GHSymF& h2);

		ClassDef(GHSymF, 1);
};

class GHSymD : public GHSym, public TArrayD {
	public:
		GHSymD();
		GHSymD(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
		GHSymD(const char* name, const char* title, Int_t nbins, const Double_t* bins);
		GHSymD(const char* name, const char* title, Int_t nbins, const Float_t* bins);
		~GHSymD();

		TH2D* GetMatrix();

		virtual void     AddBinContent(Int_t bin) { ++fArray[bin]; }
		virtual void     AddBinContent(Int_t bin, Double_t w)	{ fArray[bin] += w; }
		virtual void     Copy(TObject& hnew) const;
		virtual TH1*     DrawCopy(Option_t *option = "") const;
		virtual void     Draw(Option_t* option="") { GetMatrix()->Draw(option); }
		virtual Double_t GetBinContent(Int_t bin) const;
		virtual Double_t GetBinContent(Int_t binx, Int_t biny) const { return GetBinContent(GetBin(binx,biny)); }
		virtual Double_t GetBinContent(Int_t binx, Int_t biny, Int_t) const { return GetBinContent(GetBin(binx,biny)); }
		virtual void     Reset(Option_t* option = "");
		virtual void     SetBinContent(Int_t bin, Double_t content);
		virtual void     SetBinContent(Int_t binx, Int_t biny, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinContent(Int_t binx, Int_t biny, Int_t, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinsLength(Int_t n = -1);
		GHSymD&    operator=(const GHSymD& h1);
		friend  GHSymD     operator*(Float_t c1, GHSymD &h1);
		friend  GHSymD     operator*(GHSymD& h1, Float_t c1) { return operator*(c1,h1); }
		friend  GHSymD     operator+(GHSymD& h1, GHSymD& h2);
		friend  GHSymD     operator-(GHSymD& h1, GHSymD& h2);
		friend  GHSymD     operator*(GHSymD& h1, GHSymD& h2);
		friend  GHSymD     operator/(GHSymD& h1, GHSymD& h2);


		ClassDef(GHSymD, 1);
};

