#include "TGRSIOptions.h"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "TEnv.h"
#include "TKey.h"
#include "TSystem.h"

#include "Globals.h"
#include "ArgParser.h"
#include "DynamicLibrary.h"
#include "TGRSIUtilities.h"
#include "GRootCommands.h"

TGRSIOptions* TGRSIOptions::fGRSIOptions = nullptr;
TAnalysisOptions* TGRSIOptions::fAnalysisOptions = new TAnalysisOptions;

TGRSIOptions* TGRSIOptions::Get(int argc, char** argv)
{
   // The Getter for the singleton TGRSIOptions. This makes it
   // so there is only ever one instance of the options during
   // a session and it can be accessed from anywhere during that
   // session.
   if(fGRSIOptions == nullptr) {
		fGRSIOptions = new TGRSIOptions(argc, argv);
	}
   return fGRSIOptions;
}

TGRSIOptions::TGRSIOptions(int argc, char** argv) : fShouldExit(false)
{
   /// Ctor used when interpreter is initialized
   Load(argc, argv);
}

void TGRSIOptions::Clear(Option_t*)
{
   /// Clears all of the variables in the TGRSIOptions
   fInputFiles.clear();
   fInputRootFiles.clear();
   fInputCalFiles.clear();
   fInputOdbFiles.clear();
   fExternalRunInfo.clear();
   fMacroFiles.clear();

   fInputCutFiles.clear();
   fInputValFiles.clear();
   fInputWinFiles.clear();
   fInputRing = "";

   fOutputFragmentFile          = "";
   fOutputAnalysisFile          = "";
   fOutputFilteredFile          = "";
   fOutputFragmentHistogramFile = "";
   fOutputAnalysisHistogramFile = "";

   fFragmentHistogramLib = "";
   fAnalysisHistogramLib = "";
   fCompiledFilterFile   = "";

   fOptionsFile.clear();

   fLogFile = "grsisort.log";

   fCloseAfterSort = false;
   fLogErrors      = false;
   fUseMidFileOdb  = false;

   fMakeAnalysisTree = false;
   fReadingMaterial  = false;
   fIgnoreFileOdb    = false;
   fIgnoreOdbChannels= false;
	fDownscaling      = 1;

   fIgnoreScaler     = false;
   fIgnoreEpics      = false;
   fWriteFragmentTree= false;
   fWriteBadFrags    = false;
   fWriteDiagnostics = false;
	fWordOffset       = 1;

   fBatch = false;

   fShowedVersion = false;
   fShowLogo      = false;
   fSortRaw       = true;
   fExtractWaves  = false;
   fIsOnline      = false;
   fStartGui      = false;
   fMakeHistos    = false;
   fSortMultiple  = false;
   fDebug         = false;

   fFragmentWriteQueueSize = 100000;
   fAnalysisWriteQueueSize = 100000;

	fNumberOfClients = 2;

	fNumberOfEvents = 0;

   fSkipInputSort = false;

   fSeparateOutOfOrder    = false;

   fShouldExit = false;

   fColumnWidth         = 20;
   fStatusWidth         = 120;
   fStatusInterval      = 10;
   fLongFileDescription = false;

	fAnalysisOptions->Clear();

   // Proof only
   fMaxWorkers   = -1;
   fSelectorOnly = false;

   fHelp          = false;

	fParserLibrary.clear();
}

