#include "TGRSIint.h"

#include "GRootGuiFactory.h"
#include "GRSIVersion.h"
#include "Getline.h"
#include "Globals.h"
#include "TDataParser.h"
#include "TGRSIOptions.h"
#include "TGRSIUtilities.h"
#include "GValue.h"
#include "TROOT.h"

#include "StoppableThread.h"
#include "TAnalysisHistLoop.h"
#include "TAnalysisWriteLoop.h"
#include "TDataLoop.h"
#include "TDetBuildingLoop.h"
#include "TEventBuildingLoop.h"
#include "TFragHistLoop.h"
#include "TFragWriteLoop.h"
#include "TFragmentChainLoop.h"
#include "TTerminalLoop.h"
#include "TUnpackingLoop.h"
#include "TPPG.h"
#include "TSortingDiagnostics.h"

#include "GRootCommands.h"
#include "TGRSIRunInfo.h"

#include "TInterpreter.h"
#include "TGHtmlBrowser.h"
//#include <pstream.h>

#include "GRootCommands.h"

#include <thread>

#include <pwd.h>

/// \cond CLASSIMP
ClassImp(TGRSIint)
/// \endcond

extern void PopupLogo(bool);
extern void WaitLogo();

TGRSIint *TGRSIint::fTGRSIint = NULL;

TEnv *TGRSIint::fGRSIEnv = NULL;
//std::vector<std::string> *TGRSIint::fInputRootFile  = new std::vector<std::string>;
//std::vector<std::string> *TGRSIint::fInputMidasFile = new std::vector<std::string>;
//std::vector<std::string> *TGRSIint::fInputCalFile   = new std::vector<std::string>;
//std::vector<std::string> *TGRSIint::fInputOdbFile   = new std::vector<std::string>;

void ReadTheNews(void);

TGRSIint *TGRSIint::instance(int argc,char** argv, void *options, int numOptions, bool noLogo, const char *appClassName) {
  if(!fTGRSIint) {
    fTGRSIint = new TGRSIint(argc,argv,options,numOptions,true,appClassName);
    fTGRSIint->ApplyOptions();
  }
  return fTGRSIint;
}

TGRSIint::TGRSIint(int argc, char **argv,void *options, Int_t numOptions, Bool_t noLogo,const char *appClassName)
  :TRint(appClassName, &argc, argv, options, numOptions,noLogo),
   fKeepAliveTimer(NULL), main_thread_id(std::this_thread::get_id()), fIsTabComplete(false),
   fAllowedToTerminate(true),fRootFilesOpened(0),fMidasFilesOpened(0) {

      fGRSIEnv = gEnv;
      //TRint::TRint(appClassName, &argc, argv, options, numOptions,noLogo)

      //TSignalHandler sig_handi;
      GetSignalHandler()->Remove();
      TGRSIInterruptHandler *ih = new TGRSIInterruptHandler();
      ih->Add();

      InitFlags();
      TGRSIOptions::Get(argc,argv);
      PrintLogo(TGRSIOptions::Get()->ShowLogo());
      SetPrompt("GRSI [%d] ");
      //PrintHelp(TGRSIOptions::Get()->ShowedHelp());
      std::string grsipath = getenv("GRSISYS");
      gInterpreter->AddIncludePath(Form("%s/include",grsipath.c_str()));
      //LoadExtraClasses();
      //ApplyOptions();
}

void TGRSIint::LoadExtraClasses() {
  // we should move to make this a loop over the entire libs directory... pcb.
  gROOT->LoadClass("TTigress");
  gROOT->LoadClass("TTigressHit");
  gROOT->LoadClass("TSharc");
  gROOT->LoadClass("TSharcHit");
  gROOT->LoadClass("TGriffin");
  gROOT->LoadClass("TGriffinHit");

  gROOT->LoadClass("TNucleus");
  gROOT->LoadClass("TReaction");
  gROOT->LoadClass("TSRIM");
}

void TGRSIint::InitFlags() {
   // fAutoSort = false;
   // fFragmentSort = false;
   // fMakeAnalysisTree = false;

//   if(fGRSIEnv) fGRSIEnv->Delete();
}

void TGRSIint::ApplyOptions() {
  TGRSIOptions* opt = TGRSIOptions::Get();

  bool missing_raw_file = !all_files_exist(opt->InputMidasFiles());

   if(!false) { // this will be change to something like, if(!ClassicRoot)
      LoadGROOTGraphics();
   }

   if(opt->ReadingMaterial()) {
      std::thread fnews = std::thread(ReadTheNews);
      fnews.detach();
   }


	//if(opt->MakeAnalysisTree()) opt->PrintSortingOptions();

   ////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////
   //////           Start of redoing section        ///////
   ////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////


   for(auto filename : opt->RootInputFiles()) {
     // this will populate gChain if able.
     //   TChannels from the root file will be loaded as file is opened.
     //   GValues from the root file will be loaded as file is opened.
     OpenRootFile(filename);
   }

   for(auto& midas_file : opt->InputMidasFiles()) {
     OpenMidasFile(midas_file.c_str());
   }

   SetupPipeline();

   for(auto& filename : opt->MacroInputFiles()){
     RunMacroFile(filename);
   }

   if(opt->StartGui()) {
     StartGUI();
   }

	LoopUntilDone();
   if(opt->CloseAfterSort()){
     int exit_status = missing_raw_file ? 1 : 0;
     Terminate(exit_status);
   }
}

