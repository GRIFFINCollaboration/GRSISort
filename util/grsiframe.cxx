#include <iostream>
#include <string>
#include <vector>

#include "RVersion.h" // IWYU pragma: keep
#if ROOT_VERSION_CODE >= ROOT_VERSION(6, 14, 0)
#include "TFile.h"
#include "TChain.h"
#include "TEnv.h"
#include "TStopwatch.h"

#include "TGRSIOptions.h"
#include "TParserLibrary.h"
#include "TGRSIFrame.h"
#include "TRedirect.h"
#include "TRunInfo.h"

int main(int argc, char** argv)
{
   auto* stopwatch = new TStopwatch;

   TGRSIOptions* opt = nullptr;
   try {
      std::string grsi_path = getenv("GRSISYS");   // Finds the GRSISYS path to be used by other parts of the grsisort code
      if(!grsi_path.empty()) {
         grsi_path += "/";
      }
      // Read in grsirc in the GRSISYS directory to set user defined options on grsisort startup
      grsi_path += ".grsirc";
      gEnv->ReadFile(grsi_path.c_str(), kEnvChange);
      opt = TGRSIOptions::Get(argc, argv);
      if(opt->ShouldExit()) {
         return 0;
      }
   } catch(ParseError& e) {
      return 1;
   }

   // load parser library if provided
   if(!opt->ParserLibrary().empty()) {
      try {
         TParserLibrary::Get()->Load();
      } catch(std::runtime_error& e) {
         // if we failed to load the library, we quit
         std::cerr << DRED << e.what() << RESET_COLOR << std::endl;
         return 1;
      }
   } else {
      std::cerr << "No parser library set!" << std::endl;
      return 1;
   }

   // check that we have input files and a macro
   if(opt->RootInputFiles().empty()) {
      std::cerr << "No input files provided!" << std::endl;
      return 1;
   }
   if(opt->DataFrameLibrary().empty()) {
      std::cerr << "No dataframe library (*.cxx file or *.so library) provided!" << std::endl;
      return 1;
   }

   // read the run info from all input files
   bool first = true;
   for(const auto& fileName : opt->RootInputFiles()) {
      TFile* file = TFile::Open(fileName.c_str());
      if(first) {
         first = false;
         TRunInfo::ReadInfoFromFile(file);
      } else {
         TRunInfo::AddCurrent();
      }
   }
   // if this is a single sub run or consecutive sub runs from a single run, we re-calculate the run length
   // this is to avoid small mistakes where the start time of the next sub run is one second after the stop time of the current sub run
   // otherwise this function call does nothing:
   TRunInfo::SetRunLength();

   // determine the name of the helper (from the provided helper library) to create a redirect of stdout
   std::string logFileName = opt->DataFrameLibrary();
   logFileName             = logFileName.substr(logFileName.find_last_of('/') + 1);   // strip everything before the last slash
   logFileName             = logFileName.substr(0, logFileName.find_last_of('.'));    // strip extension
   logFileName.append(TRunInfo::CreateLabel(true));
   logFileName.append(".log");

   // start redirect of stdout only w/o appending (ends when we delete it)
   std::cout << "redirecting stdout to " << logFileName << std::endl;
   auto* redirect = new TRedirect(logFileName.c_str(), nullptr, false);

   // this reads and compiles the user code
   TGRSIFrame frame;
   // run it and write the results
   frame.Run(redirect);

   // re-start redirect of stdout only w/ appending if needed (ends when we delete it)
   if(redirect == nullptr) {
      redirect = new TRedirect(logFileName.c_str(), nullptr, true);
   }

   // print time it took to run
   double realTime = stopwatch->RealTime();
   int    hour     = static_cast<int>(realTime / 3600);
   realTime -= hour * 3600;
   int min = static_cast<int>(realTime / 60);
   realTime -= min * 60;
   // print goes to log file due to redirect, so we don't need colours here
   std::cout << std::endl
             << "Done after " << hour << ":" << std::setfill('0') << std::setw(2) << min << ":"
             << std::setprecision(3) << std::fixed << realTime << " h:m:s"
             << std::endl;

   // delete the redirect and print again to true stdout
   delete redirect;

   std::cout << DMAGENTA << std::endl
             << "Done after " << hour << ":" << std::setfill('0') << std::setw(2) << min << ":"
             << std::setprecision(3) << std::fixed << realTime << " h:m:s"
             << RESET_COLOR << std::endl;

   return 0;
}
#else
int main(int, char** argv)
{
   std::cerr << argv[0] << ": need at least ROOT version 6.14" << std::endl;
   return 1;
}
#endif
