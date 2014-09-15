#include "TBetaDecay.h"

ClassImp(TBetaDecay);


TBetaDecay::TBetaDecay(){}

TBetaDecay::TBetaDecay(TNucleus *parent):fparent(parent){

//This is where we want to open parameter files,etc.
//This is currently dangerous and will delete the TNucleus that is passed
//even though you may want it to survive beyond the deconstruction of TBetaDecay


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