void TGRSIint::LoopUntilDone() {
  while(StoppableThread::AnyThreadRunning()){
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // We need to process events in case a different thread is asking for a file to be opened.
    // However, if there is no stdin, ProcessEvents() will call Terminate().
    // This prevents the terminate from taking effect while in this context.
    fAllowedToTerminate = false;
    gSystem->ProcessEvents();
    fAllowedToTerminate = true;

    std::cout << "\r" << StoppableThread::AnyThreadStatus() << std::flush;
  }
  std::cout << std::endl;
}


TGRSIint::~TGRSIint()   {
  // SafeDelete();
}

bool TGRSIint::HandleTermInput() {
  return TRint::HandleTermInput();
}

void TGRSIint::Terminate(Int_t status){
  if(!fAllowedToTerminate){
    return;
  }

  StoppableThread::StopAll();

  if(TGRSIOptions::Get()->MakeAnalysisTree()) {
	  TSortingDiagnostics::Get()->Print("error");
  }

  //if(GUIIsRunning()){
  //  TPython::Exec("on_close()");
  //}

  //Be polite when you leave.
  printf(DMAGENTA "\nbye,bye\t" DCYAN "%s" RESET_COLOR  "\n",
         getpwuid(getuid())->pw_name);

  if((clock()%60) == 0){
    printf("DING!");
    fflush(stdout);
    gSystem->Sleep(500);
    printf("\r              \r");
    fflush(stdout);
  }

  //TChannel::DeleteAllChannels();
  TRint::Terminate(status);
}

Int_t TGRSIint::TabCompletionHook(char* buf, int* pLoc, std::ostream& out){
  fIsTabComplete = true;
  auto result = TRint::TabCompletionHook(buf, pLoc, out);
  fIsTabComplete = false;
  return result;
}


Long_t TGRSIint::ProcessLine(const char* line,Bool_t sync, Int_t *error) {
  // If you print while fIsTabComplete is true, you will break tab complete.
  // Any diagnostic print statements should be done after this if statement.
  if(fIsTabComplete){
    long res = TRint::ProcessLine(line, sync, error);
    return res;
  }

  //printf("line = %s\n");
  //if(!strcmp(line,"TCanvas::MakeDefCanvas();"))
  //  line = "GCanvas::MakeDefCanvas();";
  TString sline(line);;
  if(sline.Contains("TCanvas")) {
    std::string s=line;
    size_t f = s.find("TCanvas");
    s.replace(f,std::string("TCanvas").length(),"GCanvas");
    s.replace(f,std::string("TCanvas").length(),"GCanvas");   line = s.c_str();
  }

  if(std::this_thread::get_id() != main_thread_id){
    return DelayedProcessLine(line);
  }

  return TRint::ProcessLine(line,sync,error);
}






void ReadTheNews(void) {
  //gROOT->ProcessLine(".! wget -q -l1 - http://en.wikipedia.org/wiki/Special:Random -Otemp.html");
  //new TGHtmlBrowser("temp.html");
  //std::ipstream wrandom("xdg-open http://en.wikipedia.org/wiki/Special:Random");
#ifdef __APPLE__
  gROOT->ProcessLine(".! open http://en.wikipedia.org/wiki/Special:Random > /dev/null 2>&1");
#else
  gROOT->ProcessLine(".! xdg-open http://en.wikipedia.org/wiki/Special:Random > /dev/null 2>&1");
#endif
  return;
}

void TGRSIint::PrintLogo(bool print) {

  if(print)   {
#ifdef LINUX
    const std::string &ref = ProgramName();
    const unsigned int reflength = ref.length() - 78;
#else
    const std::string &ref = "Sorting Program for Online and Offline Nuclear Data";
    const unsigned int reflength = 53;
#endif

    const unsigned int width = reflength + (reflength % 2);
    printf("\t*%s*\n", std::string(width,'*').c_str());
    printf("\t*%*s%*s*\n",width/2+4,"GRSI Sort", width/2-4, "");
    printf("\t*%*s%*s*\n",width/2+12,"a remake of GRSI SPOON", width/2-12, "");
    printf("\t*%*s%*s*\n",width/2+reflength/2, ref.c_str(), width/2-reflength/2, "");
    printf("\t*%*s%*s*\n",width/2+14,"A lean, mean sorting machine", width/2-14, "");
    printf("\t*%*s%*s*\n",width/2+9,"version " GRSI_RELEASE, width/2-9, "");
    printf("\t*%s*\n", std::string(width,'*').c_str());

    std::thread drawlogo(&TGRSIint::DrawLogo,this);
    drawlogo.detach();
  }
}

