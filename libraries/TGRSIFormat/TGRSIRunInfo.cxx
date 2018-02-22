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

//TGRSIRunInfo* TGRSIRunInfo::fGRSIRunInfo = new TGRSIRunInfo();

std::string TGRSIRunInfo::fGRSIVersion;

//TGRSIRunInfo* TGRSIRunInfo::Get()
//{
//   // The Getter for the singleton TGRSIRunInfo. This makes it
//   // so there is only even one instance of the run info during
//   // a session and it can be accessed from anywhere during that
//   // session.
//   if(fGRSIRunInfo == nullptr) {
//      fGRSIRunInfo = new TGRSIRunInfo();
//   }
//   return fGRSIRunInfo;
//}

//void TGRSIRunInfo::Set(TSingleton* tmp)
//{
//   // Sets the TGRSIRunInfo to the info passes as tmp.
//   if((fGRSIRunInfo != nullptr) && (tmp != fGRSIRunInfo)) {
//      delete fGRSIRunInfo;
//   }
//   fGRSIRunInfo = tmp;
//}

Bool_t TGRSIRunInfo::ReadInfoFromFile(TFile* tempf)
{
   TDirectory* savdir = gDirectory;
   if(tempf != nullptr) {
      tempf->cd();
   }

   if((gDirectory->GetFile()) == nullptr) {
      printf("File does not exist\n");
      savdir->cd();
      return false;
   }

   tempf = gDirectory->GetFile();

   TList* list = tempf->GetListOfKeys();
   TIter  iter(list);
	std::cout<<R"(Reading run info from file ")"<<CYAN<<tempf->GetName()<<RESET_COLOR<<R"(")"<<std::endl;
   while(TKey* key = static_cast<TKey*>(iter.Next())) {
      if((key == nullptr) || (strcmp(key->GetClassName(), "TGRSIRunInfo") != 0)) {
         continue;
      }

      Set(*static_cast<TGRSIRunInfo*>(key->ReadObj()));
		std::cout<<"read from file "<<&Get()<<":"<<std::endl;
		Get().Print();
      savdir->cd();
      return true;
   }
   savdir->cd();

   return false;
}

TGRSIRunInfo::TGRSIRunInfo() : fRunNumber(0), fSubRunNumber(-1)
{
   /// Default ctor for TGRSIRunInfo. The default values are:
   ///
   /// fHPGeArrayPosition = 110.0;

   Clear();
}

TGRSIRunInfo::~TGRSIRunInfo() = default;

void TGRSIRunInfo::Print(Option_t* opt) const
{
   // Prints the TGRSIRunInfo. Options:
   // a: Print out more details.
   std::cout<<"Title: "<<RunTitle()<<std::endl;
   std::cout<<"Comment: "<<RunComment()<<std::endl;
	time_t tmpStart = static_cast<time_t>(RunStart());
	time_t tmpStop  = static_cast<time_t>(RunStop());
	struct tm runStart = *localtime(const_cast<const time_t*>(&tmpStart));
	struct tm runStop  = *localtime(const_cast<const time_t*>(&tmpStop));
	printf("\t\tRunNumber:          %05i\n", RunNumber());
	printf("\t\tSubRunNumber:       %03i\n", SubRunNumber());
	if(Get().RunStart != 0 && Get().RunStop != 0) {
		printf("\t\tRunStart:           %s", asctime(&runStart));
		printf("\t\tRunStop:            %s", asctime(&runStop));
		printf("\t\tRunLength:          %.0f s\n", RunLength());
	} else {
		printf("\t\tCombined RunLength: %.0f s\n", RunLength());
	}
   if(strchr(opt, 'a') != nullptr) {
      printf("\t\tTIGRESS:            %s\n", Tigress() ? "true" : "false");
      printf("\t\tSHARC:              %s\n", Sharc() ? "true" : "false");
      printf("\t\tTRIFOIL:            %s\n", TriFoil() ? "true" : "false");
      printf("\t\tTIP:                %s\n", Tip() ? "true" : "false");
      printf("\t\tCSM:                %s\n", CSM() ? "true" : "false");
      printf("\t\tSPICE:              %s\n", Spice() ? "true" : "false");
      printf("\t\tGENERIC:            %s\n", Generic() ? "true" : "false");
      printf("\t\tS3:                 %s\n", S3() ? "true" : "false");
      printf("\t\tBAMBINO:            %s\n", Bambino() ? "true" : "false");
      printf("\t\tRF:                 %s\n", RF() ? "true" : "false");
      printf("\t\tGRIFFIN:            %s\n", Griffin() ? "true" : "false");
      printf("\t\tSCEPTAR:            %s\n", Sceptar() ? "true" : "false");
      printf("\t\tPACES:              %s\n", Paces() ? "true" : "false");
      printf("\t\tDESCANT:            %s\n", Descant() ? "true" : "false");
      printf("\t\tZDS:                %s\n", ZeroDegree() ? "true" : "false");
      printf("\t\tDANTE:              %s\n", Dante() ? "true" : "false");
      printf("\t\tFIPPS:              %s\n", Fipps() ? "true" : "false");
      printf("\n");
      printf(DBLUE "\tArray Position (mm) = " DRED "%.01f" RESET_COLOR "\n", TGRSIRunInfo::HPGeArrayPosition());
      printf(DBLUE "\tDESCANT in ancillary positions = " DRED "%s" RESET_COLOR "\n",
             TGRSIRunInfo::DescantAncillary() ? "TRUE" : "FALSE");
      printf("\n");
      printf("\t==============================\n");
   }
}

