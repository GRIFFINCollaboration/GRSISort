#include "TBetaDecay.h"

ClassImp(TBetaDecay);


TBetaDecay::TBetaDecay(){}

TBetaDecay::TBetaDecay(TNucleus *parent):fparent(parent){


}

TBetaDecay::TBetaDecay(char *name){
   
   fparent_allocated = true;
   TNucleus *fparent = new TNucleus(name);

}

TBetaDecay::TBetaDecay(Int_t Z, Int_t N){
  
   fparent_allocated = true;
   TNucleus *fparent = new TNucleus(Z,N);
 
}

TBetaDecay::~TBetaDecay(){

   if(fparent_allocated && fparent)
   	delete fparent;

}
