#include "TRunInfo.h"
#include "TMnemonic.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "TROOT.h"

#include "TGRSIOptions.h"
#include "GVersion.h"
#include "TGRSIUtilities.h"

Bool_t TRunInfo::ReadInfoFromFile(TFile* tempf)
{
   TDirectory* savdir = gDirectory;
   if(tempf != nullptr) {
      tempf->cd();
   }

   if((gDirectory->GetFile()) == nullptr) {
      std::cout << "File does not exist" << std::endl;
      savdir->cd();
      return false;
   }

   tempf = gDirectory->GetFile();

   // try and read the run info directly
   if(tempf->FindObjectAny("TRunInfo") != nullptr) {
      Get()->fDetectorInformation = static_cast<TDetectorInformation*>(tempf->Get("TRunInfo"));
      savdir->cd();
      return true;
   }

   // if the above failed we try all keys to find a run info
   TList* list = tempf->GetListOfKeys();
   TIter  iter(list);
   TKey*  key = nullptr;
   while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
      if(strcmp(key->GetClassName(), "TRunInfo") != 0) {
         continue;
      }

      Set(static_cast<TRunInfo*>(key->ReadObj()));
      Get()->fDetectorInformation = nullptr;   // just to be safe
      // see if we can find detector information
      while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
         // for some reason the classic way of using TClass::InheritsFrom fails in grsiproof
         // so instead we just try to dynamically cast every key into TDetectorInformation
         Get()->fDetectorInformation = dynamic_cast<TDetectorInformation*>(key->ReadObj());
         if(Get()->fDetectorInformation != nullptr) { break; }
      }
      savdir->cd();
      return true;
   }
   savdir->cd();

   return false;
}

TRunInfo::TRunInfo()
{
   /// Default ctor for TRunInfo. The default values are:
   ///
   /// fHPGeArrayPosition = 110.0;

   Clear();
}

void TRunInfo::Print(Option_t* opt) const
{
   /// Prints the TRunInfo. Options:
   /// a: Print out more details (array position and detector information).
   TSingleton<TRunInfo>::PrintDirectory();
   std::ostringstream str;
   str << "Title: " << RunTitle() << std::endl;
   str << "Comment: " << RunComment() << std::endl;
   auto      tmpStart = static_cast<time_t>(RunStart());
   auto      tmpStop  = static_cast<time_t>(RunStop());
   struct tm runStart = *localtime(&tmpStart);
   struct tm runStop  = *localtime(&tmpStop);
   str << std::setfill('0');
   if(RunNumber() != 0 && SubRunNumber() != -1) {
      str << "\t\tRunNumber:          " << std::setw(5) << RunNumber() << std::endl;
      str << "\t\tSubRunNumber:       " << std::setw(3) << SubRunNumber() << std::endl;
   } else if(RunNumber() != 0) {
      str << "\t\tRunNumber:          " << std::setw(5) << RunNumber() << std::endl;
      str << "\t\tSubRunNumbers:      " << std::setw(3) << FirstSubRunNumber() << "-" << std::setw(3) << LastSubRunNumber() << std::endl;
   } else if(FirstRunNumber() != LastRunNumber()) {
      str << "\t\tRunNumbers:         " << std::setw(5) << FirstRunNumber() << "-" << std::setw(5) << LastRunNumber() << std::endl;
      str << "\t\tNo missing runs" << std::endl;
   } else if(!fRunList.empty()) {
      str << "\t\tRunNumbers:         " << std::setw(5) << fRunList.begin()->first << "-" << std::setw(5) << fRunList.rbegin()->first << std::endl;
      str << "\t\tMissing runs:       " << ListOfMissingRuns() << std::endl;
   } else {
      str << "\t\tNo runs in list?" << std::endl;
   }
   str << std::setfill(' ');
   if(RunStart() != 0 && RunStop() != 0) {
      str << "\t\tRunStart:           " << asctime(&runStart);
      str << "\t\tRunStop:            " << asctime(&runStop);
      str << "\t\tRunLength:          " << RunLength() << " s" << std::endl;
   } else {
      str << "\t\tCombined RunLength: " << RunLength() << " s" << std::endl;
   }
   if(strchr(opt, 'a') != nullptr) {
      str << std::endl;
      str << "\t==============================" << std::endl;
      str << DBLUE "\t\tArray Position (mm) = " << DRED << TRunInfo::HPGeArrayPosition() << RESET_COLOR << std::endl;
      if(fDetectorInformation != nullptr) {
         fDetectorInformation->Print(opt);
      } else {
         str << "no detector information" << std::endl;
      }
      str << "\t==============================" << std::endl;
   }
   std::cout << str.str();
}

