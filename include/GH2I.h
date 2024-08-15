#ifndef GH2I__H
#define GH2I__H

#include <cstdio>
#include <map>

#include <TNamed.h>
#include <TH2.h>
#include <TList.h>
#include <TVirtualPad.h>
#include <TFrame.h>

#include <GH2Base.h>

class GH1D;

class GH2I : public TH2I, public GH2Base {

public:
   GH2I() = default;
   explicit GH2I(const TObject&);
   GH2I(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, const Double_t* ybins);
   GH2I(const char* name, const char* title, Int_t nbinsx, const Float_t* xbins, Int_t nbinsy, const Float_t* ybins);
   GH2I(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, Double_t ylow, Double_t yup);
   GH2I(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t* ybins);
   GH2I(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, Int_t nbinsy, Double_t ylow, Double_t yup);
	GH2I(const GH2I&) = default;
	GH2I(GH2I&&) noexcept = default;
	GH2I& operator=(const GH2I&) = default;
	GH2I& operator=(GH2I&&) noexcept = default;
   ~GH2I();

   void Draw(Option_t* opt = "") override;

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
   TH1* DrawCopy(Option_t* opt = "") const;
#else
   TH1* DrawCopy(Option_t* opt = "", const char* name_postfix = "_copy") const override;
#endif

   TH1* DrawNormalized(Option_t* opt = "", Double_t norm = 1) const override;

   void     Clear(Option_t* opt = "") override;
   void     Print(Option_t* opt = "") const override;
   void     Copy(TObject&) const override;
   TObject* Clone(const char* newname = "") const override;

   GH1D* ProjectionX(const char* name = "_px", int firstbin = 0, int lastbin = -1, Option_t* option = "");   // *MENU*

   GH1D* ProjectionY(const char* name = "_py", int firstbin = 0, int lastbin = -1, Option_t* option = "");   // *MENU*

   TH2* GetTH2() override { return this; }

private:
	/// /cond CLASSIMP
   ClassDefOverride(GH2I, 2) // NOLINT
	/// /endcond
};
#endif
