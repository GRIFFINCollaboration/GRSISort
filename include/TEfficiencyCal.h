#ifndef __TEFFICIENCYCAL_H__
#define __TEFFICIENCYCAL_H__

#include "TCal.h"
#include "TPeak.h"
#include "TSpectrum.h"

class TEfficiencyCal : public TCal {
 public: 
   TEfficiencyCal();
   TEfficiencyCal(const char* name, const char* title) : TCal(name,title){}
   ~TEfficiencyCal(); 

 public:
   std::vector<Double_t> GetParameters() const;
   Double_t GetParameter(Int_t parameter) const;

   void Clear(Option_t *opt = "");
   void Print(Option_t *opt = "") const;

   void SetFitFunction(void* fnc);

   Bool_t IsGroupable() const {return true;}

   void ScaleGraph(Double_t scale_factor) const{};

 private:
   Double_t fscale_factor;
   
   ClassDef(TEfficiencyCal,1);

};

#endif