void TGRSIOptions::Print(Option_t*) const
{
   /// Print the current status of TGRSIOptions, includes all names, lists and flags
   std::cout<<"fCloseAfterSort: "<<fCloseAfterSort<<std::endl
            <<"fLogErrors: "<<fLogErrors<<std::endl
            <<"fUseMidFileOdb: "<<fUseMidFileOdb<<std::endl
            <<"fSuppressErrors: "<<fSuppressErrors<<std::endl
            <<"fReconstructTimeStamp: "<<fReconstructTimeStamp<<std::endl
            <<std::endl
            <<"fMakeAnalysisTree: "<<fMakeAnalysisTree<<std::endl
            <<"fReadingMaterial;: "<<fReadingMaterial<<std::endl
            <<"fIgnoreFileOdb: "<<fIgnoreFileOdb<<std::endl
            <<"fIgnoreOdbChannels: "<<fIgnoreOdbChannels<<std::endl
            <<"fDownscaling: "<<fDownscaling<<std::endl
            <<std::endl
            <<"fIgnoreScaler: "<<fIgnoreScaler<<std::endl
            <<"fIgnoreEpics: "<<fIgnoreEpics<<std::endl
            <<"fWriteFragmentTree: "<<fWriteFragmentTree<<std::endl
            <<"fWriteBadFrags: "<<fWriteBadFrags<<std::endl
            <<"fWriteDiagnostics: "<<fWriteDiagnostics<<std::endl
            <<"fWordOffset: "<<fWordOffset<<std::endl
            <<std::endl
            <<"fBatch: "<<fBatch<<std::endl
            <<std::endl
            <<"fShowedVersion: "<<fShowedVersion<<std::endl
            <<"fShowLogo: "<<fShowLogo<<std::endl
            <<"fSortRaw: "<<fSortRaw<<std::endl
            <<"fExtractWaves;: "<<fExtractWaves<<std::endl
            <<"fIsOnline: "<<fIsOnline<<std::endl
            <<"fStartGui: "<<fStartGui<<std::endl
            <<"fMakeHistos: "<<fMakeHistos<<std::endl
            <<"fSortMultiple: "<<fSortMultiple<<std::endl
            <<"fDebug: "<<fDebug<<std::endl
            <<"fLogFile: "<<fLogFile<<std::endl
            <<std::endl
            <<"fFragmentWriteQueueSize: "<<fFragmentWriteQueueSize<<std::endl
            <<"fAnalysisWriteQueueSize: "<<fAnalysisWriteQueueSize<<std::endl
            <<std::endl
            <<"fSkipInputSort: "<<fSkipInputSort<<std::endl
            <<"fSortDepth: "<<fSortDepth<<std::endl
            <<std::endl
            <<"fSeparateOutOfOrder: "<<fSeparateOutOfOrder<<std::endl
            <<std::endl
            <<"fShouldExit: "<<fShouldExit<<std::endl
            <<std::endl
            <<"fColumnWidth: "<<fColumnWidth<<std::endl
            <<"fStatusWidth: "<<fStatusWidth<<std::endl
            <<"fStatusInterval: "<<fStatusInterval<<std::endl
            <<"fLongFileDescription: "<<fLongFileDescription<<std::endl
				<<std::endl
            <<"fMaxWorkers: "<<fMaxWorkers<<std::endl
            <<"fSelectorOnly: "<<fSelectorOnly<<std::endl
				<<std::endl
				<<"fHelp: "<<fHelp<<std::endl
				<<std::endl
				<<"fParserLibrary: "<<fParserLibrary<<std::endl;

				fAnalysisOptions->Print();
}

