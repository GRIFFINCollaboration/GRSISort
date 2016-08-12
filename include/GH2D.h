#ifndef GH2D__H
#define GH2D__H

#include <cstdio>
#include <map>

#include <TNamed.h>
#include <TH2.h>
#include <TList.h>
#include <TVirtualPad.h>
#include <TFrame.h>

#include <TCutG.h>
#include <GH2Base.h>

class GH1D;

class GH2D : public TH2D , public GH2Base {

public:
  GH2D() { }
  GH2D(const TObject&);
  GH2D(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins,Int_t nbinsy, const Double_t *ybins);
  GH2D(const char *name,const char *title,Int_t nbinsx,const Float_t *xbins,Int_t nbinsy, const Float_t *ybins);
  GH2D(const char *name,const char *title,Int_t nbinsx,const Double_t *xbins,
                                          Int_t nbinsy, Double_t ylow, Double_t yup);
  GH2D(const char *name,const char *title,Int_t nbinsx, Double_t xlow, Double_t xup,
                                          Int_t nbinsy, Double_t *ybins);
  GH2D(const char *name,const char *title,Int_t nbinsx, Double_t xlow, Double_t xup,
                                          Int_t nbinsy, Double_t ylow, Double_t yup);
  ~GH2D();

  virtual void Draw(Option_t *opt="");
  virtual void Draw(TCutG*);
  TH1 *DrawCopy(Option_t *opt="") const;
  TH1 *DrawNormalized(Option_t *opt="",Double_t norm=1) const;

  virtual void Clear(Option_t *opt="");
  virtual void Print(Option_t *opt="") const;
  virtual void Copy(TObject&) const;
  virtual TObject *Clone(const char *newname="") const;

  GH1D* ProjectionX(const char* name="_px",
                    int firstbin = 0,
                    int lastbin = -1,
                    Option_t* option=""); // *MENU* 

  GH1D* ProjectionY(const char* name="_py",
                    int firstbin = 0,
                    int lastbin = -1,
                    Option_t* option=""); // *MENU*

  virtual TH2 *GetTH2() { return this; }

private:
  ClassDef(GH2D,1)
};

#endif
