#include "TGRSIRunInfo.h"

#include <fstream>
#include <sstream>

#include <TGRSIOptions.h>

/// \cond CLASSIMP
ClassImp(TGRSIRunInfo)
   /// \endcond

   TGRSIRunInfo *TGRSIRunInfo::fGRSIRunInfo = new TGRSIRunInfo();  

   std::string TGRSIRunInfo::fGRSIVersion;

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

   void TGRSIRunInfo::Streamer(TBuffer &b) {
      //Streamer for TGRSIRunInfo. Allows us to write all of the run info
      //to disk like a string. This way there isn't any compatibility issues
      //between different TGRSIRunInfo classes written by different users.
      UInt_t R__s, R__c;
      if(b.IsReading()) {
         Version_t R__v = b.ReadVersion(&R__s,&R__c); if (R__v) { }
         TObject::Streamer(b);  
         {Int_t  R__int ; b >> R__int;  fRunNumber = R__int;}
         {Int_t  R__int ; b >> R__int;  fSubRunNumber = R__int;}
         if(R__v>3) {
            {Double_t  R__double ; b >> R__double;  fRunStart = R__double;}
            {Double_t  R__double ; b >> R__double;  fRunStop  = R__double;}
         }
         if(R__v>4) {
            {Double_t  R__double ; b >> R__double;  fRunLength = R__double;}
         }
         if(R__v>2) {
            {Int_t  R__int ; b >> R__int;  fHPGeArrayPosition = R__int;}
            if(R__v>5) {
               {Long_t  R__int ; b >> R__int;  fBuildWindow = R__int;}
            } else {
               {Int_t  R__int ; b >> R__int;  fBuildWindow = R__int;}
            }
            {Double_t  R__double ; b >> R__double;  fAddBackWindow = R__double;}
         }
         if(R__v>4) {
            {Bool_t R__bool; b >> R__bool; fIsMovingWindow = R__bool;}
         }
         if(R__v>6) {
            {Bool_t R__bool; b >> R__bool; fWaveformFitting = R__bool;}
         }
         {Bool_t R__bool; b >> R__bool; fTigress = R__bool;   }
         {Bool_t R__bool; b >> R__bool; fSharc = R__bool;     }
         {Bool_t R__bool; b >> R__bool; fTriFoil = R__bool;   }
         {Bool_t R__bool; b >> R__bool; fRf = R__bool;        }
         {Bool_t R__bool; b >> R__bool; fCSM = R__bool;       }
         {Bool_t R__bool; b >> R__bool; fSpice = R__bool;     }
         {Bool_t R__bool; b >> R__bool; fTip = R__bool;       }
         {Bool_t R__bool; b >> R__bool; fS3 = R__bool;        }
         {Bool_t R__bool; b >> R__bool; fBambino = R__bool;		}

         {Bool_t R__bool; b >> R__bool; fGriffin = R__bool;   }
         {Bool_t R__bool; b >> R__bool; fSceptar = R__bool;   }
         {Bool_t R__bool; b >> R__bool; fPaces = R__bool;     }
         {Bool_t R__bool; b >> R__bool; fDante = R__bool;     }
         {Bool_t R__bool; b >> R__bool; fZeroDegree = R__bool;}
         {Bool_t R__bool; b >> R__bool; fDescant = R__bool;   }
         {TString R__str; R__str.Streamer(b); fMajorIndex.assign(R__str.Data()); } 
         //printf("fMajorIndex = %s\n",fMajorIndex.c_str());
         {TString R__str; R__str.Streamer(b); fMinorIndex.assign(R__str.Data()); }
         //printf("fMinorIndex = %s\n",fMinorIndex.c_str());
         if(R__v >2) {
            {TString R__str; R__str.Streamer(b); fRunInfoFileName.assign(R__str.Data()); }
            //printf("fRunInfoFileNameMajor = %s\n",fRunInfoFileName.c_str());
            {TString R__str; R__str.Streamer(b); fRunInfoFile.assign(R__str.Data()); }
            //printf("fMajorIndex = %s\n",fMajorIndex.c_str());
         }
         if(R__v > 8){
            {Bool_t R__bool; b >> R__bool; fDescantAncillary = R__bool;   }
         }
         fGRSIRunInfo = this;
         b.CheckByteCount(R__s,R__c,TGRSIRunInfo::IsA());
      } else {
         R__c = b.WriteVersion(TGRSIRunInfo::IsA(),true);
         TObject::Streamer(b);  
         {Int_t R__int = fRunNumber;    b << R__int;}
         {Int_t R__int = fSubRunNumber; b << R__int;}
         {Double_t R__double = fRunStart;  b << R__double;}
         {Double_t R__double = fRunStop ;  b << R__double;}
         {Double_t R__double = fRunLength ;  b << R__double;}
         {Int_t R__int = fHPGeArrayPosition; b << R__int;}
         {Long_t R__long = fBuildWindow;       b << R__long;}
         {Double_t R__double = fAddBackWindow;  b << R__double;}
         {Bool_t R__bool = fIsMovingWindow; b << R__bool;}
         {Bool_t R__bool = fWaveformFitting; b << R__bool;}
         {Bool_t R__bool = fTigress;    b << R__bool;}
         {Bool_t R__bool = fSharc;      b << R__bool;}
         {Bool_t R__bool = fTriFoil;    b << R__bool;}
         {Bool_t R__bool = fRf;         b << R__bool;}
         {Bool_t R__bool = fCSM;        b << R__bool;}
         {Bool_t R__bool = fSpice;      b << R__bool;}
         {Bool_t R__bool = fTip;        b << R__bool;}
         {Bool_t R__bool = fS3;         b << R__bool;}
         {Bool_t R__bool = fBambino;	  b << R__bool;}

         {Bool_t R__bool = fGriffin;    b << R__bool;}
         {Bool_t R__bool = fSceptar;    b << R__bool;}
         {Bool_t R__bool = fPaces;      b << R__bool;}
         {Bool_t R__bool = fDante;      b << R__bool;}
         {Bool_t R__bool = fZeroDegree; b << R__bool;}
         {Bool_t R__bool = fDescant;    b << R__bool;}
         //printf("fMajorIndex = %s\n",fMajorIndex.c_str());
         //printf("fMinorIndex = %s\n",fMinorIndex.c_str());
         {TString R__str(fMajorIndex.c_str());      R__str.Streamer(b);   }//printf("TString::data = %s\n",R__str.Data());  }//; R__str = fMajorIndex.c_str();      R__str.Streamer(b);}
{TString R__str(fMinorIndex.c_str());      R__str.Streamer(b);   }//printf("TString::data = %s\n",R__str.Data()); }//; R__str = fMinorIndex.c_str();      R__str.Streamer(b);}
{TString R__str(fRunInfoFileName.c_str()); R__str.Streamer(b);   }//; R__str = fRunInfoFileName.c_str(); R__str.Streamer(b);}
{TString R__str(fRunInfoFile.c_str());     R__str.Streamer(b);   }//; R__str = fRunInfoFile.c_str();     R__str.Streamer(b);}
{Bool_t R__bool = fDescantAncillary;    b << R__bool;}
b.SetByteCount(R__c,true);
}
}


