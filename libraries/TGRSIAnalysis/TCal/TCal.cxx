#include "TCal.h"

ClassImp(TCal)

TCal::TCal(){
   InitTCal();
}

TCal::TCal(const char* name, const char* title) {
   InitTCal();
   SetNameTitle(name,title);
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
   //Set our TRef to point at the TChannel
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
   return (TChannel*)(fchan.GetObject()); //Gets the object pointed at by the TRef and casts it to a TChannel
}

void TCal::Clear(Option_t *opt) {
   fchan = 0;
   fgraph->Clear();
}

void TCal::Print(Option_t *opt) const{
   if(GetChannel())
      printf("Channel Number: %u\n",GetChannel()->GetNumber());
   else
      printf("Channel Number: NOT SET\n");


   if(ffitfunc){
   printf("\n*******************************\n");
   printf(" Fit:\n");      
      ffitfunc->Print();
   printf("\n*******************************\n");   
   }
   else
      printf("Parameters: FIT NOT SET\n");

}

void TCal::InitTCal() {
   fgraph = new TGraphErrors;
   ffitfunc = 0;
   fchan = 0;
   Clear();
}
