#include "TCal.h"

ClassImp(TCal)

TCal::TCal(){
   InitTCal();
}

TCal::TCal(const char* name, const char* title) {
   InitTCal();
   SetNameTitle(name,title);
   fgraph = new TGraphErrors;
   fgraph->SetNameTitle(name,title);
}

TCal::~TCal(){
   if(fgraph)
      delete fgraph;
   fgraph = 0;
}

void TCal::Clear(Option_t *opt) {
   this->fchanNum = 9999;
   fgraph->Clear();
}

void TCal::Print(Option_t *opt) const{
   printf("Channel Number: %u\n",fchanNum);
}

void TCal::InitTCal() {
   fgraph = new TGraphErrors;
   Clear();
}