void TGRSIRunInfo::Clear(Option_t*)
{
   // Clears the TGRSIRunInfo. Currently, there are no available
   // options.

   fHPGeArrayPosition = 110.0;

   fTigress = false;
   fSharc   = false;
   fTriFoil = false;
   fRf      = false;
   fCSM     = false;
   fSpice   = false;
   fTip     = false;
   fS3      = false;
   fGeneric = false;
   fBambino = false;

   fGriffin    = false;
   fSceptar    = false;
   fPaces      = false;
   fDante      = false;
   fZeroDegree = false;
   fDescant    = false;
   fFipps      = false;

   fDescantAncillary = false;

   fNumberOfTrueSystems = 0;
   fBadCycleList.clear();
   fBadCycleListSize = 0;
}

void TGRSIRunInfo::SetRunInfo(int runnum, int subrunnum)
{
   /// Sets the run info. This figures out what systems are available.

   printf("In runinfo, found %i channels.\n", TChannel::GetNumberOfChannels());
   if(runnum != 0) {
      if(RunNumber() != 0 && RunNumber() != runnum) {
         std::cout<<"Warning, overwriting non-default run-number "<<RunNumber()<<" with "<<runnum<<std::endl;
      }
      SetRunNumber(runnum);
   }
   if(subrunnum != -1) {
      if(SubRunNumber() != -1 && SubRunNumber() != subrunnum) {
         std::cout<<"Warning, overwriting non-default sub-run-number "<<SubRunNumber()<<" with "<<subrunnum
                  <<std::endl;
      }
      SetSubRunNumber(subrunnum);
   }

   std::map<unsigned int, TChannel*>::iterator iter;

   for(iter = TChannel::GetChannelMap()->begin(); iter != TChannel::GetChannelMap()->end(); iter++) {
      std::string channelname = iter->second->GetName();

      //  detector system type.
      //  for more info, see: https://www.triumf.info/wiki/tigwiki/index.php/Detector_Nomenclature
      switch(iter->second->GetMnemonic()->System()) {
			case TMnemonic::ESystem::kTigress:
				if(!Tigress()) {
					Get().fNumberOfTrueSystems++;
				}
				SetTigress();
				break;
			case TMnemonic::ESystem::kSharc:
				if(!Sharc()) {
					Get().fNumberOfTrueSystems++;
				}
				SetSharc();
				break;
			case TMnemonic::ESystem::kTriFoil:
				if(!TriFoil()) {
					Get().fNumberOfTrueSystems++;
				}
				SetTriFoil();
				break;
			case TMnemonic::ESystem::kRF:
				if(!RF()) {
					Get().fNumberOfTrueSystems++;
				}
				SetRF();
				break;
			case TMnemonic::ESystem::kCSM:
				if(!CSM()) {
					Get().fNumberOfTrueSystems++;
				}
				SetCSM();
				break;
			case TMnemonic::ESystem::kTip:
				if(!Tip()) {
					Get().fNumberOfTrueSystems++;
				}
				SetTip();
				break;
			case TMnemonic::ESystem::kGriffin:
				if(!Griffin()) {
					Get().fNumberOfTrueSystems++;
				}
				SetGriffin();
				break;
			case TMnemonic::ESystem::kSceptar:
				if(!Sceptar()) {
					Get().fNumberOfTrueSystems++;
				}
				SetSceptar();
				break;
			case TMnemonic::ESystem::kPaces:
				if(!Paces()) {
					Get().fNumberOfTrueSystems++;
				}
				SetPaces();
				break;
			case TMnemonic::ESystem::kLaBr:
				if(!Dante()) {
					Get().fNumberOfTrueSystems++;
				}
				SetDante();
				break;
			case TMnemonic::ESystem::kZeroDegree:
				if(!ZeroDegree()) {
					Get().fNumberOfTrueSystems++;
				}
				SetZeroDegree();
				break;
			case TMnemonic::ESystem::kDescant:
				if(!Descant()) {
					Get().fNumberOfTrueSystems++;
				}
				SetDescant();
				break;
			case TMnemonic::ESystem::kFipps:
				if(!Fipps()) {
					Get().fNumberOfTrueSystems++;
				}
				SetFipps();
				break;
			case TMnemonic::ESystem::kGeneric:
				if(!Generic()) {
					Get().fNumberOfTrueSystems++;
				}
				SetGeneric();
				break;
			default:
				std::string system = iter->second->GetMnemonic()->SystemString();
				if(!Spice() && !S3()) {
					if(system.compare("SP") == 0) {
						if(!Spice()) {
							Get().fNumberOfTrueSystems++;
						}
						SetSpice();
						if(!S3()) {
							Get().fNumberOfTrueSystems++;
						}
						SetS3();
					}
				} else if(!Bambino()) {
					if(system.compare("BA") == 0) {
						Get().fNumberOfTrueSystems++;
					}
					SetBambino();
				}
		};
	}

	if(Get().fRunInfoFile.length() != 0u) {
		ParseInputData(Get().fRunInfoFile.c_str());
	}

   // Get().Print("a");
}