void TRunInfo::Clear(Option_t*)
{
   // Clears the TRunInfo. Currently, there are no available
   // options.

   fHPGeArrayPosition = 110.0;

   fBadCycleList.clear();

   delete fDetectorInformation;
   fDetectorInformation = nullptr;
}

void TRunInfo::SetRunInfo(int runnum, int subrunnum)
{
   /// Sets the run info. This figures out what systems are available.

   std::cout << "found " << TChannel::GetNumberOfChannels() << " channels" << std::endl;
   if(runnum != 0) {
      if(RunNumber() != 0 && RunNumber() != runnum) {
         std::cout << "Warning, overwriting non-default run-number " << RunNumber() << " with " << runnum << std::endl;
      }
      SetRunNumber(runnum);
   }
   if(subrunnum != -1) {
      if(SubRunNumber() != -1 && SubRunNumber() != subrunnum) {
         std::cout << "Warning, overwriting non-default sub-run-number " << SubRunNumber() << " with " << subrunnum
                   << std::endl;
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
   /// Read in a run info file. These files have the extension .info.
   std::string infilename;
   infilename.append(filename);
   std::cout << "Reading info from file: " << CYAN << filename << RESET_COLOR << std::endl;
   if(infilename.length() == 0) {
      std::cout << "Bad file name length" << std::endl;
      return false;
   }

   std::ifstream infile;
   infile.open(infilename.c_str());
   if(!infile) {
      std::cout << "could not open file." << std::endl;
      return false;
   }
   infile.seekg(0, std::ios::end);
   auto length = infile.tellg();
   if(length < 1) {
      std::cout << "file is empty." << std::endl;
      return false;
   }
   auto* buffer = new char[length];
   infile.seekg(0, std::ios::beg);
   infile.read(buffer, length);

   SetRunInfoFileName(filename);
   SetRunInfoFile(buffer);

   return ParseInputData(buffer);
}

Bool_t TRunInfo::ParseInputData(const char* inputdata, Option_t* opt)
{
   // A helper function to parse the run info file.

   std::istringstream infile(inputdata);
   std::string        line;
   int                linenumber = 0;

   // Parse the info file.
   while(!std::getline(infile, line).fail()) {
      linenumber++;
      trim(line);
      size_t comment = line.find("//");
      if(comment != std::string::npos) {
         line = line.substr(0, comment);
      }
      if(line.length() == 0u) {
         continue;
      }

      size_t ntype = line.find(':');
      if(ntype == std::string::npos) {
         continue;   // no seperator, not useful.
      }

      std::string type = line.substr(0, ntype);
      line             = line.substr(ntype + 1, line.length());
      trim(line);
      std::transform(type.begin(), type.end(), type.begin(), ::toupper);
      if(type == "CAL" || type == "CALFILE") {
         // TODO Make this work again, using priorities
         // TGRSIOptions::AddInputCalFile(line);
      } else if(type == "MID" || type == "MIDAS" || type == "MIDASFILE") {
         // TODO Make this work again, using priorities
         // TGRSIOptions::AddInputMidasFile(line);
      } else if(type == "ARRAYPOS" || type == "HPGEPOS") {
         std::istringstream str(line);
         double             temp_double = 0.;
         str >> temp_double;
         TRunInfo::SetHPGeArrayPosition(temp_double);
      } else if(type == "BADCYCLE") {
         std::istringstream str(line);
         int                tmp_int = 0;
         while(!(str >> tmp_int).fail()) {
            TRunInfo::AddBadCycle(tmp_int);
         }
      }
   }

   if(strcmp(opt, "q") != 0) {
      std::cout << "parsed " << linenumber << " lines." << std::endl;
      std::cout << DBLUE "\tArray Position (mm) = " << DRED << TRunInfo::HPGeArrayPosition() << RESET_COLOR << std::endl;
   }
   return true;
}

Long64_t TRunInfo::Merge(TCollection* list)
{
   // Loop through the TCollection of TRunInfos, and add each entry to the original TRunInfo List
   TIter iter(list);
   // The TCollection will be filled by something like hadd. Each element in the list will be a TRunInfo from
   // an individual file that was submitted to hadd.
   TRunInfo* runinfo = nullptr;

   while((runinfo = static_cast<TRunInfo*>(iter.Next())) != nullptr) {
      Add(runinfo);
   }
   return 0;
}

void TRunInfo::PrintBadCycles()
{
   std::cout << "Bad Cycles:\t";
   if(Get()->fBadCycleList.empty()) {
      std::cout << "NONE" << std::endl;
   } else {
      for(int item : Get()->fBadCycleList) {
         std::cout << " " << item;
      }
      std::cout << std::endl;
   }
}

void TRunInfo::AddBadCycle(int bad_cycle)
{
   if(!(std::binary_search(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end(), bad_cycle))) {
      Get()->fBadCycleList.push_back(bad_cycle);
      std::sort(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end());
   }
}

void TRunInfo::RemoveBadCycle(int cycle)
{
   Get()->fBadCycleList.erase(std::remove(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end(), cycle), Get()->fBadCycleList.end());
   std::sort(Get()->fBadCycleList.begin(), Get()->fBadCycleList.end());
}

bool TRunInfo::IsBadCycle(int cycle)
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
   TRunInfo* runInfo = Get();

   if(fileptr == nullptr) {
      fileptr = gDirectory->GetFile();
   }
   fileptr->cd();
   std::string oldoption = std::string(fileptr->GetOption());
   if(oldoption == "READ") {
      fileptr->ReOpen("UPDATE");
   }
   if(!gDirectory) {
      std::cout << "No file opened to write TRunInfo to." << std::endl;
      bool2return = false;
   } else {
      runInfo->Write("RunInfo", TObject::kOverwrite);
      if(runInfo->fDetectorInformation != nullptr) { runInfo->fDetectorInformation->Write("DetectorInformation", TObject::kOverwrite); }
   }

   std::cout << "Writing TRunInfo to " << gDirectory->GetFile()->GetName() << std::endl;
   if(oldoption == "READ") {
      std::cout << "  Returning " << gDirectory->GetFile()->GetName() << " to \"" << oldoption << "\" mode." << std::endl;
      fileptr->ReOpen("READ");
   }
   savdir->cd();   // Go back to original gDirectory

   return bool2return;
}

bool TRunInfo::WriteInfoFile(const std::string& filename)
{

   if(filename.length() > 0) {
      std::ofstream infoout;
      infoout.open(filename.c_str());
      std::string infostr = TRunInfo::PrintToString();
      infoout << infostr.c_str();
      infoout << std::endl;
      infoout << std::endl;
      infoout.close();
   } else {
      std::cout << "Please enter a file name" << std::endl;
      return false;
   }

   return true;
}

std::string TRunInfo::PrintToString(Option_t*)
{
   std::string buffer;
   buffer.append("//The Array Position in mm.\n");
   buffer.append(Form("HPGePos: %lf\n", HPGeArrayPosition()));
   buffer.append("\n\n");
   if(!Get()->fBadCycleList.empty()) {
      buffer.append("//A List of bad cycles.\n");
      buffer.append("BadCycle:");
      for(int& item : Get()->fBadCycleList) {
         buffer.append(Form(" %d", item));
      }
      buffer.append("\n\n");
   }

   return buffer;
}

TEventBuildingLoop::EBuildMode TRunInfo::BuildMode() const
{
   std::cout << this << ": default build mode " << static_cast<int>(TEventBuildingLoop::EBuildMode::kDefault) << std::endl;
   return TEventBuildingLoop::EBuildMode::kDefault;
}

void TRunInfo::Add(TRunInfo* runinfo, bool verbose)
{
   // add new run to list of runs (and check if the current run needs to be added)
   //if(fRunList.empty()) {
   //   fRunList.emplace(fRunNumber, fSubRunNumber);
   //}
   std::pair<int, int> newPair = std::make_pair(runinfo->fRunNumber, runinfo->fSubRunNumber);
   // check for dual entries
   if(fRunList.find(newPair) != fRunList.end()) {
      std::cerr << DYELLOW << "Warning, adding run " << std::setfill('0') << std::setw(5) << newPair.first << "_" << std::setw(3) << newPair.second << std::setfill(' ') << " again!" << RESET_COLOR << std::endl;
      return;
   }
   fRunList.insert(newPair);

   if(verbose) { std::cout << "adding run " << runinfo->fRunNumber << ", sub run " << runinfo->fSubRunNumber << " to run " << fRunNumber << ", sub run " << fSubRunNumber << std::endl; }
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
            if(verbose) { std::cout << "found second run (" << runinfo->fRunNumber << ") before current run (" << fRunNumber << ")" << std::endl; }
            // use runinfo as first run and the current run as last
            fFirstRunNumber = runinfo->fRunNumber;
            fLastRunNumber  = fRunNumber;
            fRunStart       = runinfo->fRunStart;   // no need to set run stop
         } else if(runinfo->fRunNumber - 1 == fRunNumber) {
            if(verbose) { std::cout << "found second run (" << runinfo->fRunNumber << ") after current run (" << fRunNumber << ")" << std::endl; }
            // use runinfo as last run and the current run as first
            fFirstRunNumber = fRunNumber;
            fLastRunNumber  = runinfo->fRunNumber;
            fRunStop        = runinfo->fRunStop;   // no need to set run start
         } else {
            if(verbose) { std::cout << "found second run (" << runinfo->fRunNumber << ") non-consecutive to run (" << fRunNumber << ")" << std::endl; }
            // run start and stop don't make a lot of sense with non-consecutive runs (?)
            fRunStart = 0.;
            fRunStop  = 0.;
            // still need to keep some kind of information about the runs (e.g. to create filenames)
            // by keeping first and last the exact same it is still obvious that these are not consecutive runs
            fFirstRunNumber = fRunNumber;
            fLastRunNumber  = fRunNumber;
         }
         // the run number is meaningful only when the run numbers are the same
         fRunNumber    = 0;
         fSubRunNumber = -1;
      } else if(fFirstRunNumber != 0 && fLastRunNumber != 0 && fFirstRunNumber != fLastRunNumber) {
         // if we already have a (good) range of runs, check if runinfo fits at the beginning or the end
         if(runinfo->fRunNumber + 1 == fFirstRunNumber) {
            if(verbose) { std::cout << "found another run (" << runinfo->fRunNumber << ") before first run (" << fFirstRunNumber << ")" << std::endl; }
            // use runinfo as first run
            fFirstRunNumber = runinfo->fRunNumber;
            fRunStart       = runinfo->fRunStart;
         } else if(runinfo->fRunNumber - 1 == fLastRunNumber) {
            if(verbose) { std::cout << "found another run (" << runinfo->fRunNumber << ") after last run (" << fLastRunNumber << ")" << std::endl; }
            // use runinfo as last run
            fLastRunNumber = runinfo->fRunNumber;
            fRunStop       = runinfo->fRunStop;
         } else if(runinfo->fRunNumber == fFirstRunNumber || runinfo->fRunNumber == fLastRunNumber) {
            if(verbose) { std::cout << "found another sub(?) run part of runs (" << fFirstRunNumber << " - " << fLastRunNumber << ")" << std::endl; }
            // found probably another subrun of a run already added
            // since we do not keep track of all subruns we have to assume this is in order
            // so we only update the run start or stop if necessary
            if(verbose) { std::cout << "changing run start/stop from " << std::setw(16) << fRunStart << "/" << std::setw(16) << fRunStop << " to "; }
            if(fRunStop < runinfo->fRunStop) { fRunStop = runinfo->fRunStop; }
            if(fRunStart > runinfo->fRunStart) { fRunStart = runinfo->fRunStart; }
            if(verbose) { std::cout << std::setw(16) << fRunStart << "/" << std::setw(16) << fRunStop << std::endl; }
         } else {
            if(verbose) { std::cout << "found another run (" << runinfo->fRunNumber << ") non-consecutive to runs (" << fFirstRunNumber << " - " << fLastRunNumber << ")" << std::endl; }
            // run start and stop don't make a lot of sense with non-consecutive runs (?)
            fRunStart = 0.;
            fRunStop  = 0.;
            // still need to keep some kind of information about the runs (e.g. to create filenames)
            // by keeping first and last the exact same it is still obvious that these are not consecutive runs
            fLastRunNumber = fFirstRunNumber;
         }
      } else {
         // the run number is zero, and we do not have a (good) range, so there is nothing to do.
         if(verbose) { std::cout << "found another run (" << runinfo->fRunNumber << ") non-consecutive run (" << fFirstRunNumber << " - " << fLastRunNumber << ")" << std::endl; }
      }
   } else if(fSubRunNumber != -1) {
      // check if the added sub run is an increment of the current run number
      if(runinfo->fSubRunNumber + 1 == fSubRunNumber) {
         if(verbose) { std::cout << "found second sub run (" << runinfo->fSubRunNumber << ") before current sub run (" << fSubRunNumber << ")" << std::endl; }
         // if the run numbers are the same and we have subsequent sub runs we can update the run start
         fRunStart          = runinfo->fRunStart;
         fFirstSubRunNumber = runinfo->fSubRunNumber;
         fLastSubRunNumber  = fSubRunNumber;
      } else if(runinfo->fSubRunNumber - 1 == fSubRunNumber) {
         if(verbose) { std::cout << "found second sub run (" << runinfo->fSubRunNumber << ") after current sub run (" << fSubRunNumber << ")" << std::endl; }
         // if the run numbers are the same and we have subsequent sub runs we can update the run stop
         fRunStop           = runinfo->fRunStop;
         fFirstSubRunNumber = fSubRunNumber;
         fLastSubRunNumber  = runinfo->fSubRunNumber;
      } else {
         if(verbose) { std::cout << "found second sub run (" << runinfo->fSubRunNumber << ") non-consecutive to current sub run (" << fSubRunNumber << ")" << std::endl; }
         // with multiple non-sequential subruns added, the sub run number and start/stop have no meaning anymore
         fRunStart          = 0.;
         fRunStop           = 0.;
         fFirstSubRunNumber = -1;
         fLastSubRunNumber  = -1;
      }
      // sub run number is only meaningful if it's the only sub run
      fSubRunNumber = -1;
   } else {
      // we have the same run with a range of sub-runs already added, so check if this one fits at the end or the beginning
      if(runinfo->fSubRunNumber + 1 == fFirstSubRunNumber) {
         if(verbose) { std::cout << "found another sub run (" << runinfo->fSubRunNumber << ") before first sub run (" << fFirstSubRunNumber << ")" << std::endl; }
         // use runinfo as first run
         fFirstSubRunNumber = runinfo->fSubRunNumber;
         fRunStart          = runinfo->fRunStart;
      } else if(runinfo->fSubRunNumber - 1 == fLastSubRunNumber) {
         if(verbose) { std::cout << "found another sub run (" << runinfo->fSubRunNumber << ") after last sub run (" << fLastSubRunNumber << ")" << std::endl; }
         // use runinfo as last run
         fLastSubRunNumber = runinfo->fSubRunNumber;
         fRunStop          = runinfo->fRunStop;
      } else {
         if(verbose) { std::cout << "found another sub run (" << runinfo->fSubRunNumber << ") non-consecutive to sub runs (" << fFirstSubRunNumber << " - " << fLastSubRunNumber << ")" << std::endl; }
         // with multiple non-sequential subruns added, the sub run number and start/stop have no meaning anymore
         fRunStart          = 0.;
         fRunStop           = 0.;
         fFirstSubRunNumber = -1;
         fLastSubRunNumber  = -1;
      }
   }
}