TGRSIRunInfo *TGRSIRunInfo::Get() {
   //The Getter for the singleton TGRSIRunInfo. This makes it
   //so there is only even one instance of the run info during
   //a session and it can be accessed from anywhere during that
   //session.
   if(!fGRSIRunInfo)
      fGRSIRunInfo = new TGRSIRunInfo();
   return fGRSIRunInfo;
}

void TGRSIRunInfo::SetRunInfo(TGRSIRunInfo *tmp) {
   //Sets the TGRSIRunInfo to the info passes as tmp.
   if(fGRSIRunInfo)
      delete fGRSIRunInfo;
   fGRSIRunInfo = tmp;
}

Bool_t TGRSIRunInfo::ReadInfoFromFile(TFile *tempf){

   TDirectory *savdir = gDirectory;
   if(tempf)
      tempf->cd();

   if (!(gDirectory->GetFile())){
      printf("File does not exist\n");
      savdir->cd();
      return false;
   }

   tempf = gDirectory->GetFile();

   TList *list =  tempf->GetListOfKeys();
   TIter iter(list);

   while(TKey *key = static_cast<TKey*>(iter.Next())) {
      if(!key || strcmp(key->GetClassName(),"TGRSIRunInfo"))
         continue;
      TGRSIRunInfo::SetRunInfo(static_cast<TGRSIRunInfo*>(key->ReadObj()));
      savdir->cd();
      return true;
   }
   savdir->cd();

   return false;
}


