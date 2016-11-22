#include "TGRSIRunInfo.h"
#include "TMnemonic.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

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
            {Double_t  R__double ; b >> R__double;  fHPGeArrayPosition = R__double;}
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
         if(R__v > 9){
            {Bool_t R__bool; b >> R__bool; fIsCorrectingCrossTalk = R__bool;   }
            {UInt_t R__uint; b >> R__uint; fBadCycleListSize = R__uint;   }
            for(UInt_t i =0; i< fBadCycleList.size(); ++i){
               Int_t R__int; b >> R__int; AddBadCycle(R__int);
            }
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
         {Double_t R__double = fHPGeArrayPosition; b << R__double;}
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
         {Bool_t R__bool = fIsCorrectingCrossTalk; b<< R__bool;   }
         {UInt_t R__uint = fBadCycleListSize; b << R__uint;   }
         for(UInt_t i =0; i< fBadCycleList.size(); ++i){
            Int_t R__int = fBadCycleList.at(i); b<<R__int;
         }
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
   if(fGRSIRunInfo && (tmp != fGRSIRunInfo))
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
   printf("Reading Info from file:" CYAN " %s" RESET_COLOR "\n",tempf->GetName());
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
   fBuildWindow       = 2000;
   fAddBackWindow     = 150.0;
   fIsMovingWindow    = true;
   fWaveformFitting	 = false;
   fBufferSize        = 1000000;
   fBufferDuration    = 60000000000;

   fDescantAncillary       = false;
   fIsCorrectingCrossTalk  = true;
   fBadCycleList.clear();
   fBadCycleListSize = 0;

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
      printf("\t\tDANTE:        %s\n", Dante() ? "true" : "false");
      printf("\n");
      printf(DBLUE"\tBuild Window (10 ns) = " DRED "%lu"   RESET_COLOR "\n",TGRSIRunInfo::BuildWindow());
      printf(DBLUE"\tMoving Window = " DRED "%s"    RESET_COLOR "\n",TGRSIRunInfo::IsMovingWindow() ? "TRUE" : "FALSE");
      printf(DBLUE"\tAddBack Window (ns) = " DRED "%.01f" RESET_COLOR "\n",TGRSIRunInfo::AddBackWindow());
      printf(DBLUE"\tArray Position (mm) = " DRED "%.01f"    RESET_COLOR "\n",TGRSIRunInfo::HPGeArrayPosition());
      printf(DBLUE"\tWaveform fitting = " DRED "%s"  RESET_COLOR "\n",TGRSIRunInfo::IsWaveformFitting() ? "TRUE" : "FALSE");
      printf(DBLUE"\tDESCANT in ancillary positions = " DRED "%s"  RESET_COLOR "\n",TGRSIRunInfo::DescantAncillary() ? "TRUE" : "FALSE");
      printf(DBLUE"\tGRIFFIN Corrected for Cross-talk = " DRED "%s"  RESET_COLOR "\n",TGRSIRunInfo::IsCorrectingCrossTalk() ? "TRUE" : "FALSE");
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
   fIsCorrectingCrossTalk  = true;
   fBadCycleList.clear();
   fBadCycleListSize = 0;

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
      std::string channelname = iter->second->GetName();

      //  detector system type.
      //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature
      switch(iter->second->GetMnemonic()->System()){
         case TMnemonic::kTigress:
            if(!Tigress()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetTigress(); 
            break;
         case TMnemonic::kSharc:
            if(!Sharc()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetSharc();
            break;
         case TMnemonic::kTriFoil:
            if(!TriFoil()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetTriFoil();
            break;
         case TMnemonic::kRF:
            if(!RF()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetRF();
            break;
         case TMnemonic::kCSM:
            if(!CSM()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetCSM();
            break;
         case TMnemonic::kTip:
            if(!Tip()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetTip();
            break;
         case TMnemonic::kGriffin:
            if(!Griffin()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetGriffin();
            break;
         case TMnemonic::kSceptar:
            if(!Sceptar()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetSceptar();
            break;
         case TMnemonic::kPaces:
            if(!Paces()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetPaces();
            break;
         case TMnemonic::kLaBr:
            if(!Dante()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetDante();
            break;
         case TMnemonic::kZeroDegree:
            if(!ZeroDegree()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetZeroDegree();
            break;
         case TMnemonic::kDescant:
            if(!Descant()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
            SetDescant();
            break;
         default:
            std::string system = iter->second->GetMnemonic()->SystemString();
            if(!Spice() && !S3()){
               if(system.compare("SP")==0) {
                  if(!Spice()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;} 
                  SetSpice();
                  if(!S3()) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;}
                  SetS3();
               }
            } else if (!Bambino()){
               if(system.compare("BA")==0) {TGRSIRunInfo::Get()->fNumberOfTrueSystems++;}
               SetBambino();
            }
      };
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

   //   TGRSIRunInfo::Get()->Print("a");
}

void TGRSIRunInfo::SetAnalysisTreeBranches(TTree*) {
   //Currently does nothing.
}

Bool_t TGRSIRunInfo::ReadInfoFile(const char *filename) {
   //Read in a run info file. These files have the extension .info.
   //An example can be found in the "examples" directory.
   std::string infilename;
   infilename.append(filename);
   printf("Reading info from file:" CYAN " %s" RESET_COLOR "\n",filename);
   if(infilename.length()==0){
      printf("Bad file name length\n");
      return false;
   }

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
        // TODO Make this work again, using priorities
        //TGRSIOptions::AddInputCalFile(line);
      } else if( type.compare("MID")==0 || type.compare("MIDAS")==0 || type.compare("MIDASFILE")==0 ) {
        // TODO Make this work again, using priorities
        // TGRSIOptions::AddInputMidasFile(line);
      } else if( type.compare("ARRAYPOS")==0 || type.compare("HPGEPOS")==0) {
         std::istringstream ss(line);
         double temp_double; ss >> temp_double;
         Get()->SetHPGeArrayPosition(temp_double);
      } else if( type.compare("DESCANTANCILLARY") == 0) {
         std::istringstream ss(line);
         int temp_int; ss >> temp_int;
         Get()->SetDescantAncillary(temp_int);
      } else if( type.compare("CROSSTALK") == 0) {
         std::istringstream ss(line);
         bool temp_ct; ss >> temp_ct;
         Get()->SetCorrectCrossTalk(temp_ct,"q");
      } else if(type.compare("BADCYCLE")==0) {
         std::istringstream ss(line);
			int tmp_int;
			while (ss >> tmp_int) {   Get()->AddBadCycle(tmp_int); }
      }
   }

   if(strcmp(opt,"q")) {
      printf("parsed %i lines.\n",linenumber);
      printf(DBLUE"\tBuild Window (10 ns) = " DRED "%lu"   RESET_COLOR "\n",TGRSIRunInfo::BuildWindow());
      printf(DBLUE"\tMoving Window = " DRED "%s"    RESET_COLOR "\n",TGRSIRunInfo::IsMovingWindow() ? "TRUE" : "FALSE");
      printf(DBLUE"\tAddBack Window (ns) = " DRED "%.01f" RESET_COLOR "\n",TGRSIRunInfo::AddBackWindow());
      printf(DBLUE"\tArray Position (mm) = " DRED "%lf"    RESET_COLOR "\n",TGRSIRunInfo::HPGeArrayPosition());
      printf(DBLUE"\tWaveform Fitting  = " DRED "%s"    RESET_COLOR "\n",TGRSIRunInfo::IsWaveformFitting() ? "TRUE" : "FALSE");
      printf(DBLUE"\tCorrecting Cross-talk  = " DRED "%s"    RESET_COLOR "\n",TGRSIRunInfo::IsCorrectingCrossTalk() ? "TRUE" : "FALSE");
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

void TGRSIRunInfo::SetCorrectCrossTalk(const bool flag, Option_t *opt) {
   fIsCorrectingCrossTalk = flag;
   TString opt1 = opt;
   opt1.ToUpper();
   if(opt1.Contains("Q")){
      return;
   }

      printf("Please call TGriffin::ResetFlags() on current event to avoid bugs\n");

}

void TGRSIRunInfo::PrintBadCycles() const {
   std::cout << "Bad Cycles:\t";
   if(!fBadCycleList.size()){
      std::cout << "NONE" << std::endl;
   }
   else{
      for(auto it = fBadCycleList.begin(); it != fBadCycleList.end(); ++it){
         std::cout << " " << *it;
      }
   std::cout << std::endl;
   }
}

void TGRSIRunInfo::AddBadCycle(int bad_cycle){
//   auto bad_cycle_it = std::binary_(fBadCycleList.begin(), fBadCycleList.end(),bad_cycle);
 /*  if(bad_cycle_it == fBadCycleList.end()){
      fBadCycleList.push_back(bad_cycle);
      std::sort(fBadCycleList.begin(), fBadCycleList.end());
   }*/
   if(!(std::binary_search(fBadCycleList.begin(), fBadCycleList.end(), bad_cycle))){
      fBadCycleList.push_back(bad_cycle);
      std::sort(fBadCycleList.begin(), fBadCycleList.end());
   }
   fBadCycleListSize = fBadCycleList.size();
}

void TGRSIRunInfo::RemoveBadCycle(int cycle){
   fBadCycleList.erase(std::remove(fBadCycleList.begin(), fBadCycleList.end(), cycle), fBadCycleList.end());
   std::sort(fBadCycleList.begin(), fBadCycleList.end());
   fBadCycleListSize = fBadCycleList.size();
}

bool TGRSIRunInfo::IsBadCycle(int cycle) const{
   return std::binary_search(fBadCycleList.begin(), fBadCycleList.end(), cycle);
}

bool TGRSIRunInfo::WriteToRoot(TFile* fileptr) {
  ///Writes Info File information to the tree
  //Maintain old gDirectory info
  bool bool2return = true;
  TDirectory* savdir = gDirectory;

  if(!fileptr)
    fileptr = gDirectory->GetFile();
  fileptr->cd();
  std::string oldoption = std::string(fileptr->GetOption());
  if(oldoption == "READ") {
    fileptr->ReOpen("UPDATE");
  }
  if(!gDirectory){
    printf("No file opened to write to.\n");
    bool2return = false;
  }
  else{
    Get()->Write();
  }

  printf("Writing Run Information to %s\n",gDirectory->GetFile()->GetName());
  if(oldoption == "READ") {
    printf("  Returning %s to \"%s\" mode.\n",gDirectory->GetFile()->GetName(),oldoption.c_str());
    fileptr->ReOpen("READ");
  }
  savdir->cd();//Go back to original gDirectory

  return bool2return;
}

bool TGRSIRunInfo::WriteInfoFile(std::string filename) {

	if(filename.length()>0) {
		std::ofstream infoout;
		infoout.open(filename.c_str());
		std::string infostr = Get()->PrintToString();
		infoout << infostr.c_str();
		infoout << std::endl;
	   infoout << std::endl;
	   infoout.close();
	} else {
      printf("Please enter a file name\n");
      return false;
	}

   return true;
}

std::string TGRSIRunInfo::PrintToString(Option_t* opt) {
	std::string buffer;
	buffer.append("//The event building time, 10 ns units.\n");
   buffer.append(Form("BuildWindow: %ld\n", Get()->BuildWindow()));
   buffer.append("\n\n");
	buffer.append("//The Addback event window, ns units.\n");
   buffer.append(Form("AddBackWindow: %lf\n", Get()->AddBackWindow()));
   buffer.append("\n\n");
	buffer.append("//The Array Position in mm.\n");
   buffer.append(Form("HPGePos: %lf\n", Get()->HPGeArrayPosition()));
   buffer.append("\n\n");
   if(Get()->IsWaveformFitting()){
	   buffer.append("//Waveforms being Fit.\n");
      buffer.append(Form("WaveFormFit: %d\n", 1));
      buffer.append("\n\n");
   }
   if(!(Get()->IsMovingWindow())){
	   buffer.append("//Using a moving BuildWindow.\n");
      buffer.append(Form("MovingWindow: %d\n", 0));
      buffer.append("\n\n");
   }
   if(Get()->DescantAncillary()){
	   buffer.append("//Is DESCANT in Ancillary positions?.\n");
      buffer.append(Form("DescantAncillary: %d\n", 1));
      buffer.append("\n\n");
   }
	buffer.append("//Correcting for Cross Talk? (Only available in GRIFFIN).\n");
   buffer.append(Form("CrossTalk: %d\n", Get()->IsCorrectingCrossTalk()));
   buffer.append("\n\n");
   if(fBadCycleList.size()){
	   buffer.append("//A List of bad cycles.\n");
      buffer.append("BadCycle:");
      for(auto it = fBadCycleList.begin(); it != fBadCycleList.end(); ++it){
         buffer.append(Form(" %d",*it));
      }
      buffer.append("\n\n");
   }

	return buffer;
}

