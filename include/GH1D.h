#ifndef GH1D_H
#define GH1D_H

#include "TH1.h"
#include "TRef.h"

#include "GH2I.h"

class TF1;

class GH1D : public TH1D {
public:
  GH1D() : TH1D(), parent(NULL), projection_axis(-1) { }
  GH1D(const TVectorD& v)
    : TH1D(v), parent(NULL), projection_axis(-1) { }
  GH1D(const char* name, const char* title, Int_t nbinsx, const Float_t* xbins)
    : TH1D(name, title, nbinsx, xbins), parent(NULL), projection_axis(-1) { }
  GH1D(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins)
    : TH1D(name, title, nbinsx, xbins), parent(NULL), projection_axis(-1) { }
  GH1D(const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup)
    : TH1D(name, title, nbinsx, xlow, xup), parent(NULL), projection_axis(-1) { }

  GH1D(const TF1& function,Int_t nbinsx,Double_t xlow,Double_t xup);

  GH1D(const TH1& source);
  //GH1D(const TH1 *source);
  //virtual void SetOption(Option_t* option=" ");

  TObject* GetParent() const { return parent.GetObject(); }
  void SetParent(TObject* obj) { parent = obj; }

  int GetProjectionAxis() const { return projection_axis; }
  void SetProjectionAxis(int axis) { projection_axis = axis; }

  void Clear(Option_t* opt="");
  void Print(Option_t* opt="") const;
  void Copy(TObject& obj) const;
  void Draw(Option_t* opt="");

#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
  TH1 *DrawCopy(Option_t *opt="") const;
#else
  TH1 *DrawCopy(Option_t *opt="",const char* name_postfix="copy") const;
#endif

  TH1 *DrawNormalized(Option_t *opt="",Double_t norm=1) const;

  bool WriteDatFile(const char *outFile);
  
  GH1D* Project(int bins=-1);

  GH1D* GetPrevious(bool DrawEmpty=false) const;
  GH1D* GetNext(bool DrawEmpty=false) const;

  GH1D* Project(double bin_low, double bin_high) const;
  GH1D* Project_Background(double bin_low, double bin_high,
                           double bg_bin_low, double bg_bin_high,
                           kBackgroundSubtraction mode = kRegionBackground) const;

private:
  TRef parent;
  int projection_axis;

  ClassDef(GH1D,1)
};

#endif /* GH1D_H */
