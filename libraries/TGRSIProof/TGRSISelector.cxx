// The class definition in TGRSISelector.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("TGRSISelector.C")
// Root > T->Process("TGRSISelector.C","some options")
// Root > T->Process("TGRSISelector.C+")
//

#include "TGRSIOptions.h"
#include "TRunInfo.h"
#include "TGRSISelector.h"
#include "GValue.h"
#include "TParserLibrary.h"

#include "TBufferFile.h"
#include "TSystem.h"
#include "TH2.h"
#include "TStyle.h"

void TGRSISelector::Begin(TTree* /*tree*/)
{
   /// The Begin() function is called at the start of the query.
   /// When running with PROOF Begin() is only called on the client.
   /// The tree argument is deprecated (on PROOF 0 is passed).
   TString option = GetOption();
}

void TGRSISelector::SlaveBegin(TTree* /*tree*/)
{
   /// The SlaveBegin() function is called after the Begin() function.
   /// When running with PROOF SlaveBegin() is called on each slave server.
   /// The tree argument is deprecated (on PROOF 0 is passed).
   TString option = GetOption();

   std::cout << "input list size = " << fInput->GetEntries() << std::endl;
   for(int i = 0; i < fInput->GetEntries(); ++i) {
      std::cout << fInput->At(i)->GetName() << ": ";
      fInput->At(i)->Print();
   }

   // read the analysis options that were passed along and copy them to the local TGRSIOptions
   fAnalysisOptions                   = static_cast<TAnalysisOptions*>(fInput->FindObject("TAnalysisOptions"));
   *(TGRSIOptions::AnalysisOptions()) = *fAnalysisOptions;
   // read the TPPG that was passed along
   fPpg = static_cast<TPPG*>(fInput->FindObject("TPPG"));
   if(fPpg == nullptr) {
      std::cerr << "failed to find TPPG!" << std::endl;
   }

   // read the TRunInfo that was passed along
   fRunInfo = static_cast<TRunInfo*>(fInput->FindObject("TRunInfo"));
   if(fRunInfo == nullptr) {
      std::cerr << "failed to find TRunInfo!" << std::endl;
   }

   // if we have a data parser/detector library load it
   if(fInput->FindObject("ParserLibrary") != nullptr) {
      std::string library(fInput->FindObject("ParserLibrary")->GetTitle());
      TGRSIOptions::Get()->ParserLibrary(library);
      if(!library.empty()) {
         // this might throw a runtime exception, but we don't want to catch it here as we need the library for things to work properly!
         TParserLibrary::Get()->Load();
      } else {
         std::cout << "no parser library!" << std::endl;
         TGRSIOptions::Get()->Print();
      }
   }

   const char* workingDirectory = "";
   if(fInput->FindObject("pwd") != nullptr) {
      workingDirectory = fInput->FindObject("pwd")->GetTitle();
   }
   int index = 0;
   while(fInput->FindObject(Form("calFile%d", index)) != nullptr) {
      const char* fileName = static_cast<TNamed*>(fInput->FindObject(Form("calFile%d", index)))->GetTitle();
      if(fileName[0] == 0) {
         std::cout << "Error, empty file name!" << std::endl;
         break;
      }
      // if we have a relative path and a working directory, combine them
      if(workingDirectory[0] != 0 && fileName[0] != '/') {
         TChannel::ReadCalFile(Form("%s/%s", workingDirectory, fileName));
      } else {
         TChannel::ReadCalFile(fileName);
      }
      ++index;
   }
   index = 0;
   while(fInput->FindObject(Form("valFile%d", index)) != nullptr) {
      const char* fileName = static_cast<TNamed*>(fInput->FindObject(Form("valFile%d", index)))->GetTitle();
      if(fileName[0] == 0) {
         std::cout << "Error, empty file name!" << std::endl;
         break;
      }
      // if we have a relative path and a working directory, combine them
      if(workingDirectory[0] != 0 && fileName[0] != '/') {
         GValue::ReadValFile(Form("%s/%s", workingDirectory, fileName));
      } else {
         GValue::ReadValFile(fileName);
      }
      ++index;
   }
   index = 0;
   while(fInput->FindObject(Form("cutFile%d", index)) != nullptr) {
      std::cout << "trying to open " << Form("cutFile%d", index) << std::flush << " = " << fInput->FindObject(Form("cutFile%d", index)) << std::flush << " with title " << static_cast<TNamed*>(fInput->FindObject(Form("cutFile%d", index)))->GetTitle() << std::endl;
      const char* fileName = static_cast<TNamed*>(fInput->FindObject(Form("cutFile%d", index)))->GetTitle();
      if(fileName[0] == 0) {
         std::cout << "Error, empty file name!" << std::endl;
         break;
      }
      // if we have a relative path and a working directory, combine them
      TFile* file = nullptr;
      if(workingDirectory[0] != 0 && fileName[0] != '/') {
         file = new TFile(Form("%s/%s", workingDirectory, fileName));
      } else {
         file = new TFile(fileName);
      }
      if(file != nullptr && file->IsOpen()) {
         TIter iter(file->GetListOfKeys());
         TKey* key = nullptr;
         while((key = static_cast<TKey*>(iter.Next())) != nullptr) {
            if(strcmp(key->GetClassName(), "TCutG") != 0) {
               continue;
            }
            auto* tmpCut = static_cast<TCutG*>(key->ReadObj());
            if(tmpCut != nullptr) {
               fCuts[tmpCut->GetName()] = tmpCut;
            }
         }
      } else {
         std::cout << "Error, failed to open file " << fileName << "!" << std::endl;
         break;
      }
      ++index;
   }
   for(const auto& cut : fCuts) {
      std::cout << cut.first << " = " << cut.second << std::endl;
   }

   if(GValue::Size() == 0) {
      std::cout << "No g-values!" << std::endl;
   } else {
      std::cout << GValue::Size() << " g-values" << std::endl;
   }

   CreateHistograms();
   CheckSizes("use");
}

