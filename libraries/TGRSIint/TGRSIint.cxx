#include "TGRSIint.h"
#include "TGRSILoop.h"

#include "GRootGuiFactory.h"
#include "GRSIVersion.h"
#include "Getline.h"
#include "Globals.h"
#include "TAnalysisTreeBuilder.h"
#include "TDataParser.h"
#include "TGRSIOptions.h"
#include "TGRSIOptions2.h"
#include "TGRSIRootIO.h"
#include "TGRSIUtilities.h"
#include "GValue.h"
#include "StoppableThread.h"

#include "TDataLoop.h"
#include "TUnpackingLoop.h"
#include "TFragmentChainLoop.h"
#include "TFragHistLoop.h"
#include "TFragWriteLoop.h"
#include "TTerminalLoop.h"

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
   fAllowedToTerminate(true),fRootFilesOpened(0),fMidasFilesOpened(0) {

      fGRSIEnv = gEnv;
      //TRint::TRint(appClassName, &argc, argv, options, numOptions,noLogo)

      //TSignalHandler sig_handi;
      GetSignalHandler()->Remove();
      TGRSIInterruptHandler *ih = new TGRSIInterruptHandler();
      ih->Add();

      InitFlags();
      TGRSIOptions2::Get(argc,argv);
      PrintLogo(TGRSIOptions2::Get()->ShowLogo());
      SetPrompt("GRSI [%d] ");
      //PrintHelp(TGRSIOptions2::Get()->ShowedHelp());
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
  TGRSIOptions2* opt = TGRSIOptions2::Get();

  bool missing_raw_file = !all_files_exist(opt->InputMidasFiles());

   if(!false) { // this will be change to something like, if(!ClassicRoot)
      LoadGROOTGraphics();
   }

   if(opt->ReadingMaterial()) {
      std::thread fnews = std::thread(ReadTheNews);
      fnews.detach();
   }




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

   //if I am passed any calibrations, lets load those, this
   //will overwrite any with the same address previously read in.
   for(auto cal_filename : opt->CalInputFiles()) {
     TChannel::ReadCalFile(cal_filename.c_str());
   }
   for(auto val_filename : opt->ValInputFiles()) {
     GValue::ReadValFile(val_filename.c_str());
   }
   for(auto info_filename : opt->ExternalRunInfo()) {
     TGRSIRunInfo::Get()->ReadInfoFile(info_filename.c_str());
   }


   SetupFragmentPipeline();

   for(auto& filename : opt->MacroInputFiles()){
     RunMacroFile(filename);
   }

   if(opt->CloseAfterSort()){
     FragmentLoopUntilDone();
     int exit_status = missing_raw_file ? 1 : 0;
     this->Terminate(exit_status);
   }
}

void TGRSIint::FragmentLoopUntilDone() {
  // TODO: Pull in all the stoppablethread stop.
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

int TGRSIint::TabCompletionHook(char* buf, int* pLoc, std::ostream& out) {
  return TRint::TabCompletionHook(buf,pLoc,out);
}


Long_t TGRSIint::ProcessLine(const char* line,Bool_t sync, Int_t *error) {
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
          gFragment = new TChain("FragmentTree");
          //gFragment->SetNotify(GrutNotifier::Get());
        }
        printf("file %s added to gFragment.\n",file->GetName());
        gFragment->Add(file->GetName());
      }


      // If FragmentTree exists, add the file to the chain.
      if(file->FindObjectAny("AnalysisTree")) {
        if(!gAnalysis) {
          gAnalysis = new TChain("AnalysisTree");
          // TODO: Once we have a notifier set up
          //gAnalysis->SetNotify(GrutNotifier::Get());
        }
        printf("file %s added to gAnalysis.\n",file->GetName());
        gAnalysis->Add(file->GetName());
      }

      if(file->FindObjectAny("TChannel")){
        file->Get("TChannel");
      }
      if(file->FindObjectAny("GValue")){
        file->Get("GValue");
      }
      // TODO: Once the run info can read itself from a string.
      // if(file->FindObjectAny("TGRSIRunInfo")){
      //   file->Get("TGRSIRunInfo");
      // }

      fRootFilesOpened++;
    } else {
      std::cout << "Could not open " << filename << std::endl;
    }
  }

  // // Pass the TFile to the python GUI.
  // if(file && GUIIsRunning()){
  //   //TPython::Bind(file,"tdir");
  //   //ProcessLine("TPython::Exec(\"window.AddDirectory(tdir)\");");
  // }
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

