#include "TH1.h"
#include "TH2.h"
#include "TArrayF.h"
#include "TArrayD.h"

class GHSym : public TH1 {
	public:
		GHSym();
		GHSym(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up);
		GHSym(const char* name, const char* title, Int_t nbins, const Double_t* bins);
		GHSym(const char* name, const char* title, Int_t nbins, const Float_t* bins);
		~GHSym();
	
		Int_t BufferEmpty(Int_t action = 0);
		Int_t BufferFill(Double_t x, Double_t y, Double_t w);
		Int_t Fill(Double_t x, Double_t y);
		Int_t Fill(Double_t x, Double_t y, Double_t w);

		TH1D* Projection(const char* name = "_pr", Int_t firstBin = 0, Int_t lastBin = -1, Option_t* opt = "") const;
		Int_t GetBin(Int_t binx, Int_t biny) const;

		virtual void     Copy(TObject& hnew) const;

	protected:
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
		virtual TH1*     DrawCopy(Option_t *option="") const;
		virtual Double_t GetBinContent(Int_t bin) const;
		virtual Double_t GetBinContent(Int_t binx, Int_t biny) const { return GetBinContent(GetBin(binx,biny)); }
		virtual Double_t GetBinContent(Int_t binx, Int_t biny, Int_t) const { return GetBinContent(GetBin(binx,biny)); }
		virtual void     Reset(Option_t* option="");
		virtual void     SetBinContent(Int_t bin, Double_t content);
		virtual void     SetBinContent(Int_t binx, Int_t biny, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinContent(Int_t binx, Int_t biny, Int_t, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinsLength(Int_t n=-1);
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
		virtual TH1*     DrawCopy(Option_t *option="") const;
		virtual Double_t GetBinContent(Int_t bin) const;
		virtual Double_t GetBinContent(Int_t binx, Int_t biny) const { return GetBinContent(GetBin(binx,biny)); }
		virtual Double_t GetBinContent(Int_t binx, Int_t biny, Int_t) const { return GetBinContent(GetBin(binx,biny)); }
		virtual void     Reset(Option_t* option="");
		virtual void     SetBinContent(Int_t bin, Double_t content);
		virtual void     SetBinContent(Int_t binx, Int_t biny, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinContent(Int_t binx, Int_t biny, Int_t, Double_t content) { SetBinContent(GetBin(binx,biny),content); }
		virtual void     SetBinsLength(Int_t n=-1);
		GHSymD&    operator=(const GHSymD& h1);
		friend  GHSymD     operator*(Float_t c1, GHSymD &h1);
		friend  GHSymD     operator*(GHSymD& h1, Float_t c1) { return operator*(c1,h1); }
		friend  GHSymD     operator+(GHSymD& h1, GHSymD& h2);
		friend  GHSymD     operator-(GHSymD& h1, GHSymD& h2);
		friend  GHSymD     operator*(GHSymD& h1, GHSymD& h2);
		friend  GHSymD     operator/(GHSymD& h1, GHSymD& h2);


		ClassDef(GHSymD, 1);
};

