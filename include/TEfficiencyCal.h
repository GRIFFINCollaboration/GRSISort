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

   void Clear();
   void Print(Option_t *opt = "") const;

   Bool_t IsGroupable() const {return true;}

 private:
   
   ClassDef(TEfficiencyCal,1);

};

#endif