void TRunInfo::PrintRunList() const
{
   if(fRunList.empty()) {
      std::cout << "No runs added to list of runs!" << std::endl;
      return;
   }
   std::cout << "Got " << fRunList.size() << " runs:" << std::endl;
   for(auto pair : fRunList) {
      std::cout << std::setw(5) << std::setfill('0') << pair.first << "_" << std::setw(3) << pair.second << std::setfill(' ') << std::endl;
   }
}

std::string TRunInfo::ListOfMissingRuns(bool all) const
{
   /// Outputs a comma separated list of all runs missing between fFirstRunNumber and fLastRunNumber.
   /// If no runs are missing prints "none".

   if(fRunList.empty()) {
      return {"no run list -> none missing?"};
   }

   std::ostringstream result;

   // loop over all runs between the first and the last one (we know that these two are included)
   // and check if the run is in the list of runs (or any subrun that is part of this run)
   for(int run = fRunList.begin()->first; run < fRunList.rbegin()->first; ++run) {
      if(std::find_if(fRunList.begin(), fRunList.end(), [&run](std::pair<int, int> i) { return run == i.first; }) == fRunList.end()) {
         if(!result.str().empty()) {
            result << ", ";
         }
         result << std::setw(5) << std::setfill('0') << run;
      }
   }

   // if we found no missing runs, we print "none"
   if(result.str().empty()) {
      return {"none"};
   }

 	// unless the "all" flag is set, we limit ourself to printing 140 characters (should be 20 runs?) 
	std::cout << " current string length is " << result.str().length() << " and all is set to " << (all?"true":"false") << std::endl;
	if(!all && result.str().length() > 140) {
			  result.str(result.str().substr(0, 140));
			  result.seekp(0, std::ios_base::end);
			  result << " ... and more";
			  std::cout << " limited string length is " << result.str().length() << std::endl;
	}
   return result.str();
}

