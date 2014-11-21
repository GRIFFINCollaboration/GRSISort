#include "TGRSIFunctions.h"
#include "TF1.h"
#include "TNamed.h"

#include <string>

using namespace TGRSIFunctions;


////////////////////////////////////////////////////////////////
//                                                            //
// TPeak                                                      //
//                                                            //
// This Class is used to represent fitted data that is        //
// Gaussian like in nature (ie centroid and area).            //
//                                                            //
////////////////////////////////////////////////////////////////

class TPeak : public TNamed {
   friend class TGRSIFitter;
 public: 
   TPeak(){};
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

   TF1* GetFit() const              { return ffitfunc; } 

 public:
   void SetCentroidErr(Double_t centerr){fd_centroid = centerr;}
   void SetCentroid(Double_t cent, Double_t d_cent) { SetCentroid(cent); SetCentroidErr(d_cent);}

   void SetArea(Double_t a){farea = a;}
   void SetAreaErr(Double_t d_a){fd_area = d_a;}
   void SetArea(Double_t a, Double_t d_a){SetArea(a);SetAreaErr(d_a);}

 public:
   virtual void Print();
   virtual void Clear();

 public:   
   Double_t fcentroid; //->
   Double_t fd_centroid; //->
   Double_t farea; //->
   Double_t fd_area; //->

   TF1* ffitfunc = 0;

  ClassDef(TPeak,1);

};