TFile* TGRSIint::OpenRootFile(const std::string& filename, Option_t* opt){
  TString sopt(opt);
  sopt.ToLower();

  TFile* file = NULL;
  if(sopt.Contains("recreate") ||
     sopt.Contains("new")) {
    // We are being asked to make a new file.
    file = new TFile(filename.c_str(), "RECREATE");
    if(file){
      // Give access to the file inside the interpreter.
      const char* command = Form("TFile* _file%i = (TFile*)%luL",
                                 fRootFilesOpened,
                                 (unsigned long)file);
      TRint::ProcessLine(command);
      fRootFilesOpened++;
    } else {
      std::cout << "Could not create " << filename << std::endl;
    }
  } else {
    // Open an already existing file.
    file = new TFile(filename.c_str(), opt);
    if(file){
      // Give access to the file inside the interpreter.
      const char* command = Form("TFile* _file%i = (TFile*)%luL",
                                 fRootFilesOpened,
                                 (unsigned long)file);
      TRint::ProcessLine(command);
      std::cout << "\tfile " << BLUE << file->GetName() << RESET_COLOR
                <<  " opened as " << BLUE <<  "_file" << fRootFilesOpened
                << RESET_COLOR <<  std::endl;

      // If FragmentTree exists, add the file to the chain.
      if(file->FindObjectAny("FragmentTree")) {
        if(!gFragment) {
          // TODO: Once we have a notifier set up
          gFragment = new TChain("FragmentChain");
          //gFragment->SetNotify(GrutNotifier::Get());
        }
        printf("file %s added to gFragment.\n",file->GetName());
#if MAJOR_ROOT_VERSION < 6
        gFragment->AddFile(file->GetName(),TChain::kBigNumber, "FragmentTree");
#else
        gFragment->AddFile(file->GetName(),TTree::kMaxEntries, "FragmentTree");
#endif
      }


      // If AnalysisTree exists, add the file to the chain.
      if(file->FindObjectAny("AnalysisTree")) {
        if(!gAnalysis) {
          gAnalysis = new TChain("AnalysisChain");
          // TODO: Once we have a notifier set up
          //gAnalysis->SetNotify(GrutNotifier::Get());
        }
        printf("file %s added to gAnalysis.\n",file->GetName());
#if MAJOR_ROOT_VERSION < 6
        gAnalysis->AddFile(file->GetName(),TChain::kBigNumber, "AnalysisTree");
#else
        gAnalysis->AddFile(file->GetName(),TTree::kMaxEntries, "AnalysisTree");
#endif
      }

      if(file->FindObjectAny("TChannel")){
        file->Get("TChannel");
      }
      if(file->FindObjectAny("GValue")){
        file->Get("GValue");
      }
      // TODO: Once the run info can read itself from a string.
       //if(file->FindObjectAny("TGRSIRunInfo")){
       //  file->Get("TGRSIRunInfo");
      // }

      fRootFilesOpened++;
    } else {
      std::cout << "Could not open " << filename << std::endl;
    }
  }

  AddFileToGUI(file);
  TGRSIRunInfo::ReadInfoFromFile();

  return file;
}

TMidasFile* TGRSIint::OpenMidasFile(const std::string& filename) {
  if(!file_exists(filename.c_str())){
    std::cerr << "File \"" << filename << "\" does not exist" << std::endl;
    return NULL;
  }

  TMidasFile* file = new TMidasFile(filename.c_str());
  fMidasFiles.push_back(file);

  const char* command = Form("TMidasFile* _midas%i = (TMidasFile*)%luL",
                             fMidasFilesOpened,
                             (unsigned long)file);
  ProcessLine(command);

  if(file){
    //std::string name = file->GetName();
    //std::replace(name.begin(),name.end(),'_','/');
    std::cout << "\tfile " << BLUE << filename << RESET_COLOR << " opened as "<< BLUE
              << "_midas" <<  fMidasFilesOpened << RESET_COLOR << std::endl;
  }
  fMidasFilesOpened++;
  return file;
}

