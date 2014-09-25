
#include "TGRSIRunInfo.h"

ClassImp(TGRSIRunInfo)

TGRSIRunInfo *TGRSIRunInfo::fGRSIRunInfo = new TGRSIRunInfo();  

//int  TGRSIRunInfo::fRunNumber    = 0;
//int  TGRSIRunInfo::fSubRunNumber = -1;

//bool TGRSIRunInfo:: fTigress     = false;
//bool TGRSIRunInfo:: fSharc       = false;
//bool TGRSIRunInfo:: fTriFoil     = false;
//bool TGRSIRunInfo:: fRf          = false;
//bool TGRSIRunInfo:: fCSM         = false;
//bool TGRSIRunInfo:: fSpice       = false;
//bool TGRSIRunInfo:: fTip         = false;
//bool TGRSIRunInfo:: fS3          = false;

//bool TGRSIRunInfo:: fGriffin     = false;
//bool TGRSIRunInfo:: fSceptar     = false;
//bool TGRSIRunInfo:: fPaces       = false;
//bool TGRSIRunInfo:: fDante       = false;
//bool TGRSIRunInfo:: fZeroDegree  = false;
//bool TGRSIRunInfo:: fDescant     = false;

//std::string TGRSIRunInfo::fMajorIndex;
//std::string TGRSIRunInfo::fMinorIndex;

//int TGRSIRunInfo::fNumberOfTrueSystems = 0;

TGRSIRunInfo *TGRSIRunInfo::Get() {
   if(!fGRSIRunInfo)
      fGRSIRunInfo = new TGRSIRunInfo();
   return fGRSIRunInfo;
}

void TGRSIRunInfo::SetInfoFromFile(TGRSIRunInfo *tmp) {
   if(fGRSIRunInfo)
      delete fGRSIRunInfo;
   fGRSIRunInfo = tmp;
}


TGRSIRunInfo::TGRSIRunInfo() { 
   //if(fNumberOfTrueSystems>0)
   //   TGRSIRunInfo::Get() = this;
   //else 
   Clear(); 
}

TGRSIRunInfo::~TGRSIRunInfo() { }

void TGRSIRunInfo::Print(Option_t *opt) {
   printf("\tTGRSIRunInfo Status:\n");
   printf("\t\tRunNumber:    %05i\n",TGRSIRunInfo::Get()->fRunNumber);
   printf("\t\tSubRunNumber: %03i\n",TGRSIRunInfo::Get()->fSubRunNumber);
   printf("\t\tTIGRESS:      %s\n", Tigress() ? "true" : "false");
   printf("\t\tSHARC:        %s\n", Sharc() ? "true" : "false");
   printf("\t\tCSM:          %s\n", CSM() ? "true" : "false");
   printf("\t\tGRIFFIN:      %s\n", Griffin() ? "true" : "false");
   printf("\t\tSCEPTAR:      %s\n", Sceptar() ? "true" : "false");
   printf("\t=====================\n");
}

void TGRSIRunInfo::Clear(Option_t *opt) {
   
   fTigress = false;
   fSharc = false;
   fTriFoil = false;
   fRf = false;
   fCSM = false;
   fSpice = false;
   fTip = false;
   fS3 = false;
   
   fGriffin = false;
   fSceptar = false;
   fPaces = false;
   fDante = false;
   fZeroDegree = false;
   fDescant = false;

   fMajorIndex.assign("");  
   fMinorIndex.assign("");  

   fNumberOfTrueSystems = 0;
}


void TGRSIRunInfo::SetRunInfo(int runnum, int subrunnum) {

   printf("In runinfo, found %i channels.\n",TChannel::GetNumberOfChannels());

   if(runnum != 0)
      SetRunNumber(runnum);
   if(subrunnum != -1) 
      SetSubRunNumber(subrunnum);

   int counter = 0;
   std::map<unsigned int,TChannel*>::iterator iter; 
   
   for(iter = TChannel::GetChannelMap()->begin();iter != TChannel::GetChannelMap()->end(); iter++) {
      std::string channelname = iter->second->GetChannelName();
      MNEMONIC mnemonic;
      ParseMNEMONIC(&channelname,&mnemonic);
     //printf("\t%-04i:\t%s\n",counter++,channelname.c_str());

      //  detector system type.
      //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature
      std::string system = mnemonic.system;
      if(system.compare("TI")==0) {
         //printf("this is working,found tigress.\n");
         if(!Tigress()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetTigress(); 
      } else if(system.compare("SH")==0) {
         //printf("this is working,found sharc.\n");
         if(!Sharc()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetSharc();
      } else if(system.compare("Tr")==0) {
         if(!TriFoil()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetTriFoil();
      } else if(system.compare("RF")==0) {
         if(!Rf()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetRf();
      } else if(system.compare("CS")==0) {
         if(!CSM()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetCSM();
      } else if(system.compare("SP")==0) {
         if(!Spice()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetSpice();
         if(!S3()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;}
         SetS3();
      } else if(system.compare("TP")==0) {
         if(!Tip()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetTip();
      } else if(system.compare("GR")==0) {
         if(!Griffin()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetGriffin();
      } else if(system.compare("SE")==0) {
         if(!Sceptar()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetSceptar();
      } else if(system.compare("PA")==0) {
         if(!Paces()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetPaces();
      } else if(system.compare("DA")==0) {
         if(!Dante()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetDante();
      } else if(system.compare("ZD")==0) {
         if(!ZeroDegree()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetZeroDegree();
      } else if(system.compare("DS")==0) {
         if(!Descant()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetDescant();
      }
   }
   //TGRSIRunInfo::Get()->Print();
}


void TGRSIRunInfo::SetAnalysisTreeBranches(TTree*) {



}





