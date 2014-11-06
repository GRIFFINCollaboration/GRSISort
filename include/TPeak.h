#include "TGRSIFunctions.h"
#include "TObject.h"
#include "TF1.h"

using namespace TGRSIFunctions;

class TPeak : public TObject {
   friend class TGRSIFitter;
 public: 
   TPeak(){};
   ~TPeak(){};

   TPeak(Double_t cent) : centroid(cent){}

 public:   
   //This is public as it may be used for initial guesses
   void SetCentroid(Double_t cent)  { centroid = cent; }

   Double_t GetCentroid() const     { return centroid; }
   Double_t GetCentroidErr() const  { return d_centroid; }
   Double_t GetArea() const         { return area; }
   Double_t GetAreaErr() const      { return d_area; }

   TF1* GetFit()                    { return peakfit; } 

 private:
   void SetCentroidErr(Double_t centerr){d_centroid = centerr;}
   void SetCentroid(Double_t cent, Double_t d_cent) { SetCentroid(cent); SetCentroidErr(d_cent);}

   void SetArea(Double_t a){area = a;}
   void SetAreaErr(Double_t d_a){d_area = d_a;}
   void SetArea(Double_t a, Double_t d_a){SetArea(a);SetAreaErr(d_a);}


 private:   
   Double_t centroid;
   Double_t d_centroid;
   Double_t area;
   Double_t d_area;
   
   TF1* peakfit;

  ClassDef(TPeak,1);

};