void TGRSIRunInfo::SetAnalysisTreeBranches(TTree*)
{
   // Currently does nothing.
}

Bool_t TGRSIRunInfo::ReadInfoFile(const char* filename)
{
   // Read in a run info file. These files have the extension .info.
   // An example can be found in the "examples" directory.
   std::string infilename;
   infilename.append(filename);
   printf("Reading info from file:" CYAN " %s" RESET_COLOR "\n", filename);
   if(infilename.length() == 0) {
      printf("Bad file name length\n");
      return false;
   }

   std::ifstream infile;
   infile.open(infilename.c_str());
   if(!infile) {
      printf("could not open file.\n");
      return false;
   }
   infile.seekg(0, std::ios::end);
   int length = infile.tellg();
   if(length < 1) {
      printf("file is empty.\n");
      return false;
   }
   auto* buffer = new char[length];
   infile.seekg(0, std::ios::beg);
   infile.read(buffer, length);

   Get().SetRunInfoFileName(filename);
   Get().SetRunInfoFile(buffer);

   return ParseInputData(const_cast<const char*>(buffer));
}

Bool_t TGRSIRunInfo::ParseInputData(const char* inputdata, Option_t* opt)
{
   // A helper function to parse the run info file.

   std::istringstream infile(inputdata);
   std::string        line;
   int                linenumber = 0;

   // Parse the info file.
   while(!std::getline(infile, line).fail() ) {
      linenumber++;
      trim(&line);
      size_t comment = line.find("//");
      if(comment != std::string::npos) {
         line = line.substr(0, comment);
      }
      if(line.length() == 0u) {
         continue;
      }

      size_t ntype = line.find(':');
      if(ntype == std::string::npos) {
         continue; // no seperator, not useful.
      }

      std::string type = line.substr(0, ntype);
      line             = line.substr(ntype + 1, line.length());
      trim(&line);
      int j = 0;
      while(type[j] != 0) {
         char c    = *(type.c_str() + j);
         c         = toupper(c);
         type[j++] = c;
      }
      if(type.compare("CAL") == 0 || type.compare("CALFILE") == 0) {
         // TODO Make this work again, using priorities
         // TGRSIOptions::AddInputCalFile(line);
      } else if(type.compare("MID") == 0 || type.compare("MIDAS") == 0 || type.compare("MIDASFILE") == 0) {
         // TODO Make this work again, using priorities
         // TGRSIOptions::AddInputMidasFile(line);
      } else if(type.compare("ARRAYPOS") == 0 || type.compare("HPGEPOS") == 0) {
         std::istringstream ss(line);
         double             temp_double;
         ss >> temp_double;
         Get().SetHPGeArrayPosition(temp_double);
      } else if(type.compare("DESCANTANCILLARY") == 0) {
         std::istringstream ss(line);
         int                temp_int;
         ss >> temp_int;
         Get().SetDescantAncillary(temp_int != 0);
      } else if(type.compare("BADCYCLE") == 0) {
         std::istringstream ss(line);
         int                tmp_int;
         while(!(ss >> tmp_int).fail() ) {
            Get().AddBadCycle(tmp_int);
         }
      }
   }

   if(strcmp(opt, "q") != 0) {
      printf("parsed %i lines.\n", linenumber);
      printf(DBLUE "\tArray Position (mm) = " DRED "%lf" RESET_COLOR "\n", TGRSIRunInfo::HPGeArrayPosition());
   }
   return true;
}

