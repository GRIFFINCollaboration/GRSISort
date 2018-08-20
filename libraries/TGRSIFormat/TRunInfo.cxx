#include "TRunInfo.h"
#include "TMnemonic.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "TROOT.h"

#include "TGRSIOptions.h"
#include "GVersion.h"

/// \cond CLASSIMP
ClassImp(TRunInfo)
/// \endcond

std::string TRunInfo::fVersion;

Bool_t TRunInfo::ReadInfoFromFile(TFile* tempf)
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
	TKey* key;
   while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
      if((key == nullptr) || (strcmp(key->GetClassName(), "TRunInfo") != 0)) {
         continue;
      }

      Set(static_cast<TRunInfo*>(key->ReadObj()));
		Get()->fDetectorInformation = nullptr;// just to be safe
		// see if we can find detector information
		while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
			if((key == nullptr) || !TClass(key->GetClassName()).InheritsFrom(TDetectorInformation::Class())) {
				continue;
			}
			Get()->fDetectorInformation = static_cast<TDetectorInformation*>(key->ReadObj());
			break;
		}
      savdir->cd();
      return true;
   }
   savdir->cd();

   return false;
}

TRunInfo::TRunInfo() : fRunNumber(0), fSubRunNumber(-1)
{
   /// Default ctor for TRunInfo. The default values are:
   ///
   /// fHPGeArrayPosition = 110.0;

	fDetectorInformation = nullptr;
   Clear();
}

TRunInfo::~TRunInfo() = default;

void TRunInfo::Print(Option_t* opt) const
{
   // Prints the TRunInfo. Options:
   // a: Print out more details.
	TSingleton<TRunInfo>::PrintDirectory();
   std::cout<<"Title: "<<RunTitle()<<std::endl;
   std::cout<<"Comment: "<<RunComment()<<std::endl;
	time_t tmpStart = static_cast<time_t>(RunStart());
	time_t tmpStop  = static_cast<time_t>(RunStop());
	struct tm runStart = *localtime(const_cast<const time_t*>(&tmpStart));
	struct tm runStop  = *localtime(const_cast<const time_t*>(&tmpStop));
	printf("\t\tRunNumber:          %05i\n", RunNumber());
	printf("\t\tSubRunNumber:       %03i\n", SubRunNumber());
	if(Get()->RunStart() != 0 && Get()->RunStop() != 0) {
		printf("\t\tRunStart:           %s", asctime(&runStart));
		printf("\t\tRunStop:            %s", asctime(&runStop));
		printf("\t\tRunLength:          %.0f s\n", RunLength());
	} else {
		printf("\t\tCombined RunLength: %.0f s\n", RunLength());
	}
   if(strchr(opt, 'a') != nullptr) {
      printf("\n");
      printf("\t==============================\n");
      printf(DBLUE "\t\tArray Position (mm) = " DRED "%.01f" RESET_COLOR "\n", TRunInfo::HPGeArrayPosition());
		if(fDetectorInformation != nullptr) fDetectorInformation->Print(opt);
      printf("\t==============================\n");
   }
}

void TRunInfo::Clear(Option_t*)
{
   // Clears the TRunInfo. Currently, there are no available
   // options.

   fHPGeArrayPosition = 110.0;

   fBadCycleList.clear();
   fBadCycleListSize = 0;

	delete fDetectorInformation;
	fDetectorInformation = nullptr;
}

void TRunInfo::SetRunInfo(int runnum, int subrunnum)
{
   /// Sets the run info. This figures out what systems are available.

   std::cout<<__PRETTY_FUNCTION__<<": found "<<TChannel::GetNumberOfChannels()<<" channels."<<std::endl;
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

	if(Get()->fRunInfoFile.length() != 0u) {
		ParseInputData(Get()->fRunInfoFile.c_str());
	}

	// set version of GRSISort
	TRunInfo::ClearVersion();
	TRunInfo::SetVersion(GRSI_RELEASE);
}

void TRunInfo::SetAnalysisTreeBranches(TTree*)
{
   // Currently does nothing.
}

Bool_t TRunInfo::ReadInfoFile(const char* filename)
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

   Get()->SetRunInfoFileName(filename);
   Get()->SetRunInfoFile(buffer);

   return ParseInputData(const_cast<const char*>(buffer));
}