void TRunInfo::PrintVersion() const
{
   if(fVersion.empty()) {
      std::cout << YELLOW << "Unknown version, this file was probably generated with TRunInfo version 17 or older" << RESET_COLOR << std::endl;
   } else {
      std::cout << "This file was generated with GRSISort version " << fVersion << " (" << fFullVersion << ")" << std::endl
                << "From " << fDate << std::endl
                << "Using the parser library version " << fLibraryVersion << " from " << fLibraryPath << std::endl;
   }
}

std::string TRunInfo::CreateLabel(bool quiet)
{
   /// This function creates a label/string based on the run number and the subrun number.
   Int_t runNumber    = RunNumber();
   Int_t subRunNumber = SubRunNumber();

   std::string result;
   if(!quiet) { std::cout << "Using run number " << runNumber << ", sub run number " << subRunNumber << ", first/last run number " << FirstRunNumber() << "/" << LastRunNumber() << ", and first/last sub run number" << FirstSubRunNumber() << "/" << LastSubRunNumber() << std::endl; }
   if(runNumber != 0 && subRunNumber != -1) {
      // both run and subrun number set => single file processed
      result = Form("%05d_%03d", runNumber, subRunNumber);
   } else if(runNumber != 0) {
      // multiple subruns of a single run
      // we could check if first and last sub run number are both -1 (which is non-consecutive runs or not initialized, the latter would mean single file w/o a subrun number, like ILL data)
      result = Form("%05d_%03d-%03d", runNumber, FirstSubRunNumber(), LastSubRunNumber());
   } else {
      // multiple runs
      result = Form("%05d-%05d", FirstRunNumber(), LastRunNumber());
   }
   if(!quiet) { std::cout << "Created label " << result << std::endl; }

   return result;
}