void TGRSIint::SetupFragmentPipeline() {
  TGRSIOptions2* opt = TGRSIOptions2::Get();

  // Determining which parts of the pipeline need to be set up.

  bool missing_raw_file = false;
  for(auto& filename : opt->InputMidasFiles()) {
    if(!file_exists(filename.c_str())) {
      missing_raw_file = true;
      std::cerr << "File not found: " << filename << std::endl;
    }
  }

  bool has_raw_file = opt->InputMidasFiles().size();
  bool self_stopping = opt->CloseAfterSort();
  bool sort_raw = ( has_raw_file &&
                    !missing_raw_file &&
                    opt->SortRaw() );

  //bool has_explicit_root_output = opt->OutputFile().length();
//  bool filter_data = opt->CompiledFilterFile().length();
//  bool raw_filtered_output = opt->OutputFilteredFile().length();

  bool has_input_fragment_tree = gFragment;
  bool has_input_analysis_tree = gAnalysis;
  bool write_fragment_histograms = (opt->MakeHistos() &&
                                    opt->FragmentHistogramLib().length()>0);
  // bool write_analysis_histograms = (opt->MakeHistos() &&
  //                                   opt->AnalysisHistogramLib().length()>0);

  bool sort_fragment_tree = (has_input_fragment_tree &&
                             (write_fragment_histograms ||
                              //write_analysis_histograms ||
                              opt->SortRoot() ||
                              opt->MakeAnalysisTree() ||
                              opt->OutputAnalysisFile().length()>0) );

  // bool sort_analysis_tree = (has_input_analysis_tree &&
  //                            !sort_fragment_tree &&
  //                            (write_analysis_histograms || opt->SortRoot() ) );

  bool write_fragment_root_tree = sort_raw;
  // bool write_analysis_root_tree = ((sort_fragment_tree || sort_raw) &&
  //                                  opt->MakeAnalysisTree() );
  //bool write_analysis_root_tree = sort_fragment;

  std::string output_fragment_root_file = "";
  if(opt->OutputFragmentFile().length()) {
    output_fragment_root_file = opt->OutputFragmentFile();
  } else if(fMidasFiles.size() >= 1) {
    output_fragment_root_file = Form("fragment%05i_%03i.root",
                                     fMidasFiles[0]->GetRunNumber(),
                                     fMidasFiles[0]->GetSubRunNumber());
  }

  std::string output_fragment_histogram_file = "";
  if(opt->OutputFragmentHistogramFile().length()) {
    output_fragment_histogram_file = opt->OutputFragmentHistogramFile();
  } else if(output_fragment_root_file.length() > 0) {
    output_fragment_histogram_file = Form("hist_fragment%05i_%03i.root",
                                          GetRunNumber(output_fragment_root_file),
                                          GetSubRunNumber(output_fragment_root_file));
  }

  // std::string output_analysis_root_file = "temp_analysis_tree.root";
  // if(opt->OutputAnalysisFile().length()) {
  //   output_analysis_root_file = opt->OutputAnalysisFile();
  // } else if(output_fragment_root_file.length() > 0) {
  //   output_analysis_root_file = Form("analysis%05i_%03i.root",
  //                                    GetRunNumber(output_fragment_root_file),
  //                                    GetSubRunNumber(output_fragment_root_file));
  // } else if(sort_fragment_tree) {
  //   output_analysis_root_file = Form("analysis%05i_%03i.root",
  //                                    GetRunNumber(gFragment->GetListOfFiles()->At(0)->GetTitle()),
  //                                    GetSubRunNumber(gFragment->GetListOfFiles()->At(0)->GetTitle()));
  // }

  // std::string output_analysis_histogram_file = "";
  // if(opt->OutputAnalysisHistogramFile().length()) {
  //   output_analysis_histogram_file = opt->OutputAnalysisHistogramFile();
  // } else if(output_analysis_root_file.length() > 0) {
  //   output_analysis_histogram_file = Form("hist_analysis%05i_%03i.root",
  //                                         GetRunNumber(output_analysis_root_file),
  //                                         GetSubRunNumber(output_analysis_root_file));
  // }

  std::cout << "Missing Raw File: " << missing_raw_file << std::endl;
  std::cout << "has_raw_file : " << has_raw_file  << std::endl;
  std::cout << "self_stopping: " << self_stopping << std::endl;
  std::cout << "sort_raw: " << sort_raw << std::endl;
  std::cout << "has_input_fragment_tree: " << has_input_fragment_tree << std::endl;
  // std::cout << "has_input_analysis_tree: " << has_input_analysis_tree << std::endl;
  std::cout << "sort_fragment_tree: " << sort_fragment_tree << std::endl;
  // std::cout << "sort_analysis_tree: " << sort_analysis_tree << std::endl;
  std::cout << "write_fragment_root_tree: " << write_fragment_root_tree << std::endl;
  std::cout << "write_fragment_histograms: " << write_fragment_histograms << std::endl;
  // std::cout << "write_analysis_root_tree: " << write_analysis_root_tree << std::endl;
  // std::cout << "write_analysis_histograms: " << write_analysis_histograms << std::endl;
  std::cout << "output_fragment_root_file: " << output_fragment_root_file << std::endl;
  std::cout << "output_fragment_histogram_file: " << output_fragment_histogram_file << std::endl;
  // std::cout << "output_analysis_root_file: " << output_analysis_root_file << std::endl;
  // std::cout << "output_analysis_histogram_file: " << output_analysis_histogram_file << std::endl;

  if(!sort_raw && !sort_fragment_tree /*&& !sort_analysis_tree*/) {
    return;
  }


  // Everything involving the fragment tree
  std::shared_ptr<ThreadsafeQueue<TFragment*> > current_queue = nullptr;
  if(sort_raw) {
    if(fMidasFiles.size() > 1) {
      std::cerr << "I'm going to ignore all but first .mid" << std::endl;
    }

    TDataLoop* data_loop = TDataLoop::Get("1_input_loop",fMidasFiles[0]);
    data_loop->SetSelfStopping(self_stopping);

    TUnpackingLoop* unpack_loop = TUnpackingLoop::Get("2_unpack_loop");
    unpack_loop->InputQueue() = data_loop->OutputQueue();
    current_queue = unpack_loop->GoodOutputQueue();
  } else if (sort_fragment_tree) {
    auto loop = TFragmentChainLoop::Get("1_chain_loop", gFragment);
    loop->SetSelfStopping(self_stopping);
    current_queue = loop->OutputQueue();
  }

  if(write_fragment_histograms) {
    TFragHistLoop* loop = TFragHistLoop::Get("3_frag_hist_loop");
    loop->InputQueue() = current_queue;
    current_queue = loop->OutputQueue();
  }

  if(write_fragment_root_tree) {
    TFragWriteLoop* loop = TFragWriteLoop::Get("3_frag_write_loop");
    loop->InputQueue() = current_queue;
    current_queue = loop->OutputQueue();
  }

  {
    auto loop = TTerminalLoop<TFragment>::Get("4_frag_term_loop");
    loop->InputQueue() = current_queue;
  }

  StoppableThread::ResumeAll();




  // // Now, start setting stuff up

  // std::vector<TFile*> cuts_files;
  // for(auto filename : opt->CutsInputFiles()) {
  //   TFile* tfile = OpenRootFile(filename);
  //   cuts_files.push_back(tfile);
  // }

  // // No need to set up all the loops if we are just opening the interpreter.
  // if(!sort_raw && !sort_tree) {
  //   return;
  // }

  // std::shared_ptr<ThreadsafeQueue<TUnpackedEvent*> > current_queue = nullptr;

  // //next most important thing, if given a raw file && NOT told to not sort!
  // if(sort_raw) {
  //   TRawEventSource* source = OpenRawSource();
  //   fDataLoop = TDataLoop::Get("1_input_loop",source);
  //   fDataLoop->SetSelfStopping(self_stopping);

  //   TBuildingLoop* build_loop = TBuildingLoop::Get("2_build_loop");
  //   build_loop->SetBuildWindow(opt->BuildWindow());
  //   build_loop->InputQueue() = fDataLoop->OutputQueue();

  //   TUnpackingLoop* unpack_loop = TUnpackingLoop::Get("3_unpack");
  //   unpack_loop->InputQueue() = build_loop->OutputQueue();
  //   current_queue = unpack_loop->OutputQueue();

  // } else if(sort_tree) {
  //   fChainLoop = TChainLoop::Get("1_chain_loop",gChain);
  //   fChainLoop->SetSelfStopping(self_stopping);
  //   current_queue = fChainLoop->OutputQueue();
  // }

  // if(filter_data) {
  //   TFilterLoop* filter_loop = TFilterLoop::Get("4_filter_loop");
  //   if(raw_filtered_output) {
  //     filter_loop->OpenRawOutputFile(opt->OutputFilteredFile());
  //   }
  //   filter_loop->InputQueue() = current_queue;
  //   current_queue = filter_loop->OutputQueue();
  // }

  // if(write_root_tree) {
  //   TWriteLoop* write_loop = TWriteLoop::Get("5_write_loop", output_root_file);
  //   write_loop->InputQueue() = current_queue;
  //   current_queue = write_loop->OutputQueue();
  // }

  // if(write_histograms) {
  //   fHistogramLoop = THistogramLoop::Get("6_hist_loop");
  //   fHistogramLoop->SetOutputFilename(output_hist_file);
  //   for(auto cut_file : cuts_files) {
  //     fHistogramLoop->AddCutFile(cut_file);
  //   }
  //   fHistogramLoop->InputQueue() = current_queue;
  //   current_queue = fHistogramLoop->OutputQueue();
  // }

  // TTerminalLoop* terminal_loop = TTerminalLoop::Get("7_terminal_loop");
  // terminal_loop->InputQueue() = current_queue;

  // StoppableThread::ResumeAll();
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