Bool_t TRunInfo::ParseInputData(const char* inputdata, Option_t* opt)
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
         Get()->SetHPGeArrayPosition(temp_double);
      } else if(type.compare("BADCYCLE") == 0) {
         std::istringstream ss(line);
         int                tmp_int;
         while(!(ss >> tmp_int).fail() ) {
            Get()->AddBadCycle(tmp_int);
         }
      }
   }

   if(strcmp(opt, "q") != 0) {
      printf("parsed %i lines.\n", linenumber);
      printf(DBLUE "\tArray Position (mm) = " DRED "%lf" RESET_COLOR "\n", TRunInfo::HPGeArrayPosition());
   }
   return true;
}

void TRunInfo::trim(std::string* line, const std::string& trimChars)
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

Long64_t TRunInfo::Merge(TCollection* list)
{
   // Loop through the TCollection of TRunInfos, and add each entry to the original TRunInfo List
   TIter it(list);
   // The TCollection will be filled by something like hadd. Each element in the list will be a TRunInfo from
   // an individual file that was submitted to hadd.
   TRunInfo* runinfo = nullptr;

   while((runinfo = static_cast<TRunInfo*>(it.Next())) != nullptr) {
      // Now we want to loop through each TGRSISortList and find the TGRSISortInfo's stored in there.
      Add(runinfo);
   }
   return 0;
}

void TRunInfo::PrintBadCycles() const
{
   std::cout<<"Bad Cycles:\t";
   if(Get()->fBadCycleList.empty()) {
      std::cout<<"NONE"<<std::endl;
   } else {
      for(int it : Get()->fBadCycleList) {
         std::cout<<" "<<it;
      }
      std::cout<<std::endl;
   }
}

void TRunInfo::AddBadCycle(int bad_cycle)
{
   //   auto bad_cycle_it = std::binary_(fBadCycleList.begin(), fBadCycleList.end(),bad_cycle);
   /*  if(bad_cycle_it == fBadCycleList.end()){
        fBadCycleList.push_back(bad_cycle);
        std::sort(fBadCycleList.begin(), fBadCycleList.end());
     }*/
   if(!(std::binary_search(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end(), bad_cycle))) {
      Get()->fBadCycleList.push_back(bad_cycle);
      std::sort(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end());
   }
   Get()->fBadCycleListSize = Get()->fBadCycleList.size();
}

void TRunInfo::RemoveBadCycle(int cycle)
{
   Get()->fBadCycleList.erase(std::remove(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end(), cycle), Get()->fBadCycleList.end());
   std::sort(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end());
   Get()->fBadCycleListSize = Get()->fBadCycleList.size();
}

bool TRunInfo::IsBadCycle(int cycle) const
{
   return std::binary_search(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end(), cycle);
}

bool TRunInfo::WriteToRoot(TFile* fileptr)
{
   /// Writes Info File information to the tree
   // Maintain old gDirectory info
   bool        bool2return = true;
   TDirectory* savdir      = gDirectory;
	gROOT->cd();
	TRunInfo* runInfo   = Get();

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
      runInfo->Write();
		runInfo->fDetectorInformation->Write();
   }

   printf("Writing TRunInfo to %s\n", gDirectory->GetFile()->GetName());
   if(oldoption == "READ") {
      printf("  Returning %s to \"%s\" mode.\n", gDirectory->GetFile()->GetName(), oldoption.c_str());
      fileptr->ReOpen("READ");
   }
   savdir->cd(); // Go back to original gDirectory

   return bool2return;
}

bool TRunInfo::WriteInfoFile(const std::string& filename)
{

   if(filename.length() > 0) {
      std::ofstream infoout;
      infoout.open(filename.c_str());
      std::string infostr = Get()->PrintToString();
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

std::string TRunInfo::PrintToString(Option_t*)
{
   std::string buffer;
   buffer.append("//The Array Position in mm.\n");
   buffer.append(Form("HPGePos: %lf\n", Get()->HPGeArrayPosition()));
   buffer.append("\n\n");
   if(!Get()->fBadCycleList.empty()) {
      buffer.append("//A List of bad cycles.\n");
      buffer.append("BadCycle:");
      for(int& it : Get()->fBadCycleList) {
         buffer.append(Form(" %d", it));
      }
      buffer.append("\n\n");
   }

   return buffer;
}

TEventBuildingLoop::EBuildMode TRunInfo::BuildMode() const
{
	std::cout<<this<<": default build mode "<<static_cast<int>(TEventBuildingLoop::EBuildMode::kDefault)<<std::endl;
	return TEventBuildingLoop::EBuildMode::kDefault;
}