void TGRSIint::SetupPipeline() {
  TGRSIOptions* opt = TGRSIOptions::Get();

  // Determining which parts of the pipeline need to be set up.

  bool missing_raw_file = false;
  for(auto& filename : opt->InputMidasFiles()) {
    if(!file_exists(filename.c_str())) {
      missing_raw_file = true;
      std::cerr << "File not found: " << filename << std::endl;
    }
  }

  // Which input files do we have
  bool has_raw_file = opt->InputMidasFiles().size() && opt->SortRaw() && !missing_raw_file;
  bool has_input_fragment_tree = gFragment;// && opt->SortRoot();
  bool has_input_analysis_tree = gAnalysis;// && opt->SortRoot();

  // Which output files are could possibly be made
  bool able_to_write_fragment_histograms = ((has_raw_file || has_input_fragment_tree) &&
                                            opt->FragmentHistogramLib().length() > 0);
  bool able_to_write_fragment_tree = (has_raw_file &&
                                      !missing_raw_file);
  bool able_to_write_analysis_histograms = ((has_raw_file ||
                                             has_input_fragment_tree ||
                                             has_input_analysis_tree) &&
                                            opt->AnalysisHistogramLib().length() > 0);
  bool able_to_write_analysis_tree = (able_to_write_fragment_tree ||
                                      has_input_fragment_tree);

  // Which output files will we make
  bool write_fragment_histograms = (able_to_write_fragment_histograms &&
                                    opt->MakeHistos());
  bool write_fragment_tree = able_to_write_fragment_tree;
  bool write_analysis_histograms = (able_to_write_analysis_histograms &&
                                    (opt->MakeAnalysisTree() || has_input_analysis_tree) &&
                                    opt->MakeHistos());
  bool write_analysis_tree = (able_to_write_analysis_tree &&
                              opt->MakeAnalysisTree());

  // Which steps need to be performed to get from the inputs to the outputs
  bool self_stopping = opt->CloseAfterSort();

  bool read_from_raw = (has_raw_file &&
                        (write_fragment_histograms ||
                         write_fragment_tree ||
                         write_analysis_histograms ||
                         write_analysis_tree));

  bool read_from_fragment_tree = (has_input_fragment_tree &&
                                  (write_fragment_histograms ||
                                   write_analysis_histograms ||
                                   write_analysis_tree));

  bool generate_analysis_data = ((read_from_raw || read_from_fragment_tree) &&
                                 (write_analysis_histograms || write_analysis_tree));

  bool read_from_analysis_tree = (has_input_analysis_tree &&
                                  (write_analysis_histograms || write_analysis_tree) &&
                                  !generate_analysis_data);


  // Extract the run number and sub run number from whatever we were given
  int run_number = 0;
  int sub_run_number = 0;
  if(read_from_raw) {
    run_number = fMidasFiles[0]->GetRunNumber();
    sub_run_number = fMidasFiles[0]->GetSubRunNumber();
  } else if(read_from_fragment_tree) {
    auto run_title = gFragment->GetListOfFiles()->At(0)->GetTitle();
    run_number = GetRunNumber(run_title);
    sub_run_number = GetSubRunNumber(run_title);
  } else if(read_from_analysis_tree) {
    auto run_title = gAnalysis->GetListOfFiles()->At(0)->GetTitle();
    run_number = GetRunNumber(run_title);
    sub_run_number = GetSubRunNumber(run_title);
  }

  // Choose output file names for the 4 possible output files
  std::string output_fragment_tree_filename = opt->OutputFragmentFile();
  if(output_fragment_tree_filename.length() == 0) {
    output_fragment_tree_filename = Form("fragment%05i_%03i.root",
                                         run_number, sub_run_number);
  }

  std::string output_fragment_hist_filename = opt->OutputFragmentHistogramFile();
  if(output_fragment_hist_filename.length() == 0) {
    output_fragment_hist_filename = Form("hist_fragment%05i_%03i.root",
                                         run_number, sub_run_number);
  }

  std::string output_analysis_tree_filename = opt->OutputAnalysisFile();
  if(output_analysis_tree_filename.length() == 0) {
    output_analysis_tree_filename = Form("analysis%05i_%03i.root",
                                         run_number, sub_run_number);
  }

  std::string output_analysis_hist_filename = opt->OutputAnalysisHistogramFile();
  if(output_analysis_hist_filename.length() == 0) {
    output_analysis_hist_filename = Form("hist_analysis%05i_%03i.root",
                                         run_number, sub_run_number);
  }


  // std::cout << "missing_raw_file: " << missing_raw_file << std::endl;
  // std::cout << "has_raw_file: " << has_raw_file << std::endl;
  // std::cout << "has_input_fragment_tree: " << has_input_fragment_tree << std::endl;
  // std::cout << "has_input_analysis_tree: " << has_input_analysis_tree << std::endl;
  // std::cout << "able_to_write_fragment_histograms: " << able_to_write_fragment_histograms << std::endl;
  // std::cout << "able_to_write_fragment_tree: " << able_to_write_fragment_tree << std::endl;
  // std::cout << "able_to_write_analysis_histograms: " << able_to_write_analysis_histograms << std::endl;
  // std::cout << "able_to_write_analysis_tree: " << able_to_write_analysis_tree << std::endl;
  // std::cout << "write_fragment_histograms: " << write_fragment_histograms << std::endl;
  // std::cout << "write_fragment_tree: " << write_fragment_tree << std::endl;
  // std::cout << "write_analysis_histograms: " << write_analysis_histograms << std::endl;
  // std::cout << "write_analysis_tree: " << write_analysis_tree << std::endl;
  // std::cout << "self_stopping: " << self_stopping << std::endl;
  // std::cout << "read_from_raw: " << read_from_raw << std::endl;
  // std::cout << "read_from_fragment_tree: " << read_from_fragment_tree << std::endl;
  // std::cout << "generate_analysis_data: " << generate_analysis_data << std::endl;
  // std::cout << "read_from_analysis_tree: " << read_from_analysis_tree << std::endl;
  // std::cout << "event_build_mode: " << event_build_mode << std::endl;
  // std::cout << "run_number: " << run_number << std::endl;
  // std::cout << "sub_run_number: " << sub_run_number << std::endl;
  // std::cout << "output_fragment_tree_filename: " << output_fragment_tree_filename << std::endl;
  // std::cout << "output_fragment_hist_filename: " << output_fragment_hist_filename << std::endl;
  // std::cout << "output_analysis_tree_filename: " << output_analysis_tree_filename << std::endl;
  // std::cout << "output_analysis_hist_filename: " << output_analysis_hist_filename << std::endl;

  if(read_from_analysis_tree) {
    std::cerr << "Reading from analysis tree not currently supported" << std::endl;
  }

  ////////////////////////////////////////////////////
  ////////////  Setting up the loops  ////////////////
  ////////////////////////////////////////////////////

  if(!write_fragment_histograms &&
     !write_fragment_tree &&
     !write_analysis_histograms &&
     !write_analysis_tree) {
    return;
  }

  // Different queues that can show up
  std::shared_ptr<ThreadsafeQueue<TFragment*> > fragment_queue = nullptr;
  std::shared_ptr<ThreadsafeQueue<TFragment*> > bad_queue = nullptr;
  std::shared_ptr<ThreadsafeQueue<TEpicsFrag*> > scaler_queue = nullptr;
  std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> > analysis_queue = nullptr;

  // If needed, read from the raw file
  if(read_from_raw) {
    if(fMidasFiles.size() > 1) {
      std::cerr << "I'm going to ignore all but first .mid" << std::endl;
    }

    TDataLoop* data_loop = TDataLoop::Get("1_input_loop",fMidasFiles[0]);
    data_loop->SetSelfStopping(self_stopping);

    TUnpackingLoop* unpack_loop = TUnpackingLoop::Get("2_unpack_loop");
    unpack_loop->InputQueue() = data_loop->OutputQueue();
    fragment_queue = unpack_loop->GoodOutputQueue();
    scaler_queue = unpack_loop->ScalerOutputQueue();
    bad_queue = unpack_loop->BadOutputQueue();
  }
   //if I am passed any calibrations, lets load those, this
   //will overwrite any with the same address previously read in.
   for(auto cal_filename : opt->CalInputFiles()) {
     TChannel::ReadCalFile(cal_filename.c_str());
   }
   if(fMidasFiles.size()) {
     TGRSIRunInfo::Get()->SetRunInfo(fMidasFiles[0]->GetRunNumber(),
                                     fMidasFiles[0]->GetSubRunNumber());
   } else {
     TGRSIRunInfo::Get()->SetRunInfo(0,0);
   }
   TPPG::Get()->Setup();
   for(auto val_filename : opt->ValInputFiles()) {
     GValue::ReadValFile(val_filename.c_str());
   }
   for(auto info_filename : opt->ExternalRunInfo()) {
     TGRSIRunInfo::Get()->ReadInfoFile(info_filename.c_str());
   }


  //this happens here, because the TDataLoop constructor is where we read the midas file ODB
  TEventBuildingLoop::EBuildMode event_build_mode = TEventBuildingLoop::kTriggerId;
  if(TGRSIRunInfo::Get()->Griffin()) {
    event_build_mode = TEventBuildingLoop::kTimestamp;
  }

  // If needed, read from the fragment tree
  if (read_from_fragment_tree) {
    auto loop = TFragmentChainLoop::Get("1_chain_loop", gFragment);
    loop->SetSelfStopping(self_stopping);
    fragment_queue = loop->OutputQueue();
  }

  // If requested, write the fragment histograms
  if(write_fragment_histograms) {
    TFragHistLoop* loop = TFragHistLoop::Get("3_frag_hist_loop");
    loop->SetOutputFilename(output_fragment_hist_filename);
    loop->InputQueue() = fragment_queue;
    fragment_queue = loop->OutputQueue();
  }

  // If requested, write the fragment tree
  if(write_fragment_tree) {
    TFragWriteLoop* loop = TFragWriteLoop::Get("4_frag_write_loop",
                                               output_fragment_tree_filename);
    fNewFragmentFile = output_fragment_tree_filename;
    loop->InputQueue() = fragment_queue;
    loop->BadInputQueue() = bad_queue;
    fragment_queue = loop->OutputQueue();
    if(scaler_queue) {
      loop->ScalerInputQueue() = scaler_queue;
      scaler_queue = nullptr;
    }
  }

  // If needed, generate the individual detectors from the TFragments
  if(generate_analysis_data) {
	 TGRSIOptions::Get()->PrintSortingOptions();
    TEventBuildingLoop* eoop = TEventBuildingLoop::Get("5_event_build_loop",
                                                       event_build_mode);
    eoop->SetSortDepth(opt->SortDepth());
    eoop->SetBuildWindow(opt->BuildWindow());
    eoop->InputQueue() = fragment_queue;
    fragment_queue = nullptr;

    TDetBuildingLoop* doop = TDetBuildingLoop::Get("6_det_build_loop");
    doop->InputQueue() = eoop->OutputQueue();
    analysis_queue = doop->OutputQueue();
  }

  // If requested, write the analysis histograms
  if(write_analysis_histograms) {
    TAnalysisHistLoop* loop = TAnalysisHistLoop::Get("7_analysis_hist_loop");
    loop->SetOutputFilename(output_analysis_hist_filename);
    loop->InputQueue() = analysis_queue;
    analysis_queue = loop->OutputQueue();
  }

  // If requested, write the analysis tree
  if(write_analysis_tree) {
    TAnalysisWriteLoop* loop = TAnalysisWriteLoop::Get("8_analysis_write_loop",
                                                       output_analysis_tree_filename);
    loop->InputQueue() = analysis_queue;
    analysis_queue = loop->OutputQueue();
  }


  // For each leftover queue, terminate if still exists.
  if(fragment_queue) {
    auto loop = TTerminalLoop<TFragment>::Get("9_frag_term_loop");
    loop->InputQueue() = fragment_queue;
    fragment_queue = nullptr;
  }

  if(scaler_queue) {
    auto loop = TTerminalLoop<TEpicsFrag>::Get("A_scaler_term_loop");
    loop->InputQueue() = scaler_queue;
    scaler_queue = nullptr;
  }

  if(bad_queue) {
    auto loop = TTerminalLoop<TFragment>::Get("B_bad_frag_term_loop");
    loop->InputQueue() = bad_queue;
    bad_queue = nullptr;
  }

  if(analysis_queue) {
    auto loop = TTerminalLoop<TUnpackedEvent>::Get("C_analysis_term_loop");
    loop->InputQueue() = analysis_queue;
    analysis_queue = nullptr;
  }

  StoppableThread::ResumeAll();
}