void TGRSIOptions::Load(int argc, char** argv)
{
   /// This checks all of the options provided to GRSISort. This also loads in some
   /// libraries in order to do on the fly histogramming.
   Clear();
   fFragmentHistogramLib = gEnv->GetValue("GRSI.FragmentHistLib", "");
   fAnalysisHistogramLib = gEnv->GetValue("GRSI.AnalysisHistLib", "");

	fParserLibrary = gEnv->GetValue("GRSI.ParserLibrary","");

   // Load default TChannels, if specified.
   {
      std::string default_calfile = gEnv->GetValue("GRSI.CalFile", "");
      if(default_calfile.length() != 0u) {
         fInputCalFiles.push_back(default_calfile);
      }
   }

   // Load default GValues, if specified.
   {
      std::string default_valfile = gEnv->GetValue("GRSI.ValFile", "");
      if(default_valfile.length() != 0u) {
         fInputValFiles.push_back(default_valfile);
      }
   }

	// Get name of the program calling this function (removing any path from the name)
	std::string program;
	if(argc > 0) program = argv[0];
	else program = "unknown";
	size_t lastSlash = program.rfind('/');
	if(lastSlash != std::string::npos) {
		program.erase(0, lastSlash+1);
	}

   ArgParser parser;
	bool useRecommendedFlags = false;

   std::vector<std::string> input_files;
   std::string              default_file_format;

   // parser.option() will initialize boolean values to false.

	// general options
	// these options are all to be set directly on the first parsing pass, so we set the firstPass flag to true
	parser.default_option(&input_files, true).description("Input file(s)");
   parser.option("h help ?", &fHelp, true).description("Show this help message");
   parser.option("v version", &fShowedVersion, true).description("Show the version of GRSISort");

	// analysis options, these options are to be parsed on the second pass, so firstPass is set to false
	parser.option("build-window", &fAnalysisOptions->fBuildWindow, false)
		.description("Build window, timestamp units").colour(DCYAN);
	parser.option("build-events-by-timestamp", &fAnalysisOptions->fBuildEventsByTimeStamp, false)
		.description("Build events by timestamp w/o using CFD").colour(DCYAN);
	parser.option("addback-window", &fAnalysisOptions->fAddbackWindow, false)
		.description("Addback window, time in ns").colour(DCYAN);
	parser.option("suppression-window", &fAnalysisOptions->fSuppressionWindow, false)
		.description("BGO suppression window, time in ns").colour(DCYAN);
	parser.option("suppression-energy", &fAnalysisOptions->fSuppressionEnergy, false)
		.description("Minimum BGO energy for suppression").colour(DCYAN);
	parser.option("static-window", &fAnalysisOptions->fStaticWindow, false)
		.description("Use static window for event building").colour(DCYAN);
	parser.option("waveform-fitting", &fAnalysisOptions->fWaveformFitting, false)
		.description("Fit waveforms using SFU algorithms").colour(DCYAN);
	parser.option("is-correcting-cross-talk", &fAnalysisOptions->fIsCorrectingCrossTalk, false)
		.takes_argument().description("Correct cross-talk").colour(DCYAN);

	// program specific options
	if(program.compare("grsisort") == 0) {
		// grsisort options
		parser.option("recommended", &useRecommendedFlags, true).description("Use recommended flags (those in " DGREEN "dark green" GREEN ")").colour(GREEN);
		parser.option("output-fragment-tree", &fOutputFragmentFile, true).description("Filename of output fragment tree");
		parser.option("output-analysis-tree", &fOutputAnalysisFile, true).description("Filename of output analysis tree");
		parser.option("output-fragment-hists", &fOutputFragmentHistogramFile, true)
			.description("Filename of output fragment hists");
		parser.option("output-analysis-hists", &fOutputAnalysisHistogramFile, true)
			.description("Filename of output analysis hists");

		parser.option("a", &fMakeAnalysisTree, true).description("Make the analysis tree").colour(DGREEN);
		parser.option("H histos", &fMakeHistos, true).description("Attempt to run events through MakeHisto lib");
		parser.option("g start-gui", &fStartGui, true).description("Start the gui at program start");
		parser.option("b batch", &fBatch, true).description("Run in batch mode");

		parser.option("sort-depth", &fSortDepth, true)
			.description("Number of events to hold when sorting by time/trigger_id")
			.default_value(200000);

		parser.option("q quit", &fCloseAfterSort, true).description("Quit after completing the sort").colour(DGREEN);
		parser.option("l no-logo", &fShowLogo, true).description("Inhibit the startup logo")
			.default_value(true).colour(DGREEN);
		parser.option("w extract-waves", &fExtractWaves, true)
			.description("Extract wave forms to data class when available.")
			.default_value(false);
		parser.option("d debug", &fDebug, true)
			.description("Write debug information to output/file, e.g. enables writing of TDescantDebug at analysis stage")
			.default_value(false);
		parser.option("write-diagnostics", &fWriteDiagnostics, true).description("Write Parsing/SortingDiagnostics to root-file")
			.colour(DGREEN);
		parser.option("word-count-offset", &fWordOffset, true)
			.description("Offset to the word count in the GRIFFIN header word, default is -1 (disabled).")
			.default_value(1).colour(DGREEN);
		parser.option("log-errors", &fLogErrors, true);
		parser.option("reading-material", &fReadingMaterial, true);
		parser.option("write-fragment-tree write-frag-tree", &fWriteFragmentTree, true)
			.description("Write fragment tree.").colour(DGREEN);
		parser.option("bad-frags write-bad-frags bad-fragments write-bad-fragments", &fWriteBadFrags, true)
			.description("Write fragments that failed parsing to BadFragmentTree").colour(DGREEN);
		parser.option("separate-out-of-order", &fSeparateOutOfOrder, true)
			.description("Write out-of-order fragments to a separate tree at the sorting stage")
			.default_value(false).colour(DGREEN);
		parser.option("ignore-odb", &fIgnoreFileOdb, true);
		parser.option("ignore-odb-channels", &fIgnoreOdbChannels, true);
		parser.option("downscaling", &fDownscaling, true).description("Downscaling factor for raw events to be processed").default_value(1);
		parser.option("ignore-epics", &fIgnoreEpics, true);
		parser.option("ignore-scaler", &fIgnoreScaler, true);
		parser.option("suppress-error suppress-errors suppress_error suppress_errors", &fSuppressErrors, true)
			.description("Suppress error output from parsing").colour(DGREEN);
		parser.option("reconstruct-timestamp reconstruct-time-stamp", &fReconstructTimeStamp, true)
			.description("Reconstruct missing high bits of timestamp").colour(DGREEN);
		parser.option("skip-input-sort", &fSkipInputSort, true)
			.description("Skip sorting fragments before building events (default is false)").default_value(false);

		parser.option("fragment-size", &fFragmentWriteQueueSize, true)
			.description("Size of fragment write queue")
			.default_value(10000000);
		parser.option("analysis-size", &fAnalysisWriteQueueSize, true)
			.description("Size of analysis write queue")
			.default_value(1000000);

		parser.option("column-width", &fColumnWidth, true).description("Width of one column of status").default_value(20);
		parser.option("status-width", &fStatusWidth, true)
			.description("Number of characters to be used for status output")
			.default_value(120);
		parser.option("status-interval", &fStatusInterval, true)
			.description(
					"Seconds between each detailed status output (each a new line), non-positive numbers mean no detailed status")
			.default_value(10);

		parser.option("write-clients", &fNumberOfClients, true)
			.description("Number of clients used to write analysis tree").default_value(2);
	} else if(program.compare("grsiproof") == 0) {
		// Proof only parser options
		parser.option("max-workers", &fMaxWorkers, true)
			.description("Max number of nodes to use when running a grsiproof session")
			.default_value(-1);

		parser.option("selector-only", &fSelectorOnly, true)
			.description("Turns off PROOF to run a selector on the main thread");
		parser.option("log-file", &fLogFile, true).description("File logs from grsiproof are written to");
	}

	parser.option("max-events", &fNumberOfEvents, true)
		.description("Maximum number of (midas, lst, or tdr) events read").default_value(0);

   // look for any arguments ending with .info, pass to parser.
   for(int i = 0; i < argc; i++) {
      std::string filename = argv[i];
      if(DetermineFileType(filename) == kFileType::CONFIG_FILE) {
         try {
            parser.parse_file(filename);
         } catch(ParseError& e) {
            std::cerr<<"ERROR: "<<e.what()<<"\n"<<parser<<std::endl;
            throw;
         }
      }
   }

   // Look at the command line.
   try {
      parser.parse(argc, argv, true);
   } catch(ParseError& e) {
      std::cerr<<"ERROR: "<<e.what()<<"\n"<<parser<<std::endl;
		throw;
   }

   // Print version if requested
   if(fShowedVersion) {
      Version();
      fShouldExit = true;
   }

	// print help if required
	if(fHelp) {
		std::cout<<parser<<std::endl;
		fShouldExit = true;
	}

   if(fOutputFragmentHistogramFile.length() > 0 && fOutputFragmentHistogramFile != "none") {
      fMakeHistos = true;
   }
   if(fOutputAnalysisHistogramFile.length() > 0 && fOutputAnalysisHistogramFile != "none") {
      fMakeHistos = true;
   }

   for(auto& file : input_files) {
      FileAutoDetect(file);
   }

	// load any additional parser library
	if(!fParserLibrary.empty()) {
		gSystem->Load(fParserLibrary.c_str());
	}

	// read analysis options from input file(s)
	for(const std::string& file : fInputRootFiles) {
		fAnalysisOptions->ReadFromFile(file);
	}
	// parse analysis options from command line options 
   try {
      parser.parse(argc, argv, false);
   } catch(ParseError& e) {
      std::cerr<<"ERROR: "<<e.what()<<"\n"<<parser<<std::endl;
		throw;
   }
	if(useRecommendedFlags) {
		fMakeAnalysisTree = true;
		fShowLogo = false;
		fCloseAfterSort = true;
		fWriteDiagnostics = true;
		fWriteFragmentTree = true;
		fWriteBadFrags = true;
		fSeparateOutOfOrder = true;
		fSuppressErrors = true;
		fReconstructTimeStamp = true;
		fWordOffset = -1;
	}
}