Bool_t TGRSISelector::Process(Long64_t entry)
{
   /// The Process() function is called for each entry in the tree (or possibly
   /// keyed object in the case of PROOF) to be processed. The entry argument
   /// specifies which entry in the currently loaded tree is to be processed.
   /// It can be passed to either TGRSISelector::GetEntry() or TBranch::GetEntry()
   /// to read either all or the required parts of the data. When processing
   /// keyed objects with PROOF, the object is already loaded and is available
   /// via the fObject pointer.
   ///
   /// This function should contain the "body" of the analysis. It can contain
   /// simple or elaborate selection criteria, run algorithms on the data
   /// of the event and typically fill histograms.
   ///
   /// The processing can be stopped by calling Abort().
   ///
   /// Use fStatus to set the return value of TTree::Process().
   ///
   /// The return value is currently not used

   static TFile* current_file = nullptr;
   if(current_file != fChain->GetCurrentFile()) {
      current_file = fChain->GetCurrentFile();
      std::cout << "Starting to sort: " << current_file->GetName() << std::endl;
      TChannel::ReadCalFromFile(current_file);
      TGRSIOptions::AnalysisOptions()->ReadFromFile(current_file);
   }

   fChain->GetEntry(entry);
   fEntry = entry;
   try {
      FillHistograms();
   } catch(TGRSIMapException<std::string>& e) {
      std::cout << DRED << "Exception in " << __PRETTY_FUNCTION__ << ": " << e.detail() << RESET_COLOR << std::endl;
      throw e;
   }

   return kTRUE;
}

void TGRSISelector::SlaveTerminate()
{
   /// The SlaveTerminate() function is called after all entries or objects
   /// have been processed. When running with PROOF SlaveTerminate() is called
   /// on each slave server.

   // check size of each object in the output list
   EndOfSort();
   CheckSizes("send to server");
   fOutput->Add(new TChannel(TChannel::GetChannelMap()->begin()->second));
   fOutput->Add(new TNamed("prefix", GetOutputPrefix().c_str()));
}

