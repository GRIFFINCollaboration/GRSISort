#include "TBetaDecay.h"

ClassImp(TBetaDecay);


TBetaDecay::TBetaDecay(){}

TBetaDecay::TBetaDecay(TNucleus *parent):fparent(parent){

//This is where we want to open parameter files,etc.



}

TBetaDecay::TBetaDecay(char *name){
   
   TNucleus *fparent = new TNucleus(name);

}

TBetaDecay::TBetaDecay(Int_t Z, Int_t N){

   TNucleus *fparent = new TNucleus(Z,N);

}


TBetaDecay::~TBetaDecay(){

   delete fparent;

}