kFileType TGRSIOptions::DetermineFileType(const std::string& filename) const
{
   size_t      dot_pos = filename.find_last_of('.');
   size_t    slash_pos = filename.find_last_of('/');
	// if we didn't find a . (or if it was before the last /) we don't have any extension
	// => so it's a TDR file
	if(dot_pos == std::string::npos || (dot_pos < slash_pos && slash_pos != std::string::npos)) {
      return kFileType::TDR_FILE;
   }
   std::string ext     = filename.substr(dot_pos + 1);

	//check if this is a zipped file and if so get the extension before the zip-extension
   bool isZipped = (ext == "gz") || (ext == "bz2") || (ext == "zip");
   if(isZipped) {
      std::string remaining = filename.substr(0, dot_pos);
      ext                   = remaining.substr(remaining.find_last_of('.') + 1);
   }

   if(ext == "mid") {
      return kFileType::MIDAS_FILE;
   } 
	if(ext == "lst") {
      return kFileType::LST_FILE;
   }
	if(ext == "evt") {
      return kFileType::NSCL_EVT;
   }
	if(ext == "cal") {
      return kFileType::CALIBRATED;
   }
	if(ext == "root") {
      return kFileType::ROOT_DATA;
   }
	if((ext == "c") || (ext == "C") || (ext == "c+") || (ext == "C+") || (ext == "c++") || (ext == "C++")) {
      return kFileType::ROOT_MACRO;
   }
	if(ext == "dat" || ext == "cvt") {
      if(filename.find("GlobalRaw") != std::string::npos) {
			return kFileType::GRETINA_MODE3;
		}
      return kFileType::GRETINA_MODE2;
   }
	if(ext == "hist") {
      return kFileType::GUI_HIST_FILE;
   }
	if(ext == "so") {
      return kFileType::COMPILED_SHARED_LIBRARY;
   }
	if(ext == "info") {
      return kFileType::CONFIG_FILE;
   }
	if(ext == "val") {
      return kFileType::GVALUE;
   }
	if(ext == "win") {
      return kFileType::PRESETWINDOW;
   }
	if(ext == "cuts") {
      return kFileType::CUTS_FILE;
   }
	if(ext == "xml") {
      return kFileType::XML_FILE;
   }
	
	// strip possible parenthese with arguments for the script from the extension
   size_t      opening_pos = ext.find_first_of('(');
	if(opening_pos != std::string::npos) {
		ext = ext.substr(0,opening_pos);
		if((ext == "c") || (ext == "C") || (ext == "c+") || (ext == "C+") || (ext == "c++") || (ext == "C++")) {
			return kFileType::ROOT_MACRO;
		}
	}
	return kFileType::UNKNOWN_FILETYPE;
}