void TGRSISelector::Terminate()
{
   /// The Terminate() function is the last function to be called during
   /// a query. It always runs on the client, it can be used to present
   /// the results graphically or save the results to file.

   CheckSizes("write");

   TGRSIOptions* options = TGRSIOptions::Get();
   if(fRunInfo == nullptr) {
      fRunInfo = TRunInfo::Get();
      std::cout << "replaced null run info with:" << std::endl;
      fRunInfo->Print();
   }
   Int_t runNumber    = TRunInfo::RunNumber();
   Int_t subRunNumber = TRunInfo::SubRunNumber();

   if(fOutput->FindObject("prefix") != nullptr) {
      fOutputPrefix = static_cast<TNamed*>(fOutput->FindObject("prefix"))->GetTitle();
   }

   std::string outputFileName;
   if(runNumber != 0 && subRunNumber != -1) {
      // both run and subrun number set => single file processed
      outputFileName = Form("%s%05d_%03d.root", fOutputPrefix.c_str(), runNumber, subRunNumber);
   } else if(runNumber != 0) {
      // multiple subruns of a single run
      outputFileName = Form("%s%05d_%03d-%03d.root", fOutputPrefix.c_str(), runNumber, TRunInfo::FirstSubRunNumber(), TRunInfo::LastSubRunNumber());
   } else {
      // multiple runs
      outputFileName = Form("%s%05d-%05d.root", fOutputPrefix.c_str(), TRunInfo::FirstRunNumber(), TRunInfo::LastRunNumber());
   }
   auto* outputFile = new TFile(outputFileName.c_str(), "recreate");
   if(!outputFile->IsOpen()) {
      std::cerr << "Failed to open output file " << outputFileName << "!" << std::endl
                << std::endl;
      return;
   }
   outputFileName = outputFileName.substr(0, outputFileName.find_last_of('.')) + ".log";
   options->LogFile(outputFileName);
   std::cout << "Opened '" << outputFile->GetName() << "' for writing:" << std::endl;

   outputFile->cd();
   fOutput->Write();
   fRunInfo->Write();
   if(fPpg != nullptr) {
      fPpg->Write();
   } else {
      std::cerr << "failed to find TPPG, can't write it!" << std::endl;
   }
   TGRSIOptions::AnalysisOptions()->WriteToFile(outputFile);
   TChannel::WriteToRoot();
   outputFile->Close();
   std::cout << "Closed '" << outputFile->GetName() << "'" << std::endl;
}

void TGRSISelector::Init(TTree* tree)
{
   /// The Init() function is called when the selector needs to initialize
   /// a new tree or chain. Typically here the branch addresses and branch
   /// pointers of the tree will be set.
   /// It is normally not necessary to make changes to the generated
   /// code, but the routine can be extended by the user if needed.
   /// Init() will be called many times when running on PROOF
   /// (once per file to be processed).
   /// Set branch addresses and branch pointers
   if(tree == nullptr) {
      return;
   }
   fChain = tree;
   InitializeBranches(tree);
}

Bool_t TGRSISelector::Notify()
{
   /// The Notify() function is called when a new file is opened. This
   /// can be either for a new TTree in a TChain or when when a new TTree
   /// is started when using PROOF. It is normally not necessary to make changes
   /// to the generated code, but the routine can be extended by the
   /// user if needed. The return value is currently not used.

   return kTRUE;
}

void TGRSISelector::CheckSizes(const char* usage)
{
   // check size of each object in the output list
   for(const auto&& obj : *fOutput) {
      TBufferFile buf(TBuffer::kWrite, 10000);
      obj->IsA()->WriteBuffer(buf, obj);
      if(buf.Length() > fSizeLimit) {
         std::cout << DRED << obj->ClassName() << " '" << obj->GetName() << "' too large to " << usage << ": " << buf.Length() << " bytes = " << buf.Length() / 1024. / 1024. / 1024. << " GB, removing it!" << RESET_COLOR << std::endl;
         // we only remove it from the output list, not deleting the object itself
         // this way the selector will still work and fill that histogram, it just won't get written to file
         fOutput->Remove(obj);
      }
   }
}
