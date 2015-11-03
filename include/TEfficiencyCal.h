#ifndef TEFFICIENCYCAL_H__
#define TEFFICIENCYCAL_H__

#include "TCal.h"
#include "TPeak.h"
#include "TSpectrum.h"

class TEfficiencyCal : public TCal {
 public: 
   TEfficiencyCal();
   TEfficiencyCal(const char* name, const char* title) : TCal(name,title){}
   virtual ~TEfficiencyCal(); 

   TEfficiencyCal(const TEfficiencyCal &copy);

 public:
   void Copy(TObject &obj) const;
   void AddPoint(Double_t energy, Double_t area, Double_t dEnergy=0.0, Double_t dArea=0.0);
   void AddPoint(TPeak *peak);

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;

   Bool_t IsGroupable() const {return true;}

   void ScaleGraph(Double_t scaleFactor);

 private:
   Double_t fScaleFactor;
   
   ClassDef(TEfficiencyCal,1);

};

#endif
