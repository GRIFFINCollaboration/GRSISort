#ifndef __TBETADECAY_H
#define __TBETADECAY_H

#include <iostream>
#include <fstream>
#include <string>

#include "TNamed.h"
#include "TNucleus.h"

class TBetaDecay : public TNamed {
public:
  TBetaDecay();
  TBetaDecay(TNucleus*);
  ~TBetaDecay(){};

  TNucleus *GetParent() const {return parent;}

private:

  TNucleus *parent; //The parent nucleus beta decaying
 


  ClassDef(TBetaDecay,1);
 
};
#endif