//void TRunInfo::Streamer(TBuffer &R__b)
//{
//   /// Stream an object of class TRunInfo.
//	/// Explicitly writes and reads the static members as well!
//
//   if(R__b.IsReading()) {
//      R__b.ReadClassBuffer(TRunInfo::Class(),this);
//		Version_t R__v = R__b.ReadVersion();
//		std::cout << "Got R__v = " << R__v << std::endl;
//		if(R__v > 17) {
//			R__b.ReadStdString(fVersion);
//			R__b.ReadStdString(fFullVersion);
//			R__b.ReadStdString(fDate);
//			R__b.ReadStdString(fLibraryVersion);
//			R__b.ReadStdString(fLibraryPath);
//		} else {
//			fVersion = "unknown";
//			fFullVersion = "unknown";
//			fDate = "unknown";
//			fLibraryVersion = "unknown";
//			fLibraryPath = "unknown";
//		}
//	} else {
//		R__b.WriteClassBuffer(TRunInfo::Class(),this);
//		R__b.WriteVersion(TRunInfo::Class(), true);
//		R__b.WriteStdString(fVersion);
//		R__b.WriteStdString(fFullVersion);
//		R__b.WriteStdString(fDate);
//		R__b.WriteStdString(fLibraryVersion);
//		R__b.WriteStdString(fLibraryPath);
//	}
//}