void TGRSIint::RunMacroFile(const std::string& filename){
  if(file_exists(filename.c_str())){
    const char* command = Form(".x %s", filename.c_str());
    ProcessLine(command);
  } else {
    std::cerr << "File \"" << filename << "\" does not exist" << std::endl;
  }
}

void TGRSIint::DrawLogo() {
  PopupLogo(false);
  WaitLogo();
}

// void TGRSIint::GetOptions(int *argc, char **argv) {

//   static char null[1] = { "" };

//   fPrintLogo = true;
//   fPrintHelp = false;

//   if(!argc)
//     return;

//   std::string pwd ="";

//   for (int i = 1; i < *argc; i++) {        //HELP!
//     std::string sargv = argv[i];
//     if(sargv.length()<2) {
//       // one char is not enough to be an option.
//       if(sargv[0] == '-')
//         printf(DBLUE "   found option flag '-' not immediately followed by an option." RESET_COLOR "\n");
//       else
//         printf(DBLUE "   stand alone option %s not understood, skipping." RESET_COLOR "\n", sargv.c_str());
//     }
//     if (!strcmp(argv[i],"-?") || !strncmp(argv[i], "--help", 6)) {
//       fPrintHelp = true;
//     } else if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) {
//       if(sargv.length()==2) {
//         i++;
//         if(i >= *argc) {
//           printf(DBLUE "   -h flag given with no host name!" RESET_COLOR "\n");
//           break;
//         }
//         sargv.assign(argv[i]);
//         if(sargv[0] == '-' || sargv[0] == '+') {
//           i--;
//           printf(DRED "     invalid host name: %s; ignoring." RESET_COLOR  "\n",sargv.c_str());
//           break;
//         }
//       } else {
//         sargv = sargv.substr(2);
//       }
//       TGRSIOptions::SetHostName(sargv);
//       printf(DYELLOW "host: %s" RESET_COLOR "\n",sargv.c_str());
//       break;
//     } else if(!strcmp(argv[i],"-e") || !strcmp(argv[i],"-E")) {
//       if(sargv.length()==2) {
//         i++;
//         if(i >= *argc) {
//           printf(DBLUE "   -e flag given with no expt name!" RESET_COLOR "\n");
//           break;
//         }
//         sargv.assign(argv[i]);
//         if(sargv[0] == '-' || sargv[0] == '+') {
//           i--;
//           printf(DRED "     invalid host expt: %s; ignoring." RESET_COLOR  "\n",sargv.c_str());
//           break;
//         }
//       } else {
//         sargv = sargv.substr(2);
//       }
//       TGRSIOptions::SetExptName(sargv);
//       printf(DYELLOW "experiment: %s" RESET_COLOR "\n",sargv.c_str());
//       break;
//     } else if (sargv[0] == '-' && sargv[1] != '-') { //single char options.
//       sargv = sargv.substr(1);  //drop the minus;
//       int defaultcounter = 0;
//       for(size_t c=0;c<sargv.length();c++) {
//         char key = sargv[c];
//         switch(toupper(key)) {
//           case 'A':
//             printf(DBLUE "Attempting to make analysis trees." RESET_COLOR "\n");
//             TGRSIOptions::SetMakeAnalysisTree();
//             break;
//           case 'Q':
//             printf(DBLUE "Closing after Sort." RESET_COLOR "\n");
//             TGRSIOptions::SetCloseAfterSort();
//             break;
//           case 'L':
//             fPrintLogo = false;
//             //argv[i] = null;
//             break;
//           case 'S':
//             printf(DBLUE "SORT!!" RESET_COLOR "\n");
//             fFragmentSort = true;
//             break;
//           case 'H':
//             printf(DBLUE "Option \"h\" found in list, but must be followed by host name; skipping!\n" RESET_COLOR);
//             break;
//           case 'E':
//             printf(DBLUE "Option \"e\" found in list, but must be followed by experiment name; skipping!\n" RESET_COLOR);
//             break;
//           default:
//             printf(DBLUE "   option %c found but not understood, skipping." RESET_COLOR "\n", sargv[c]);
//             defaultcounter++;
//             if(defaultcounter>1) {
//               printf("Perhaps you are trying to use a word length argument?\n");
//               printf("if so, use -- in front of the word instead\n.");
//               fPrintHelp = true;
//               c = sargv.length() + 1;
//               i = *argc + 1;
//             }
//             break;
//         }
//       }
//     } else if (sargv[0] == '-' && sargv[1] == '-') { //word length options.
//       std::string temp = sargv.substr(2);
//       if(temp.compare("no_waveforms")==0) {
//         printf(DBLUE  "    no waveform option set, no waveforms will be in the output tree." RESET_COLOR "\n");
//         TDataParser::SetNoWaveForms(true);
//       } else if(temp.compare("no_record_diag")==0) {
//         printf(DBLUE "     not recording run diagnostics." RESET_COLOR "\n");
//         TDataParser::SetRecordDiag(false);
//       } else if(temp.compare("write_diag")==0) {
//         printf(DBLUE "     writing run diagnostics to separte .log file." RESET_COLOR "\n");
//         TGRSIOptions::SetWriteDiagnostics(false);
//       } else if((temp.compare("suppress_error")==0) ||  (temp.compare("suppress_errors")==0)){
//         printf(DBLUE "     suppressing loop error statements." RESET_COLOR "\n");
//         TGRSILoop::Get()->SetSuppressError(true);
//       } else if(temp.compare("log_errors")==0) {
//         printf(DBLUE "     sending parsing errors to file." RESET_COLOR "\n");
//         TGRSIOptions::SetLogErrors(true);
//       } else if(temp.compare("work_harder")==0) {
//         printf(DBLUE "     running a macro with .x after making fragment/analysistree." RESET_COLOR "\n");
//         TGRSIOptions::SetWorkHarder(true);
//       } else if(temp.compare("reading_material")==0) {
//         printf(DBLUE"      now providing reading material while you wait." RESET_COLOR "\n");
//         TGRSIOptions::SetReadingMaterial(true);
//       } else if(temp.compare("no_speed")==0) {
//         printf(DBLUE "    not opening the PROOF speedometer." RESET_COLOR "\n");
//         TGRSIOptions::SetProgressDialog(false);
//       } else if((temp.compare("bad_frags")==0)     || (temp.compare("write_bad_frags")==0) ||
//           (temp.compare("bad_fragments")==0) || (temp.compare("write_bad_fragments")==0)) {
//         printf(DBLUE "    failed fragements being written to BadFragmentTree." RESET_COLOR "\n");
//         TGRSIOptions::SetWriteBadFrags(true);
//       } else if(temp.compare("help")==0) {
//         fPrintHelp = true;
//       } else if(temp.compare("ignore_odb")==0) {
//         // useful when dealing with midas file that have corrupt odbs in them .
//         TGRSIOptions::SetIgnoreFileOdb(true);
//       } else if(temp.compare("ignore_epics")==0) {
//         TGRSIOptions::SetIgnoreEpics(true);
//       } else if(temp.compare("ignore_scaler")==0) {
//         TGRSIOptions::SetIgnoreScaler(true);
//       } else {
//         printf(DBLUE  "    option: " DYELLOW "%s " DBLUE "passed but not understood." RESET_COLOR "\n",temp.c_str());
//       }
//     } else if (sargv[0] != '-' && sargv[0] != '+') { //files and directories!
//       long size;
//       long id, flags, modtime;
//       char *dir = gSystem->ExpandPathName(argv[i]);
//       if (!gSystem->GetPathInfo(dir, &id, &size, &flags, &modtime)) {
//         if ((flags & 2)) {
//           //I am not sur what to do with directorys right now.
//           //if (pwd == "") {
//           pwd = argv[i];
//           argv[i]= null;
//           //}
//           printf("\tOption %s is a directory, ignoring for now.\n",pwd.c_str());
//         } else if (size > 0) {
//           // if file add to list of files to be processed
//           FileAutoDetect(argv[i],size);
//           argv[i] = null;
//         } else {
//           printf("file %s has size 0, skipping\n", dir);
//         }
//       } else {
//         //file does not exist... complain to the user about this
//         if(!FileAutoDetect(argv[i],-1)) {
//           printf(DRED "File %s does not exist, ignoring it!" RESET_COLOR "\n",argv[i]);
//         }
//         argv[i] = null;
//       }
//     }
//   }
// }



