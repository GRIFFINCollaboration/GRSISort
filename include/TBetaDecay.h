#ifndef TBETADECAY_H
#define TBETADECAY_H

#include <iostream>
#include <fstream>
#include <string>

#include "TNamed.h"
#include "TNucleus.h"

class TBetaDecay : public TNamed {
public:
  TBetaDecay();
  TBetaDecay(TNucleus *parent);
  TBetaDecay(char* name);
  TBetaDecay(Int_t Z, Int_t N);
  ~TBetaDecay();

public:
  TNucleus *GetParent() const {return fparent;} 

private:

  Bool_t fparent_allocated; //true if TNucleus was instantiated in TBetaDecay
  TNucleus *fparent;        //The parent nucleus beta decaying

  ClassDef(TBetaDecay,1);   //Information about beta decays
 
};
#endif
