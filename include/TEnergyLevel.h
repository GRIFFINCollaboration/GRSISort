#ifndef __TENERGYLEVEL_HH
#define __TENERGYLEVEL_HH

#include "TObject.h"

class TEnergyLevel : public TObject {
public:
   TEnergyLevel(Double_t energy);
   ~TEnergyLevel();

   Double_t GetEnergy() const {return fEnergy;}
   Double_t GetSpin() const   {return fSpin;}
   Bool_t GetParity() const   {return fParity;}

   void SetEnergy(Double_t energy){fEnergy = energy;}
   void SetSpin(Double_t spin)    {fSpin = spin;}
   void SetParity(Bool_t parity)  {fParity = parity;}

private:
   Double_t fEnergy;
   Double_t fSpin;
   Bool_t fParity; //0 for +, 1 for -





   ClassDef(TEnergyLevel,1);

};

#endif