void TGRSIint::LoadGROOTGraphics() {
  if (gROOT->IsBatch()) return;
  // force Canvas to load, this ensures global GUI Factory ptr exists.
  gROOT->LoadClass("TCanvas", "Gpad");
  gGuiFactory =  new GRootGuiFactory();
}


void TGRSIint::PrintHelp(bool print) {
  if(print) {
    printf( DRED BG_WHITE "     Sending Help!!     " RESET_COLOR  "\n");
    new TGHtmlBrowser(gSystem->ExpandPathName("${GRSISYS}/README.html"));
  }
  return;
}

/*
bool TGRSIint::FileAutoDetect(std::string filename, long filesize) {
  //first search for extensions.
  std::string ext = filename.substr(filename.find_last_of('.')+1);
  //printf("\text = %s\n",ext.c_str());
  if(ext.compare("root")==0 && filesize > 0) {
    //printf("\tFound root file: %s\n",filename.c_str());
    //fInputRootFile->push_back(filename);
    TGRSIOptions::AddInputRootFile(filename);
    return true;
  } else if((ext.compare("mid")==0 || ext.compare("bz2")==0) && filesize > 0) {
    //printf("\tFound midas file: %s\n",filename.c_str());
    //fInputMidasFile->push_back(filename);
    TGRSIOptions::AddInputMidasFile(filename);
    fAutoSort = true;
    return true;
  } else if(ext.compare("cal")==0 && filesize > 0) {
    //printf("\tFound custom calibration file: %s\n",filename.c_str());
    //fInputCalFile->push_back(filename);
    TGRSIOptions::AddInputCalFile(filename);
    return true;
  } else if(ext.compare("info")==0 && filesize > 0) {
      TGRSIOptions::AddExternalRunInfo(filename);
      return true;
  } else if(ext.compare("xml")==0 && filesize > 0) {
    //fInputOdbFile->push_back(filename);
    TGRSIOptions::AddInputOdbFile(filename);
    //printf("\tFound xml odb file: %s\n",filename.c_str());
    return true;
  } else if(ext.compare("odb")==0 && filesize > 0) {
    //printf("\tFound c-like odb file: %s\n",filename.c_str());
    printf("c-like odb structures can't be read yet.\n");
    return false;
  } else if((ext.compare("c")==0) || (ext.compare("C")==0) || (ext.compare("c+")==0) || (ext.compare("C+")==0)) {
    //scripts are the only files that don't have to exist, they may also be found in the macro-paths
    TGRSIOptions::AddMacroFile(filename);
    return true;
  } else {
    printf("\tDiscarding unknown file: %s\n",filename.c_str());
    return false;
  }
  return false;
}
*/

