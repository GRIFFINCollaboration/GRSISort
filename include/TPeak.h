#include "TGRSIFunctions.h"
#include "TGRSIFit.h"
#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include <string>
#include <algorithm>

using namespace TGRSIFunctions;


////////////////////////////////////////////////////////////////
//                                                            //
// TPeak                                                      //
//                                                            //
// This Class is used to represent fitted data that is        //
// Gaussian like in nature (ie centroid and area).            //
//                                                            //
////////////////////////////////////////////////////////////////

//It might make sense to have this inherit from TF1 instead of including a TF1 inside of it. Not exactly sure.
class TPeak : public TGRSIFit {
 public: 
   TPeak():ffitfunc(0),ffitbg(0),ffithist(0){};
   ~TPeak(){};

   TPeak(Double_t cent, Double_t xlow = 0, Double_t xhigh = 0, TH1* = 0, Option_t* type = "gsc");

 public:   
   //This is public as it may be used for initial guesses
   void SetCentroid(Double_t cent)  { fcentroid = cent; }
   void SetType(Option_t *);

   Double_t GetCentroid() const     { return fcentroid; }
   Double_t GetCentroidErr() const  { return fd_centroid; }
   Double_t GetArea() const         { return farea; }
   Double_t GetAreaErr() const      { return fd_area; }

   Double_t GetParameter(const char *parname) const      { return ffitbg->GetParameter(parname);}
   Double_t GetParameter(Int_t &parnumber) const         { return ffitbg->GetParameter(parnumber);}
   Double_t GetParError(const char *parname) const       { return ffitbg->GetParError(GetParNumber(parname)); }
   Double_t GetParError(Int_t &parnumber) const          { return ffitbg->GetParError(parnumber);}
   const char *GetParName(Int_t &parnumber) const        { return ffitbg->GetParName(parnumber);}
   Int_t GetParNumber(const char *parname) const         { return ffitbg->GetParNumber(parname);}

   Double_t Fit(Option_t *opt = "");
   Double_t Fit(TH1* hist, Option_t *opt = "");
   Double_t Fit(const char* histname, Option_t *opt);

   TF1* GetFitFunction() const      { return ffitbg; } 
   TH1* GetHist() const             { return ffithist;} 

 public:
   void SetCentroidErr(Double_t centerr){fd_centroid = centerr;}
   void SetCentroid(Double_t cent, Double_t d_cent) { SetCentroid(cent); SetCentroidErr(d_cent);}

   Bool_t SetHist(TH1* hist = 0);
   Bool_t SetHist(const char* histname);

 protected:  
   void SetFitResult(TFitResultPtr fitres);
   void SetArea(Double_t a){farea = a;}
   void SetAreaErr(Double_t d_a){fd_area = d_a;}
   void SetArea(Double_t a, Double_t d_a){SetArea(a);SetAreaErr(d_a);}

   void InitParams();

 public:
   virtual void Print(Option_t *opt = "") const;
   virtual void Clear();

 private:  
   //Centroid will eventually be read from parameters
   Double_t fcentroid; //->
   Double_t fd_centroid; //->
   Double_t farea; //->
   Double_t fd_area; //->

   TFitResultPtr ffitres;//->
   TF1* ffitfunc;
   TF1* ffitbg;
   TH1* ffithist;

   Bool_t init_flag;

  ClassDef(TPeak,1);

};

