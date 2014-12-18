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
   TPeak():ffitfunc(0),ffitbg(0){};
   ~TPeak(){};

   TPeak(Double_t cent, Double_t xlow = 0, Double_t xhigh = 0, Option_t* type = "gsc");

 public:   
   //This is public as it may be used for initial guesses
   void SetCentroid(Double_t cent)  { fcentroid = cent; }
   void SetType(Option_t *);

   Double_t GetCentroid() const     { return fcentroid; }
   Double_t GetCentroidErr() const  { return fd_centroid; }
   Double_t GetArea() const         { return farea; }
   Double_t GetAreaErr() const      { return fd_area; }

   TF1* GetFitFunction() const      { return ffitbg; } 

 public:
   void SetCentroidErr(Double_t centerr){fd_centroid = centerr;}
   void SetCentroid(Double_t cent, Double_t d_cent) { SetCentroid(cent); SetCentroidErr(d_cent);}

   void SetArea(Double_t a){farea = a;}
   void SetAreaErr(Double_t d_a){fd_area = d_a;}
   void SetArea(Double_t a, Double_t d_a){SetArea(a);SetAreaErr(d_a);}

 protected:  
   void SetFitResult(TFitResultPtr fitres);

 public:
   virtual void Print() const;
   virtual void Clear();

 private:   
   Double_t fcentroid; //->
   Double_t fd_centroid; //->
   Double_t farea; //->
   Double_t fd_area; //->

   TFitResultPtr ffitres;//->
   TF1* ffitfunc;
   TF1* ffitbg;

  ClassDef(TPeak,1);

};