void TGRSIRunInfo::trim(std::string* line, const std::string& trimChars)
{
   /// Removes the string "trimCars" from  the string 'line'
   if(line->length() == 0) {
      return;
   }
   std::size_t found = line->find_first_not_of(trimChars);
   if(found != std::string::npos) {
      *line = line->substr(found, line->length());
   }
   found = line->find_last_not_of(trimChars);
   if(found != std::string::npos) {
      *line = line->substr(0, found + 1);
   }
   return;
}

Long64_t TGRSIRunInfo::Merge(TCollection* list)
{
   // Loop through the TCollection of TGRSIRunInfos, and add each entry to the original TGRSIRunInfo List
   TIter it(list);
   // The TCollection will be filled by something like hadd. Each element in the list will be a TGRSIRunInfo from
   // an individual file that was submitted to hadd.
   TGRSIRunInfo* runinfo = nullptr;

   while((runinfo = static_cast<TGRSIRunInfo*>(it.Next())) != nullptr) {
      // Now we want to loop through each TGRSISortList and find the TGRSISortInfo's stored in there.
      Add(runinfo);
   }
   return 0;
}

void TGRSIRunInfo::PrintBadCycles() const
{
   std::cout<<"Bad Cycles:\t";
   if(Get().fBadCycleList.empty()) {
      std::cout<<"NONE"<<std::endl;
   } else {
      for(int it : Get().fBadCycleList) {
         std::cout<<" "<<it;
      }
      std::cout<<std::endl;
   }
}

void TGRSIRunInfo::AddBadCycle(int bad_cycle)
{
   //   auto bad_cycle_it = std::binary_(fBadCycleList.begin(), fBadCycleList.end(),bad_cycle);
   /*  if(bad_cycle_it == fBadCycleList.end()){
        fBadCycleList.push_back(bad_cycle);
        std::sort(fBadCycleList.begin(), fBadCycleList.end());
     }*/
   if(!(std::binary_search(Get().fBadCycleList.begin(), Get().fBadCycleList.end(), bad_cycle))) {
      Get().fBadCycleList.push_back(bad_cycle);
      std::sort(Get().fBadCycleList.begin(), Get().fBadCycleList.end());
   }
   Get().fBadCycleListSize = Get().fBadCycleList.size();
}

void TGRSIRunInfo::RemoveBadCycle(int cycle)
{
   Get().fBadCycleList.erase(std::remove(Get().fBadCycleList.begin(), Get().fBadCycleList.end(), cycle), Get().fBadCycleList.end());
   std::sort(Get().fBadCycleList.begin(), Get().fBadCycleList.end());
   Get().fBadCycleListSize = Get().fBadCycleList.size();
}

