#include "TBetaDecay.h"

ClassImp(TBetaDecay)

/////////////////////////////////////////////////////////////////
//
// TBetaDecay
//
// This class contains information about beta decays to be used
// in analyses.
//
//////////////////////////////////////////////////////////////////


TBetaDecay::TBetaDecay(){}

TBetaDecay::TBetaDecay(TNucleus *parent):fparent(parent){
   fparent_allocated = false;
}

TBetaDecay::TBetaDecay(char *name){
   fparent_allocated = true;
   fparent = new TNucleus(name);
}

TBetaDecay::TBetaDecay(Int_t Z, Int_t N){
   fparent_allocated = true;
   fparent = new TNucleus(Z,N);
}

TBetaDecay::~TBetaDecay(){
   if(fparent_allocated && fparent)
   	delete fparent;
}