bool TGRSIInterruptHandler::Notify() {
  static int timespressed  = 0;
  timespressed++;
  if(timespressed>3) {
    printf("\n" DRED BG_WHITE  "   No you shutup! " RESET_COLOR "\n"); fflush(stdout);
    exit(1);
  }
  printf("\n" DRED BG_WHITE  "   Control-c was pressed.   " RESET_COLOR "\n"); fflush(stdout);
  TGRSIint::instance()->Terminate();
  return true;
}


//   These variables are to be accessed only from DelayedProcessLine
// and DelayedProcessLine_Action, nowhere else.
//   These are used to pass information between the two functions.
// DelayedProcessLine_Action() should ONLY be called from a TTimer running
// on the main thread.  DelayedProcessLine may ONLY be called
// from inside ProcessLine().
namespace {
  std::mutex g__CommandListMutex;
  std::mutex g__ResultListMutex;
  std::mutex g__CommandWaitingMutex;
  std::condition_variable g__NewResult;

  std::string g__LineToProcess;
  bool g__ProcessingNeeded;

  Long_t g__CommandResult;
  bool g__CommandFinished;
}

Long_t TGRSIint::DelayedProcessLine(std::string command){
  std::lock_guard<std::mutex> any_command_lock(g__CommandWaitingMutex);

  g__LineToProcess = command;
  g__CommandFinished = false;
  g__ProcessingNeeded = true;
  TTimer::SingleShot(0,"TGRSIint",this,"DelayedProcessLine_Action()");

  std::unique_lock<std::mutex> lock(g__ResultListMutex);
  while(!g__CommandFinished){
    g__NewResult.wait(lock);
  }

  return g__CommandResult;
}


void TGRSIint::DelayedProcessLine_Action(){
  std::string message;
  {
    std::lock_guard<std::mutex> lock(g__CommandListMutex);
    if(!g__ProcessingNeeded){
      return;
    }
    message = g__LineToProcess;
  }

  Long_t result = this->ProcessLine(message.c_str());
  Getlinem(EGetLineMode::kInit,((TRint*)gApplication)->GetPrompt());

  {
    std::lock_guard<std::mutex> lock(g__ResultListMutex);
    g__CommandResult = result;
    g__CommandFinished = true;
    g__ProcessingNeeded = false;
  }

  g__NewResult.notify_one();
}