bool TGRSIOptions::FileAutoDetect(const std::string& filename)
{
   /// Detects the type of file provided on the command line. This uses the extension to determine
   /// the type of the file. Once the type is determined, the file is sent to the appropriate list
   /// in TGRSIOptions. This is also smart enough to dynamically link histogramming libraries.
   switch(DetermineFileType(filename)) {
   case kFileType::NSCL_EVT:
   case kFileType::GRETINA_MODE2:
   case kFileType::GRETINA_MODE3:
   case kFileType::MIDAS_FILE: fInputFiles.push_back(filename); return true;

   case kFileType::LST_FILE: fInputFiles.push_back(filename); return true;

   case kFileType::TDR_FILE: fInputFiles.push_back(filename); return true;

   case kFileType::ROOT_DATA: fInputRootFiles.push_back(filename); return true;

   case kFileType::ROOT_MACRO: fMacroFiles.push_back(filename); return true;

   case kFileType::CALIBRATED: fInputCalFiles.push_back(filename); return true;

   case kFileType::COMPILED_SHARED_LIBRARY: {

      bool           used = false;
      DynamicLibrary lib(filename);
      if(lib.GetSymbol("MakeFragmentHistograms") != nullptr) {
         fFragmentHistogramLib = filename;
         used                  = true;
      }
      if(lib.GetSymbol("MakeAnalysisHistograms") != nullptr) {
         fAnalysisHistogramLib = filename;
         used                  = true;
      }
      if(lib.GetSymbol("CreateParser") != nullptr && lib.GetSymbol("DestroyParser") != nullptr &&
			lib.GetSymbol("CreateFile")   != nullptr && lib.GetSymbol("DestroyFile")   != nullptr) {
         fParserLibrary        = filename;
         used                  = true;
      }
      if(!used) {
         std::cerr<<filename<<" did not contain MakeFragmentHistograms() or MakeAnalysisHistograms()"<<std::endl;
      }
      return true;
   }

   case kFileType::GVALUE: fInputValFiles.push_back(filename); return true;

   case kFileType::PRESETWINDOW: fInputWinFiles.push_back(filename); return true;

   case kFileType::CUTS_FILE: fInputCutFiles.push_back(filename); return true;

   case kFileType::CONFIG_FILE: return false;

   case kFileType::XML_FILE: fInputOdbFiles.push_back(filename); return true;

   case kFileType::UNKNOWN_FILETYPE:
   default: std::cout<<"\tDiscarding unknown file: "<<filename<<std::endl; return false;
   }
}

