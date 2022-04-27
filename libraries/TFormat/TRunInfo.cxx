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
std::string TRunInfo::fFullVersion;
std::string TRunInfo::fDate;
std::string TRunInfo::fLibraryVersion;

Bool_t TRunInfo::ReadInfoFromFile(TFile* tempf)
{
   TDirectory* savdir = gDirectory;
   if(tempf != nullptr) {
      tempf->cd();
   }

   if((gDirectory->GetFile()) == nullptr) {
      std::cout<<"File does not exist"<<std::endl;
      savdir->cd();
      return false;
   }

   tempf = gDirectory->GetFile();

   TList* list = tempf->GetListOfKeys();
   TIter  iter(list);
	TKey* key;
   while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
      if(strcmp(key->GetClassName(), "TRunInfo") != 0) {
         continue;
      }

      Set(static_cast<TRunInfo*>(key->ReadObj()));
		Get()->fDetectorInformation = nullptr;// just to be safe
		// see if we can find detector information
		while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
			// for some reason the classic way of using TClass::InheritsFrom fails in grsiproof
			// so instead we just try to dynamically cast every key into TDetectorInformation
			Get()->fDetectorInformation = dynamic_cast<TDetectorInformation*>(key->ReadObj());
			if(Get()->fDetectorInformation != nullptr) break;
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
   /// Prints the TRunInfo. Options:
   /// a: Print out more details (array position and detector information).
	TSingleton<TRunInfo>::PrintDirectory();
	std::ostringstream str;
   str<<"Title: "<<RunTitle()<<std::endl;
   str<<"Comment: "<<RunComment()<<std::endl;
	time_t tmpStart = static_cast<time_t>(RunStart());
	time_t tmpStop  = static_cast<time_t>(RunStop());
	struct tm runStart = *localtime(const_cast<const time_t*>(&tmpStart));
	struct tm runStop  = *localtime(const_cast<const time_t*>(&tmpStop));
	str<<std::setfill('0');
	if(RunNumber() != 0 && SubRunNumber() != -1) {
		str<<"\t\tRunNumber:          "<<std::setw(5)<<RunNumber()<<std::endl;
		str<<"\t\tSubRunNumber:       "<<std::setw(3)<<SubRunNumber()<<std::endl;
	} else if(RunNumber() != 0) {
		str<<"\t\tRunNumber:          "<<std::setw(5)<<RunNumber()<<std::endl;
		str<<"\t\tSubRunNumbers:      "<<std::setw(3)<<FirstSubRunNumber()<<"-"<<std::setw(3)<<LastSubRunNumber()<<std::endl;
	} else {
		str<<"\t\tRunNumbers:         "<<std::setw(5)<<FirstRunNumber()<<"-"<<std::setw(5)<<LastRunNumber()<<std::endl;
	}
	str<<std::setfill(' ');
	if(RunStart() != 0 && RunStop() != 0) {
		str<<"\t\tRunStart:           "<<asctime(&runStart);
		str<<"\t\tRunStop:            "<<asctime(&runStop);
		str<<"\t\tRunLength:          "<<RunLength()<<" s"<<std::endl;
	} else {
		str<<"\t\tCombined RunLength: "<<RunLength()<<" s"<<std::endl;
	}
   if(strchr(opt, 'a') != nullptr) {
      str<<std::endl;
      str<<"\t=============================="<<std::endl;
      str<<DBLUE "\t\tArray Position (mm) = "<<DRED<<TRunInfo::HPGeArrayPosition()<<RESET_COLOR<<std::endl;
		if(fDetectorInformation != nullptr) fDetectorInformation->Print(opt);
		else str<<"no detector information"<<std::endl;
      str<<"\t=============================="<<std::endl;
   }
	std::cout<<str.str();
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

	TRunInfo::ClearFullVersion();
	TRunInfo::SetFullVersion(GRSI_GIT_COMMIT);

	TRunInfo::ClearDate();
	TRunInfo::SetDate(GRSI_GIT_COMMIT_TIME);
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
   std::cout<<"Reading info from file: "<<CYAN<<filename<<RESET_COLOR<<std::endl;
   if(infilename.length() == 0) {
      std::cout<<"Bad file name length"<<std::endl;
      return false;
   }

   std::ifstream infile;
   infile.open(infilename.c_str());
   if(!infile) {
      std::cout<<"could not open file."<<std::endl;
      return false;
   }
   infile.seekg(0, std::ios::end);
   int length = infile.tellg();
   if(length < 1) {
      std::cout<<"file is empty."<<std::endl;
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
      std::cout<<"parsed "<<linenumber<<" lines."<<std::endl;
      std::cout<<DBLUE "\tArray Position (mm) = "<<DRED<<TRunInfo::HPGeArrayPosition()<<RESET_COLOR<<std::endl;
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
      std::cout<<"No file opened to write to."<<std::endl;
      bool2return = false;
   } else {
      runInfo->Write("RunInfo", TObject::kOverwrite);
		runInfo->fDetectorInformation->Write("DetectorInformation", TObject::kOverwrite);
   }

   std::cout<<"Writing TRunInfo to "<<gDirectory->GetFile()->GetName()<<std::endl;
   if(oldoption == "READ") {
      std::cout<<"  Returning "<<gDirectory->GetFile()->GetName()<<" to \""<<oldoption<<"\" mode."<<std::endl;
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
      std::cout<<"Please enter a file name"<<std::endl;
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

void TRunInfo::Add(TRunInfo* runinfo)
{
	// add new run to list of runs (and check if the current run needs to be added)
	if(fRunList.empty()) {
		std::pair<int, int> currentPair = std::make_pair(fRunNumber, fSubRunNumber);
		fRunList.push_back(currentPair);
	}
	std::pair<int, int> newPair = std::make_pair(runinfo->fRunNumber, runinfo->fSubRunNumber);
	// check for dual entries
	if(std::find(fRunList.begin(), fRunList.end(), newPair) != fRunList.end()) {
		std::cerr<<DYELLOW<<"Warning, adding run "<<std::setfill('0')<<std::setw(5)<<newPair.first<<"_"<<std::setw(3)<<newPair.second<<std::setfill(' ')<<" again!"<<RESET_COLOR<<std::endl;
		return;
	}
	fRunList.push_back(newPair);

	//std::cout<<"adding run "<<runinfo->fRunNumber<<", sub run "<<runinfo->fSubRunNumber<<" to run "<<fRunNumber<<", sub run "<<fSubRunNumber<<std::endl;
	// add the run length together
	if(runinfo->fRunLength > 0) {
		if(fRunLength > 0) {
			fRunLength += runinfo->fRunLength;
		} else {
			fRunLength = runinfo->fRunLength;
		}
	}

	if(runinfo->fRunNumber != fRunNumber) {
		// check if the added run is an increment of the current run number (if the run number is set)
		if(fRunNumber != 0) {
			if(runinfo->fRunNumber + 1 == fRunNumber) {
				//std::cout<<"found second run ("<<runinfo->fRunNumber<<") before current run ("<<fRunNumber<<")"<<std::endl;
				// use runinfo as first run and the current run as last
				fFirstRunNumber = runinfo->fRunNumber;
				fLastRunNumber = fRunNumber;
				fRunStart = runinfo->fRunStart; // no need to set run stop
			} else if(runinfo->fRunNumber - 1 == fRunNumber) {
				//std::cout<<"found second run ("<<runinfo->fRunNumber<<") after current run ("<<fRunNumber<<")"<<std::endl;
				// use runinfo as last run and the current run as first
				fFirstRunNumber = fRunNumber;
				fLastRunNumber = runinfo->fRunNumber;
				fRunStop = runinfo->fRunStop; // no need to set run start
			} else {
				//std::cout<<"found second run ("<<runinfo->fRunNumber<<") non-consecutive to run ("<<fRunNumber<<")"<<std::endl;
				// run start and stop don't make a lot of sense with non-consecutive runs (?)
				fRunStart = 0.;
				fRunStop  = 0.;
				// still need to keep some kind of information about the runs (e.g. to create filenames)
				// by keeping first and last the exact same it is still obvious that these are not consecutive runs
				fFirstRunNumber = fRunNumber;
				fLastRunNumber = fRunNumber;
			}
			// the run number is meaningful only when the run numbers are the same
			fRunNumber = 0;
			fSubRunNumber = -1;
		} else if(fFirstRunNumber != 0 && fLastRunNumber != 0 && fFirstRunNumber != fLastRunNumber) {
			// if we already have a (good) range of runs, check if runinfo fits at the beginning or the end
			if(runinfo->fRunNumber + 1 == fFirstRunNumber) {
				//std::cout<<"found another run ("<<runinfo->fRunNumber<<") before first run ("<<fFirstRunNumber<<")"<<std::endl;
				// use runinfo as first run
				fFirstRunNumber = runinfo->fRunNumber;
				fRunStart = runinfo->fRunStart;
			} else if(runinfo->fRunNumber - 1 == fLastRunNumber) {
				//std::cout<<"found another run ("<<runinfo->fRunNumber<<") after last run ("<<fLastRunNumber<<")"<<std::endl;
				// use runinfo as last run
				fLastRunNumber = runinfo->fRunNumber;
				fRunStop = runinfo->fRunStop;
			} else if(runinfo->fRunNumber == fFirstRunNumber || runinfo->fRunNumber == fLastRunNumber) {
				//std::cout<<"found another sub(?) run part of runs ("<<fFirstRunNumber<<" - "<<fLastRunNumber<<")"<<std::endl;
				// found probably another subrun of a run already added
				// since we do not keep track of all subruns we have to assume this is in order
				// so we only update the run start or stop if necessary
				//std::cout<<"changing run start/stop from "<<std::setw(16)<<fRunStart<<"/"<<std::setw(16)<<fRunStop<<" to ";
				if(fRunStop < runinfo->fRunStop) fRunStop = runinfo->fRunStop;
				if(fRunStart > runinfo->fRunStart) fRunStart = runinfo->fRunStart;
				//std::cout<<std::setw(16)<<fRunStart<<"/"<<std::setw(16)<<fRunStop<<std::endl;
			} else {
				//std::cout<<"found another run ("<<runinfo->fRunNumber<<") non-consecutive to runs ("<<fFirstRunNumber<<" - "<<fLastRunNumber<<")"<<std::endl;
				// run start and stop don't make a lot of sense with non-consecutive runs (?)
				fRunStart = 0.;
				fRunStop  = 0.;
				// still need to keep some kind of information about the runs (e.g. to create filenames)
				// by keeping first and last the exact same it is still obvious that these are not consecutive runs
				fLastRunNumber = fFirstRunNumber;
			}
		} else {
			// the run number is zero, and we do not have a (good) range, so there is nothing to do.
			//std::cout<<"found another run ("<<runinfo->fRunNumber<<") non-consecutive run ("<<fFirstRunNumber<<" - "<<fLastRunNumber<<")"<<std::endl;
		}
	} else if(fSubRunNumber != -1) {
		// check if the added sub run is an increment of the current run number
		if(runinfo->fSubRunNumber + 1 == fSubRunNumber) {
			//std::cout<<"found second sub run ("<<runinfo->fSubRunNumber<<") before current sub run ("<<fSubRunNumber<<")"<<std::endl;
			// if the run numbers are the same and we have subsequent sub runs we can update the run start
			fRunStart = runinfo->fRunStart;
			fFirstSubRunNumber = runinfo->fSubRunNumber;
			fLastSubRunNumber = fSubRunNumber;
		} else if(runinfo->fSubRunNumber - 1 == fSubRunNumber) {
			//std::cout<<"found second sub run ("<<runinfo->fSubRunNumber<<") after current sub run ("<<fSubRunNumber<<")"<<std::endl;
			// if the run numbers are the same and we have subsequent sub runs we can update the run stop
			fRunStop = runinfo->fRunStop;
			fFirstSubRunNumber = fSubRunNumber;
			fLastSubRunNumber = runinfo->fSubRunNumber;
		} else {
			//std::cout<<"found second sub run ("<<runinfo->fSubRunNumber<<") non-consecutive to current sub run ("<<fSubRunNumber<<")"<<std::endl;
			// with multiple non-sequential subruns added, the sub run number and start/stop have no meaning anymore
			fRunStart = 0.;
			fRunStop  = 0.;
		}
		// sub run number is only meaningful if it's the only sub run
		fSubRunNumber = -1;
	} else {
		// we have the same run with a range of sub-runs already added, so check if this once fits at the end or the beginning
		if(runinfo->fSubRunNumber + 1 == fFirstSubRunNumber) {
			//std::cout<<"found another sub run ("<<runinfo->fSubRunNumber<<") before first sub run ("<<fFirstSubRunNumber<<")"<<std::endl;
			// use runinfo as first run
			fFirstSubRunNumber = runinfo->fSubRunNumber;
			fRunStart = runinfo->fRunStart;
		} else if(runinfo->fSubRunNumber - 1 == fLastSubRunNumber) {
			//std::cout<<"found another sub run ("<<runinfo->fSubRunNumber<<") after last sub run ("<<fLastSubRunNumber<<")"<<std::endl;
			// use runinfo as last run
			fLastSubRunNumber = runinfo->fSubRunNumber;
			fRunStop = runinfo->fRunStop;
		} else {
			//std::cout<<"found another sub run ("<<runinfo->fSubRunNumber<<") non-consecutive to sub runs ("<<fFirstSubRunNumber<<" - "<<fLastSubRunNumber<<")"<<std::endl;
			// with multiple non-sequential subruns added, the sub run number and start/stop have no meaning anymore
			fRunStart = 0.;
			fRunStop  = 0.;
			fFirstSubRunNumber = -1;
			fLastSubRunNumber = -1;
		}
	}
}

void TRunInfo::PrintRunList()
{
	if(fRunList.empty()) {
		std::cout<<"No runs added to list of runs!"<<std::endl;
		return;
	}
	std::sort(fRunList.begin(), fRunList.end());
	std::cout<<"Got "<<fRunList.size()<<" runs:"<<std::endl;
	for(auto pair : fRunList) {
		std::cout<<std::setw(5)<<std::setfill('0')<<pair.first<<"_"<<std::setw(3)<<pair.second<<std::setfill(' ')<<std::endl;
	}
}
