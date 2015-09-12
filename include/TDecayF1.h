// Author: Ryan Dunlop    09/15

#ifndef TDECAYF1_H
#define TDECAYF1_H

#include "TF1.h"

#include "Rtypes.h"

//#ifndef __CINT__
//#include "Globals.h"
//#endif

////////////////////////////////////////////////////////////////
//                                                            //
// TDecayF1                                                   //
//                                                            //
// This Class is used to extend TF1 to be able to do useful    //
// things with nuclear decays                                 //
//                                                            //
////////////////////////////////////////////////////////////////

class TDecayF1 : public TF1	{
public:
   TDecayF1() : TF1() {}; 
   //TGRSIFit(const char *name,Double_t (*fcn)(Double_t *, Double_t *), Double_t xmin, Double_t xmax, Int_t npar) : TF1(name, fcn, xmin, xmax, npar){};
   TDecayF1(const char* name, const char* formula, Double_t xmin = 0, Double_t xmax = 1) : TF1(name,formula,xmin,xmax){ } 
   TDecayF1(const char* name, Double_t xmin, Double_t xmax, Int_t npar) : TF1(name,xmin,xmax,npar){ }
   TDecayF1(const char* name, void* fcn, Double_t xmin, Double_t xmax, Int_t npar) : TF1(name, fcn,xmin,xmax,npar){}
   TDecayF1(const char* name, ROOT::Math::ParamFunctor f, Double_t xmin = 0, Double_t xmax = 1, Int_t npar = 0) : TF1(name,f,xmin,xmax,npar){}
   TDecayF1(const char* name, void* ptr, Double_t xmin, Double_t xmax, Int_t npar, const char* className) : TF1(name,ptr, xmin, xmax, npar, className){ }
#ifndef __CINT__
   TDecayF1(const char *name, Double_t (*fcn)(Double_t *, Double_t *), Double_t xmin=0, Double_t xmax=1, Int_t npar=0) : TF1(name,fcn,xmin,xmax,npar) {}
   TDecayF1(const char *name, Double_t (*fcn)(const Double_t *, const Double_t *), Double_t xmin=0, Double_t xmax=1, Int_t npar=0) : TF1(name,fcn,xmin,xmax,npar) {}
#endif
   TDecayF1(const char *name, void *ptr, void *ptr2,Double_t xmin, Double_t xmax, Int_t npar, const char *className, const char *methodName = 0) : TF1(name,ptr,ptr2,xmin,xmax,npar,className,methodName){}

   template <class PtrObj, typename MemFn>
   TDecayF1(const char *name, const  PtrObj& p, MemFn memFn, Double_t xmin, Double_t xmax, Int_t npar, const char * className = 0, const char *methodName = 0) : TF1(name,p,memFn,xmin,xmax,npar,className,methodName) {}

   template <typename Func>
   TDecayF1(const char *name, Func f, Double_t xmin, Double_t xmax, Int_t npar, const char *className = 0  ) : TF1(name,f,xmin,xmax,npar,className){}

   ClassDef(TDecayF1,1);  // Extends TF1 for nuclear decays
};
#endif // TFRAGMENT_H