TGRSIRunInfo::TGRSIRunInfo() : fRunNumber(0),fSubRunNumber(-1) { 
   ///Default ctor for TGRSIRunInfo. The default values are:
   ///
   ///fHPGeArrayPosition = 110.0;
   ///fBuildWindow       = 200;  
   ///fAddBackWindow     = 15.0;
   ///fIsMovingWindow    = true;
   ///fWaveformFitting	 = false;
   ///fBufferSize        = 1000000;
   ///fBufferDuration    = 60000000000;

   fHPGeArrayPosition = 110.0;
   fBuildWindow       = 200;  
   fAddBackWindow     = 15.0;
   fIsMovingWindow    = true;
   fWaveformFitting	 = false;
   fBufferSize        = 1000000;
   fBufferDuration    = 60000000000;

   fDescantAncillary = false;

   //printf("run info created.\n");

   Clear();
}

TGRSIRunInfo::~TGRSIRunInfo() { }

void TGRSIRunInfo::Print(Option_t *opt) const {
   //Prints the TGRSIRunInfo. Options:
   // a: Print out more details.
   if(strchr(opt,'a') != NULL){
      printf("\tTGRSIRunInfo Status:\n");
      printf("\t\tRunNumber:    %05i\n",TGRSIRunInfo::Get()->fRunNumber);
      printf("\t\tSubRunNumber: %03i\n",TGRSIRunInfo::Get()->fSubRunNumber);
      printf("\t\tRunStart:     %.0f\n",TGRSIRunInfo::Get()->fRunStart);
      printf("\t\tRunStop:      %.0f\n",TGRSIRunInfo::Get()->fRunStop);
      printf("\t\tRunLength:    %.0f\n",TGRSIRunInfo::Get()->fRunLength);
      printf("\t\tTIGRESS:      %s\n", Tigress() ? "true" : "false");
      printf("\t\tSHARC:        %s\n", Sharc() ? "true" : "false");
      printf("\t\tTRIFOIL:      %s\n", TriFoil() ? "true" : "false");
      printf("\t\tTIP:          %s\n", Tip() ? "true" : "false");
      printf("\t\tCSM:          %s\n", CSM() ? "true" : "false");
      printf("\t\tSPICE:        %s\n", Spice() ? "true" : "false");
      printf("\t\tS3:           %s\n", S3() ? "true" : "false");
      printf("\t\tBAMBINO:      %s\n", Bambino() ? "true" : "false");
      printf("\t\tRF:           %s\n", RF() ? "true" : "false");
      printf("\t\tGRIFFIN:      %s\n", Griffin() ? "true" : "false");
      printf("\t\tSCEPTAR:      %s\n", Sceptar() ? "true" : "false");
      printf("\t\tPACES:        %s\n", Paces() ? "true" : "false");
      printf("\t\tDESCANT:      %s\n", Descant() ? "true" : "false");
      printf("\t\tZDS:          %s\n", ZeroDegree() ? "true" : "false");
      printf("\n");
      printf(DBLUE"\tBuild Window   = " DRED "%lu"   RESET_COLOR "\n",TGRSIRunInfo::BuildWindow());
      printf(DBLUE"\tMoving Window  = " DRED "%s"    RESET_COLOR "\n",TGRSIRunInfo::IsMovingWindow() ? "TRUE" : "FALSE");
      printf(DBLUE"\tAddBack Window = " DRED "%.01f" RESET_COLOR "\n",TGRSIRunInfo::AddBackWindow());
      printf(DBLUE"\tArray Position = " DRED "%i"    RESET_COLOR "\n",TGRSIRunInfo::HPGeArrayPosition());
      printf(DBLUE"\tWaveform fitting = " DRED "%s"  RESET_COLOR "\n",TGRSIRunInfo::IsWaveformFitting() ? "TRUE" : "FALSE");
      printf(DBLUE"\tDESCANT in ancillary positions = " DRED "%s"  RESET_COLOR "\n",TGRSIRunInfo::DescantAncillary() ? "TRUE" : "FALSE");
      printf("\n");
      printf("\t==============================\n");
   }
   else{
      printf("\t\tRunNumber:    %05i\t",TGRSIRunInfo::Get()->fRunNumber);
      printf("\t\tSubRunNumber: %03i\n",TGRSIRunInfo::Get()->fSubRunNumber);
   }
}

