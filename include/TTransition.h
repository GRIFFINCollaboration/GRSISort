#ifndef __TTRANSITION_HH
#define __TTRANSITION_HH

#include "TObject.h"

class TTransition : public TObject {
public:
   TTransition(){};
   TTransition(Double_t energy);
   ~TTransition(){};



   Double_t GetEnergy() const {return fEnergy;}
   Char_t   GetType() const   {return fType;} 

   void SetEnergy(Double_t energy){fEnergy = energy;}
   void SetType(Char_t type)      {fType = type;}

private:
   Double_t fEnergy;
   Char_t     fType;








   ClassDef(TTransition,1);

};


#endif
