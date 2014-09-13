#ifndef __TBETADECAY_H
#define __TBETADECAY_H

#include <iostream>
#include <fstream>
#include <string>

#include "TNamed.h"
#include "TNucleus.h"

class TBetaDecay : public TNamed {
public:
  TBetaDecay(TNucleus*);
  ~TBetaDecay(); 


  ClassDef(TBetaDecay,1);
 
};
#endif
