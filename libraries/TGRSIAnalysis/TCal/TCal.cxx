#include "TCal.h"

ClassImp(TCal)

TCal::TCal(){
   fgraph = new TGraphErrors;
   fchanNum = 9999;
}

TCal::TCal(const char* name, const char* title) {
   SetNameTitle(name,title);
   fgraph->SetNameTitle(name,title);
}

TCal::~TCal(){
   delete fgraph;
}

void TCal::Clear(Option_t *opt) {
   this->fchanNum = 9999;
   fgraph->Clear();
}

void TCal::Print(Option_t *opt) const{
   printf("Channel Number: %u\n",fchanNum);
}