void TGRSIRunInfo::Clear(Option_t *opt) {
   //Clears the TGRSIRunInfo. Currently, there are no available
   //options.

   fTigress = false;
   fSharc = false;
   fTriFoil = false;
   fRf = false;
   fCSM = false;
   fSpice = false;
   fTip = false;
   fS3 = false;
   fBambino = false;

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
   //Sets the run info. This figures out what systems are available.

   printf("In runinfo, found %i channels.\n",TChannel::GetNumberOfChannels());

   if(runnum != 0)
      SetRunNumber(runnum);
   if(subrunnum != -1) 
      SetSubRunNumber(subrunnum);

   std::map<unsigned int,TChannel*>::iterator iter; 

   for(iter = TChannel::GetChannelMap()->begin();iter != TChannel::GetChannelMap()->end(); iter++) {
      std::string channelname = iter->second->GetChannelName();
      MNEMONIC mnemonic;
      ParseMNEMONIC(&channelname,&mnemonic);

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
         if(!RF()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
         SetRF();
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
      } else if(system.compare("BA")==0) {
         if(!Bambino()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;}
         SetBambino();
      }
   }
   if(Tigress()) {
      Get()->fMajorIndex.assign("TriggerId");
      Get()->fMinorIndex.assign("FragmentId");
   } else if(Griffin()) {
      Get()->fMajorIndex.assign("TimeStampHigh");
      Get()->fMinorIndex.assign("TimeStampLow");
   }

   if(Get()->fRunInfoFile.length())
      ParseInputData(Get()->fRunInfoFile.c_str());

   //TGRSIRunInfo::Get()->Print();
}

void TGRSIRunInfo::SetAnalysisTreeBranches(TTree*) {
   //Currently does nothing.
}

Bool_t TGRSIRunInfo::ReadInfoFile(const char *filename) {
   //Read in a run info file. These files have the extension .info.
   //An example can be found in the "examples" directory.
   std::string infilename;
   infilename.append(filename);
   printf("Reading file: %s\n",filename);
   if(infilename.length()==0)
      return false;

   std::ifstream infile;
   infile.open(infilename.c_str());
   if (!infile) {
      printf("could not open file.\n");
      return false;
   }
   infile.seekg(0,std::ios::end);
   int length = infile.tellg();
   if(length<1) {
      printf("file is empty.\n");
      return false;
   }
   char* buffer = new char[length];
   infile.seekg(0,std::ios::beg);
   infile.read(buffer,length);

   Get()->SetRunInfoFileName(filename);
   Get()->SetRunInfoFile(buffer);

   return ParseInputData(const_cast<const char*>(buffer));
}