bool TGRSIRunInfo::IsBadCycle(int cycle) const
{
   return std::binary_search(Get().fBadCycleList.begin(), Get().fBadCycleList.end(), cycle);
}

bool TGRSIRunInfo::WriteToRoot(TFile* fileptr)
{
   /// Writes Info File information to the tree
   // Maintain old gDirectory info
   bool        bool2return = true;
   TDirectory* savdir      = gDirectory;

   if(fileptr == nullptr) {
      fileptr = gDirectory->GetFile();
   }
   fileptr->cd();
   std::string oldoption = std::string(fileptr->GetOption());
   if(oldoption == "READ") {
      fileptr->ReOpen("UPDATE");
   }
   if(!gDirectory) {
      printf("No file opened to write to.\n");
      bool2return = false;
   } else {
      Get().Write();
   }

   printf("Writing TGRSIRunInfo to %s\n", gDirectory->GetFile()->GetName());
   if(oldoption == "READ") {
      printf("  Returning %s to \"%s\" mode.\n", gDirectory->GetFile()->GetName(), oldoption.c_str());
      fileptr->ReOpen("READ");
   }
   savdir->cd(); // Go back to original gDirectory

   return bool2return;
}

bool TGRSIRunInfo::WriteInfoFile(const std::string& filename)
{

   if(filename.length() > 0) {
      std::ofstream infoout;
      infoout.open(filename.c_str());
      std::string infostr = Get().PrintToString();
      infoout<<infostr.c_str();
      infoout<<std::endl;
      infoout<<std::endl;
      infoout.close();
   } else {
      printf("Please enter a file name\n");
      return false;
   }

   return true;
}

std::string TGRSIRunInfo::PrintToString(Option_t*)
{
   std::string buffer;
   buffer.append("//The Array Position in mm.\n");
   buffer.append(Form("HPGePos: %lf\n", Get().HPGeArrayPosition()));
   buffer.append("\n\n");
   if(Get().DescantAncillary()) {
      buffer.append("//Is DESCANT in Ancillary positions?.\n");
      buffer.append(Form("DescantAncillary: %d\n", 1));
      buffer.append("\n\n");
   }
   if(!Get().fBadCycleList.empty()) {
      buffer.append("//A List of bad cycles.\n");
      buffer.append("BadCycle:");
      for(int& it : Get().fBadCycleList) {
         buffer.append(Form(" %d", it));
      }
      buffer.append("\n\n");
   }

   return buffer;
}

//void TGRSIRunInfo::Streamer(TBuffer& R__b)
//{
//	std::cout<<__PRETTY_FUNCTION__<<std::endl;
//	std::cout<<this<<".Print():"<<std::endl;
//	Print();
//	std::cout<<&Get()<<".Print():"<<std::endl;
//	Get().Print();
//	if(R__b.IsReading()) {
//		std::cout<<"Reading TBuffer size "<<R__b.BufferSize()<<", "<<Class()->GetName()<<", "<<&(Get())<<"/"<<this<<std::endl;
//      R__b.ReadClassBuffer(TGRSIRunInfo::Class(),this);
//		//Set(*this);
//		std::cout<<"ReadClassBuffer("<<Class()->GetName()<<", "<<&(Get())<<"/"<<this<<"), TBuffer size "<<R__b.BufferSize()<<std::endl;
//	} else {
//		std::cout<<"Writing TBuffer size "<<R__b.BufferSize()<<std::endl;
//      R__b.WriteClassBuffer(TGRSIRunInfo::Class(),this);
//		std::cout<<"WriteClassBuffer("<<Class()->GetName()<<", "<<&(Get())<<"/"<<this<<"), TBuffer size "<<R__b.BufferSize()<<std::endl;
//	}
//	std::cout<<this<<".Print():"<<std::endl;
//	Print();
//	std::cout<<&Get()<<".Print():"<<std::endl;
//	Get().Print();
//	std::cout<<"----------------------------------------"<<std::endl;
//}
