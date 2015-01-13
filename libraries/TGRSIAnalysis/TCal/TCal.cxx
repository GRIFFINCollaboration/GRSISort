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

Bool_t TCal::SetChannel(const TChannel* chan){
   if(!chan){
      Error("SetChannel","TChannel does not exist");
      return false;
   }
   fchan = (TChannel*)chan;
   return true;
}

Bool_t TCal::SetChannel(UInt_t channum){
   TChannel *chan = TChannel::GetChannelByNumber(channum);
   if(!chan){
      Error("SetChannel","Channel Number %d does not exist in current memory.",channum);
      return false;
   }
   else
      return SetChannel(chan);
}

TChannel* const TCal::GetChannel() const {
   return fchan;
}

void TCal::Clear(Option_t *opt) {
   fchan = 0;
   fgraph->Clear();
}

void TCal::Print(Option_t *opt) const{
   if(fchan)
      printf("Channel Number: %u\n",fchan->GetNumber());
   else
      printf("Channel Number: NOT SET\n");
}

void TCal::InitTCal() {
   fgraph = new TGraphErrors;
   ffitfunc = 0;
   fchan = 0;
   Clear();
}