Bool_t TGRSIRunInfo::ParseInputData(const char *inputdata,Option_t *opt) {
   //A helper function to parse the run info file.

   std::istringstream infile(inputdata);
   std::string line;
   int linenumber = 0;

   //Parse the info file. 
   while(std::getline(infile, line)) {
      linenumber++;
      trim(&line);
      size_t comment = line.find("//");
      if(comment != std::string::npos) {
         line = line.substr(0, comment);
      }
      if(!line.length())
         continue;

      size_t ntype = line.find(":");
      if(ntype == std::string::npos) //no seperator, not useful.
         continue;

      std::string type = line.substr(0, ntype);
      line = line.substr(ntype + 1, line.length());
      trim(&line);
      int j = 0;
      while (type[j]) {
         char c = *(type.c_str() + j);
         c = toupper(c);
         type[j++] = c;
      }
      if( type.compare("BW")==0 || type.compare("BUILDWINDOW")==0 ) {     
         std::istringstream ss(line);
         long int temp_bw; ss >> temp_bw;
         Get()->SetBuildWindow(temp_bw);
      } else if( type.compare("WF")==0 || type.compare("WAVEFORMFIT")==0) {
         std::istringstream ss(line);
         bool temp_wff; ss >> temp_wff;
         Get()->SetWaveformFitting(temp_wff);
      } else if( type.compare("MW")==0 || type.compare("MOVINGWINDOW")==0) {
         std::istringstream ss(line);
         bool temp_mw; ss >> temp_mw;
         Get()->SetMovingWindow(temp_mw);
      } else if( type.compare("ABW")==0 || type.compare("ADDBACKWINDOW")==0 || type.compare("ADDBACK")==0 ) {
         std::istringstream ss(line);
         double temp_abw; ss >> temp_abw;
         Get()->SetAddBackWindow(temp_abw);
      } else if( type.compare("CAL")==0 || type.compare("CALFILE")==0 ) {
         TGRSIOptions::AddInputCalFile(line);
      } else if( type.compare("MID")==0 || type.compare("MIDAS")==0 || type.compare("MIDASFILE")==0 ) {
         TGRSIOptions::AddInputMidasFile(line);
      } else if( type.compare("ARRAYPOS")==0 || type.compare("HPGEPOS")==0) {
         std::istringstream ss(line);
         double temp_int; ss >> temp_int;
         Get()->SetHPGeArrayPosition(temp_int);
      } else if( type.compare("DESCANTANCILLARY") == 0) {
         std::istringstream ss(line);
         int temp_int; ss >> temp_int;
         Get()->SetDescantAncillary(temp_int);
      }
   }

   if(strcmp(opt,"q")) {
      printf("parsed %i lines.\n",linenumber);
      printf(DBLUE"\tBuild Window   = " DRED "%lu"   RESET_COLOR "\n",TGRSIRunInfo::BuildWindow());
      printf(DBLUE"\tMoving Window  = " DRED "%s"    RESET_COLOR "\n",TGRSIRunInfo::IsMovingWindow() ? "TRUE" : "FALSE");
      printf(DBLUE"\tAddBack Window = " DRED "%.01f" RESET_COLOR "\n",TGRSIRunInfo::AddBackWindow());
      printf(DBLUE"\tArray Position = " DRED "%i"    RESET_COLOR "\n",TGRSIRunInfo::HPGeArrayPosition());
      printf(DBLUE"\tWaveform Fitting  = " DRED "%s"    RESET_COLOR "\n",TGRSIRunInfo::IsWaveformFitting() ? "TRUE" : "FALSE");
   }
   return true;
}


void TGRSIRunInfo::trim(std::string * line, const std::string & trimChars) {
   //Removes the the string "trimCars" from  the string 'line'
   if (line->length() == 0)
      return;
   std::size_t found = line->find_first_not_of(trimChars);
   if (found != std::string::npos)
      *line = line->substr(found, line->length());
   found = line->find_last_not_of(trimChars);
   if (found != std::string::npos)
      *line = line->substr(0, found + 1);
   return;
}

Long64_t TGRSIRunInfo::Merge(TCollection *list){
   //Loop through the TCollection of TGRSISortLists, and add each entry to the original TGRSISort List
   TIter it(list);
   //The TCollection will be filled by something like hadd. Each element in the list will be a TGRSISortList from
   //An individual file that was submitted to hadd.
   TGRSIRunInfo *runinfo = 0;

   while ((runinfo = static_cast<TGRSIRunInfo*>(it.Next())) != NULL){
      //Now we want to loop through each TGRSISortList and find the TGRSISortInfo's stored in there.    
      this->Add(runinfo);
   }
   return 0;
}
