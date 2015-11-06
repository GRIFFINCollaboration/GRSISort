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
  virtual ~TBetaDecay();

public:
  TNucleus* GetParent() const { return fParent; } 

private:

  Bool_t fParentAllocated;  ///< true if TNucleus was instantiated in TBetaDecay
  TNucleus* fParent;        ///< The parent nucleus beta decaying

/// \cond CLASSIMP
  ClassDef(TBetaDecay,1);   //Information about beta decays
/// \endcond
};
#endif
