#include "TBetaDecay.h"

TBetaDecay::TBetaDecay(TNucleus* parent) : fParent(parent)
{
}

TBetaDecay::TBetaDecay(char* name) : fParentAllocated(true), fParent(new TNucleus(name))
{
}

TBetaDecay::TBetaDecay(Int_t Z, Int_t N) : fParentAllocated(true), fParent(new TNucleus(Z, N))
{
}

TBetaDecay::~TBetaDecay()
{
   if(fParentAllocated) {
      delete fParent;
   }
}
