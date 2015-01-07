#include "TCal.h"

ClassImp(TCal)

TCal::TCal(){
   fgraph = new TGraphErrors;
   Clear();
}

TCal::TCal(const char* name, const char* title) : TCal() {
   SetNameTitle(name,title);
   fgraph->SetNameTitle(name,title);
}

TCal::~TCal(){
   delete fgraph;
}

void TCal::Clear(){
   this->fchanNum = 9999;
}

void TCal::Print(Option_t *opt) const{
   printf("Channel Number: %u\n",fchanNum);
   printf("Coefficients:\n");
//   for(Int_t i=0; i<fcoeffs.size();i++){
//      printf("Coefficient %d: %lf +/- %lf\n",i,fcoeffs[i],fdcoeffs[i]);
 //  }

}