std::string TGRSIOptions::GenerateOutputFilename(const std::string&)
{
   /// Currently does nothing
   return "temp.root";
}

std::string TGRSIOptions::GenerateOutputFilename(const std::vector<std::string>&)
{
   /// Currently does nothing
   return "temp_from_multi.root";
}

bool TGRSIOptions::WriteToFile(TFile* file)
{
   /// Writes options information to the tree
   // Maintain old gDirectory info
   bool        success = true;
   TDirectory* oldDir  = gDirectory;

   if(file == nullptr) {
		file = gDirectory->GetFile();
	}
   file->cd();
   std::string oldoption = std::string(file->GetOption());
   if(oldoption == "READ") {
      file->ReOpen("UPDATE");
   }
   if(!gDirectory) { // we don't compare to nullptr here, as ROOT >= 6.24.00 uses the TDirectoryAtomicAdapter structure with a bool() operator
		std::cout<<"No file opened to write to."<<std::endl;
      success = false;
   } else {
      Get()->Write("GRSIOptions", TObject::kOverwrite);
		fAnalysisOptions->WriteToFile(file);
   }

   std::cout<<"Writing TGRSIOptions to "<<gDirectory->GetFile()->GetName()<<std::endl;
   if(oldoption == "READ") {
      std::cout<<"  Returning "<<gDirectory->GetFile()->GetName()<<" to \""<<oldoption<<"\" mode."<<std::endl;
      file->ReOpen("READ");
   }
   oldDir->cd(); // Go back to original gDirectory

   return success;
}

void TGRSIOptions::SetOptions(TGRSIOptions* tmp)
{
   // Sets the TGRSIOptions to the info passes as tmp.
   if((fGRSIOptions != nullptr) && (tmp != fGRSIOptions)) {
		delete fGRSIOptions;
	}
   fGRSIOptions = tmp;
}

Bool_t TGRSIOptions::ReadFromFile(TFile* file)
{
   TDirectory* oldDir = gDirectory;
   if(file != nullptr) {
      file->cd();
   }

   if(gDirectory->GetFile() == nullptr) {
      std::cout<<"File does not exist"<<std::endl;
      oldDir->cd();
      return false;
   }

   file = gDirectory->GetFile();

   TList* list = file->GetListOfKeys();
   TIter  iter(list);
	std::cout<<"Reading options from file: "<<CYAN<<file->GetName()<<RESET_COLOR<<std::endl;
   while(TKey* key = static_cast<TKey*>(iter.Next())) {
      if((key == nullptr) || (strcmp(key->GetClassName(), "TGRSIOptions") != 0)) {
			continue;
		}

      TGRSIOptions::SetOptions(static_cast<TGRSIOptions*>(key->ReadObj()));
      oldDir->cd();
      return true;
   }
   oldDir->cd();

   return false;
}
