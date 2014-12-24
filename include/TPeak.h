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

class TPeak : public TGRSIFit {
 public: 
   //ctors and dtors
   ~TPeak();
 //  TPeak(const TPeak &copy);
   TPeak(Double_t cent, Double_t xlow = 0, Double_t xhigh = 0, TH1* fithist = 0, Option_t* type = "gsc");
   
 protected:
   TPeak():ffitfunc(0),ffithist(0),TGRSIFit(){}; //I might make it so if you call this ctor, the TPeak yells at you since it's a fairly useless call anyway

 public:   
   void SetCentroid(Double_t cent)  { fcentroid = cent; }
   void SetType(Option_t *type);

   Bool_t Fit(TH1* fithist = 0);

   Double_t GetCentroid() const     { return fcentroid; }
   Double_t GetCentroidErr() const  { return fd_centroid; }
   Double_t GetArea() const         { return farea; }
   Double_t GetAreaErr() const      { return fd_area; }
/*
   Double_t Fit(Option_t *opt = "");
   Double_t Fit(TH1* hist, Option_t *opt = "");
   Double_t Fit(const char* histname, Option_t *opt);
*/
   TF1* GetFitFunction() const      { return (TF1*)(this); } //I might move the fit functions to TGRSIFit, it's just a little tricky to initilize the function


 protected:  
   void SetCentroidErr(Double_t centerr){fd_centroid = centerr;}
   void SetCentroid(Double_t cent, Double_t d_cent) { SetCentroid(cent); SetCentroidErr(d_cent);}
   void SetArea(Double_t a){farea = a;}
   void SetAreaErr(Double_t d_a){fd_area = d_a;}
   void SetArea(Double_t a, Double_t d_a){SetArea(a);SetAreaErr(d_a);}

 public:
   Bool_t InitParams(TH1 *fithist = 0);

 public:
   virtual void Print(Option_t *opt = "") const;
   virtual void Clear();

 private:  
   //Centroid will eventually be read from parameters
   Double_t fcentroid; //->
   Double_t fd_centroid; //->
   Double_t farea; //->
   Double_t fd_area; //->

   TF1* ffitfunc;
   TH1* ffithist;//!

  ClassDef(TPeak,1);

};

