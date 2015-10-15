#include "TCal.h"

ClassImp(TCal)
////////////////////////////////////////////////////////////////
//                                                            //
// TCal                                                       //
//                                                            //
// This is an abstract class that contains the basic info     //
// about a calibration. Calibrations here are TGraphErrors
// that are fit, with the resulting fit function being the 
// calibrating function.
//                                                            //
////////////////////////////////////////////////////////////////

TCal::TCal(){
   //Default constructor
   InitTCal();
}

TCal::TCal(const char* name, const char* title) {
   //Constructor for naming the calibration
   InitTCal();
   SetNameTitle(name,title);
   //fgraph->SetNameTitle(name,title);
}

TCal::~TCal(){
   //Default dtor
   fnuc = 0;
   //fgraph = 0;
   fhist = 0;
}

TCal::TCal(const TCal &copy) : TGraphErrors(copy){
   //Copy constructor
   InitTCal();
   ((TCal&)copy).Copy(*this);
}

/*TGraphErrors TCal::MergeGraphs(TCal *cal,...){
   //Probably does not work properly yet
   TGraphErrors ge;
   TList* glist;
   TClass *cl = cal->Class();
   if(!(cal->IsGroupable())){
      cal->Error("MergeGraphs","%s is not groupable",cal->ClassName());
      return ge;
   }
   va_list ap;
   va_start(ap, cal);
   while(cal){
      if(cal->Class() != cl){
         cal->Error("MergeGraphs","Trying to merge a %s to a %s",cal->ClassName(),cl->ClassName());
         return ge;
      }
      glist.Add(cal->Graph());
      cal= var_arg(ap, TCal*);
   }
   va_end(ap);
   ge.Merge(glist);
   return ge;
}*/

void TCal::SetNucleus(TNucleus* nuc,Option_t * opt){
   //Sets the nucleus to be calibrated against
   if(!nuc){
      Error("SetNucleus","Nucleus does not exist");
      return;
   }
   if(fnuc)
      Warning("SetNucleus","Overwriting nucleus: %s",fnuc->GetName());
   fnuc = nuc;
}

void TCal::Copy(TObject &obj) const{
   //Copies the TCal.
   ((TCal&)obj).fchan = fchan;
   //Things to make deep copies of
   if(ffitfunc)   *(((TCal&)obj).ffitfunc)   =  *ffitfunc;

   //Members to make shallow copies of
                    ((TCal&)obj).fnuc        =  fnuc;
   TNamed::Copy((TGraphErrors&)obj);
}

Bool_t TCal::SetChannel(const TChannel* chan){
   //Sets the channel being calibrated
   if(!chan){
      Error("SetChannel","TChannel does not exist");
      printf("%p\n", (void*) chan);
      return false;
   }
   //Set our TRef to point at the TChannel
   fchan = (TChannel*)chan;
   return true;
}

void TCal::WriteToAllChannels(std::string mnemonic){
   //Writes this calibration to all channels in the current TChannel Map
   std::map<unsigned int,TChannel*>::iterator mapit;
   std::map<unsigned int,TChannel*> *chanmap = TChannel::GetChannelMap();
   TChannel* orig_chan = GetChannel();
   for(mapit = chanmap->begin(); mapit != chanmap->end(); mapit++){
      if(!mnemonic.size() || !strncmp(mapit->second->GetChannelName(),mnemonic.c_str(),mnemonic.size())){
         SetChannel(mapit->second);
         WriteToChannel();
      }
   }
   printf("\n");
   if(orig_chan)
      SetChannel(orig_chan);
}

std::vector<Double_t> TCal::GetParameters() const{
   //Returns all of the parameters in the current TCal.
   std::vector<Double_t> paramlist;
   if(!GetFitFunction()){
      Error("GetParameters","Function has not been fitted yet");
      return paramlist;
   }
   
   Int_t nparams = GetFitFunction()->GetNpar();

   for(int i=0;i<nparams;i++)
      paramlist.push_back(GetParameter(i));

   return paramlist;
}

Double_t TCal::GetParameter(Int_t parameter) const{
   //Returns the parameter at the index parameter
   if(!GetFitFunction()){
      Error("GetParameter","Function have not been fitted yet");
      return 0;
   }
   return GetFitFunction()->GetParameter(parameter); //Root does all of the checking for us.
}

Bool_t TCal::SetChannel(UInt_t channum){
   //Sets the channel for the calibration to the channel number channum. Returns 
   //0 if the channel does not exist
   TChannel *chan = TChannel::GetChannelByNumber(channum);
   if(!chan){
      Error("SetChannel","Channel Number %d does not exist in current memory.",channum);
      return false;
   }
   else
      return SetChannel(chan);
}

TChannel* TCal::GetChannel() const {
   //Gets the channel being pointed to by the TCal. Returns 0 if no channel
   //is set.
   return (TChannel*)(fchan.GetObject()); //Gets the object pointed at by the TRef and casts it to a TChannel
}

void TCal::SetHist(TH1* hist) {
//Sets this histogram pointed to. TCal does NOT take ownership so you cannot delete this
//histogram as long as you want to access the hist in the TCal/write it out. I will add this
//functionality if I get annoyed enough with the way it is.
   fhist = hist;

}   

void TCal::Clear(Option_t *opt) {
   //Clears the calibration. Does not delete nuclei or channels.
   fnuc = 0;
   fchan = 0;
   TGraphErrors::Clear();
}
/*
void TCal::Draw(Option_t* chopt){
   Draw(chopt);
}
*/
void TCal::Print(Option_t *opt) const{
   //Prints calibration information
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

   printf("Nucleus: ");
   if(GetNucleus())
      printf("%s\n",GetNucleus()->GetName());
   else
      printf("NOT SET\n");

}

void TCal::InitTCal() {
   //Initiallizes the TCal.
  /* fgraph = new TGraphErrors;*/
   ffitfunc = 0;
   fchan = 0;
   fnuc = 0;
   fhist = 0;
   Clear();
}
