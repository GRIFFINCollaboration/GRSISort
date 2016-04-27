#include "TGRSIint.h"
#include "TGRSILoop.h"

#include "TGRSIOptions.h"

#include "TGRSIRootIO.h"
#include "TDataParser.h"
#include "TAnalysisTreeBuilder.h"
#include "Getline.h"
#include "GROOTGuiFactory.h"

#include "Globals.h"
#include "GRSIVersion.h"

#include "TInterpreter.h"
#include "TGHtmlBrowser.h"
//#include <pstream.h>

#include <thread>

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
   if(!fTGRSIint)
      fTGRSIint = new TGRSIint(argc,argv,options,numOptions,true,appClassName);
   return fTGRSIint;
}

TGRSIint::TGRSIint(int argc, char **argv,void *options, Int_t numOptions, Bool_t noLogo,const char *appClassName) 
      :TRint(appClassName, &argc, argv, options, numOptions,noLogo) {

      fGRSIEnv = gEnv;
      //TRint::TRint(appClassName, &argc, argv, options, numOptions,noLogo)

      //TSignalHandler sig_handi;
      GetSignalHandler()->Remove();
      TGRSIInterruptHandler *ih = new TGRSIInterruptHandler();
      ih->Add();

      InitFlags();
      GetOptions(&argc,argv);
      PrintLogo(fPrintLogo);
    //  SetPrompt( DYELLOW "GRSI [%d] " RESET_COLOR);
      SetPrompt("GRSI [%d] ");
      PrintHelp(fPrintHelp);
      std::string grsipath = getenv("GRSISYS");
      gInterpreter->AddIncludePath(Form("%s/include",grsipath.c_str()));
      LoadExtraClasses();
      ApplyOptions();
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
   fAutoSort = false;
   fFragmentSort = false;
   fMakeAnalysisTree = false;

//   if(fGRSIEnv) fGRSIEnv->Delete();
}

void TGRSIint::ApplyOptions() {
   
   if(!false) { // this will be change to something like, if(!ClassicRoot) 
      LoadGROOTGraphics();
   }

   if(TGRSIOptions::ReadingMaterial()) {
      std::thread fnews = std::thread(ReadTheNews);
      fnews.detach();
   }


   if(fAutoSort){
     TGRSILoop::Get()->SortMidas();
   }
   //bool foundCal = false;
   if(TGRSIOptions::MakeAnalysisTree() && TGRSIOptions::GetInputRoot().size()!=0) { 
      TAnalysisTreeBuilder::Get()->StartMakeAnalysisTree();
   }
   for(int x=0;x<(int)(TGRSIOptions::GetInputRoot().size());x++) {
      //printf("TFile *_file%i = new TFile(\"%s\",\"read\")\n",x,TGRSIOptions::GetInputRoot().at(x).c_str());
      long error = ProcessLine(Form("TFile *_file%i = new TFile(\"%s\",\"read\");",x,TGRSIOptions::GetInputRoot().at(x).c_str()));
      if(error <=0) continue;
      TFile *file = (TFile*)gROOT->GetListOfFiles()->FindObject(TGRSIOptions::GetInputRoot().at(x).c_str());
		if(file != NULL) {
			printf("\tfile %s opened as _file%i\n",file->GetName(),x);
			TGRSIRootIO::Get()->LoadRootFile(file);
		} else {
			printf("\tfailed to open file '%s' as _file%i (file = %p)\n", TGRSIOptions::GetInputRoot().at(x).c_str(), x, static_cast<void*>(file));
		}
	}
	// if(TGRSIOptions::GetInputRoot().size() > 0 && !fAutoSort && !fFragmentSort) {
	if(TGRSIOptions::GetInputRoot().size() > 0 && !fAutoSort) {
		if(TGRSIOptions::GetInputRoot().at(0).find("fragment") != std::string::npos){
			Int_t chans_read = ProcessLine("TChannel::ReadCalFromTree(FragmentTree)");
			printf("Read calibration info for %d channels from \"%s\" FragmentTree\n",chans_read,TGRSIOptions::GetInputRoot().at(0).c_str()); 
			TGRSIRunInfo::ReadInfoFromFile();
		}   
		if(TGRSIOptions::GetInputRoot().at(0).find("analysis") != std::string::npos){ 
			Int_t chans_read = ProcessLine("TChannel::ReadCalFromTree(AnalysisTree)");    
			printf("Read calibration info for %d channels from \"%s\" AnalysisTree\n",chans_read,TGRSIOptions::GetInputRoot().at(0).c_str());
			TGRSIRunInfo::ReadInfoFromFile();
		}
		if(TGRSIOptions::GetInputRoot().at(0).find("hists") != std::string::npos){ 
			Int_t chans_read = ProcessLine("TChannel::ReadCalFromCurrentFile()");    
			printf("Read calibration info for %d channels from \"%s\" HistFile\n",chans_read,TGRSIOptions::GetInputRoot().at(0).c_str());
			// TGRSIRunInfo::ReadInfoFromFile(); Not implemented yet
		}
	}

	if(TGRSIOptions::GetInputCal().size() > 0){
		for(size_t i =0; i<TGRSIOptions::GetInputCal().size();++i){
			TChannel::ReadCalFile(TGRSIOptions::GetInputCal().at(i).c_str());
		}
	}

	if(fFragmentSort && TGRSIOptions::GetInputRoot().size()!=0)
		TGRSIRootIO::Get()->MakeUserHistsFromFragmentTree();

	if(TGRSIOptions::WorkHarder()) {
		for(size_t x=0;x<TGRSIOptions::GetMacroFile().size();x++) {
			gROOT->Macro(TGRSIOptions::GetMacroFile().at(x).c_str());  
			// gROOT->ProcessLineSync(Form(".x %s",TGRSIOptions::GetMacroFile().at(x).c_str()));
		}
	}


	if(TGRSIOptions::CloseAfterSort())
		gApplication->Terminate();
}


TGRSIint::~TGRSIint()   {
	// SafeDelete();
}

bool TGRSIint::HandleTermInput() {
	return TRint::HandleTermInput();
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

void TGRSIint::DrawLogo() {
	PopupLogo(false);
	WaitLogo();
}

void TGRSIint::GetOptions(int *argc, char **argv) {

	static char null[1] = { "" };

	fPrintLogo = true;
	fPrintHelp = false;

	if(!argc)
		return;

	std::string pwd ="";

	for (int i = 1; i < *argc; i++) {        //HELP!
		std::string sargv = argv[i];
		if(sargv.length()<2) {
			// one char is not enough to be an option.
			if(sargv[0] == '-') 
				printf(DBLUE "   found option flag '-' not immediately followed by an option." RESET_COLOR "\n");
			else   
				printf(DBLUE "   stand alone option %s not understood, skipping." RESET_COLOR "\n", sargv.c_str());
		}
		if (!strcmp(argv[i],"-?") || !strncmp(argv[i], "--help", 6)) {
			fPrintHelp = true;
		} else if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"-H")) { 
			if(sargv.length()==2) {
				i++; 
				if(i >= *argc) {
					printf(DBLUE "   -h flag given with no host name!" RESET_COLOR "\n");
					break;
				}
				sargv.assign(argv[i]);
				if(sargv[0] == '-' || sargv[0] == '+') {
					i--;
					printf(DRED "     invalid host name: %s; ignoring." RESET_COLOR  "\n",sargv.c_str());
					break;
				} 
			} else {
				sargv = sargv.substr(2);
			}  
			TGRSIOptions::SetHostName(sargv);      
			printf(DYELLOW "host: %s" RESET_COLOR "\n",sargv.c_str());
			break;
		} else if(!strcmp(argv[i],"-e") || !strcmp(argv[i],"-E")) {
			if(sargv.length()==2) {
				i++; 
				if(i >= *argc) {
					printf(DBLUE "   -e flag given with no expt name!" RESET_COLOR "\n");
					break;
				}
				sargv.assign(argv[i]);
				if(sargv[0] == '-' || sargv[0] == '+') {
					i--;
					printf(DRED "     invalid host expt: %s; ignoring." RESET_COLOR  "\n",sargv.c_str());
					break;
				} 
			} else {
				sargv = sargv.substr(2);
			}
			TGRSIOptions::SetExptName(sargv);      
			printf(DYELLOW "experiment: %s" RESET_COLOR "\n",sargv.c_str());
			break;
		} else if (sargv[0] == '-' && sargv[1] != '-') { //single char options.
			sargv = sargv.substr(1);  //drop the minus;
			int defaultcounter = 0;
			for(size_t c=0;c<sargv.length();c++) {
				char key = sargv[c];
				switch(toupper(key)) {
					case 'A':
						printf(DBLUE "Attempting to make analysis trees." RESET_COLOR "\n");
						TGRSIOptions::SetMakeAnalysisTree();
						break;
					case 'Q':
						printf(DBLUE "Closing after Sort." RESET_COLOR "\n");
						TGRSIOptions::SetCloseAfterSort();
						break;
					case 'L':
						fPrintLogo = false;       
						//argv[i] = null;
						break;
					case 'S':
						printf(DBLUE "SORT!!" RESET_COLOR "\n");
						fFragmentSort = true;
						break;
					case 'H':
						printf(DBLUE "Option \"h\" found in list, but must be followed by host name; skipping!\n" RESET_COLOR);
						break;
					case 'E':
						printf(DBLUE "Option \"e\" found in list, but must be followed by experiment name; skipping!\n" RESET_COLOR);
						break;
					default:
						printf(DBLUE "   option %c found but not understood, skipping." RESET_COLOR "\n", sargv[c]);
						defaultcounter++;
						if(defaultcounter>1) {
							printf("Perhaps you are trying to use a word length argument?\n");
							printf("if so, use -- in front of the word instead\n.");
							fPrintHelp = true;
							c = sargv.length() + 1;
							i = *argc + 1;   
						}   
						break;
				}
			}
		} else if (sargv[0] == '-' && sargv[1] == '-') { //word length options.
			std::string temp = sargv.substr(2);
			if(temp.compare("no_waveforms")==0) {
				printf(DBLUE  "    no waveform option set, no waveforms will be in the output tree." RESET_COLOR "\n"); 
				TDataParser::SetNoWaveForms(true);
			} else if(temp.compare("no_record_diag")==0) { 
				printf(DBLUE "     not recording run diagnostics." RESET_COLOR "\n");
				TDataParser::SetRecordDiag(false);
			} else if(temp.compare("write_diag")==0) { 
				printf(DBLUE "     writing run diagnostics to separte .log file." RESET_COLOR "\n");
				TGRSIOptions::SetWriteDiagnostics(false);
			} else if((temp.compare("suppress_error")==0) ||  (temp.compare("suppress_errors")==0)){
				printf(DBLUE "     suppressing loop error statements." RESET_COLOR "\n");
				TGRSILoop::Get()->SetSuppressError(true);
			} else if(temp.compare("log_errors")==0) {
				printf(DBLUE "     sending parsing errors to file." RESET_COLOR "\n");
				TGRSIOptions::SetLogErrors(true);
			} else if(temp.compare("work_harder")==0) {
				printf(DBLUE "     running a macro with .x after making fragment/analysistree." RESET_COLOR "\n");
				TGRSIOptions::SetWorkHarder(true);
			} else if(temp.compare("reading_material")==0) {
				printf(DBLUE"      now providing reading material while you wait." RESET_COLOR "\n");
				TGRSIOptions::SetReadingMaterial(true);
			} else if(temp.compare("no_speed")==0) {
				printf(DBLUE "    not opening the PROOF speedometer." RESET_COLOR "\n");
				TGRSIOptions::SetProgressDialog(false);
			} else if((temp.compare("bad_frags")==0)     || (temp.compare("write_bad_frags")==0) ||
					(temp.compare("bad_fragments")==0) || (temp.compare("write_bad_fragments")==0)) {
				printf(DBLUE "    failed fragements being written to BadFragmentTree." RESET_COLOR "\n");
				TGRSIOptions::SetWriteBadFrags(true);
			} else if(temp.compare("help")==0) {
				fPrintHelp = true;
			} else if(temp.compare("ignore_odb")==0) { 
				// useful when dealing with midas file that have corrupt odbs in them .
				TGRSIOptions::SetIgnoreFileOdb(true);          
			} else if(temp.compare("ignore_epics")==0) { 
				TGRSIOptions::SetIgnoreEpics(true);          
			} else if(temp.compare("ignore_scaler")==0) { 
				TGRSIOptions::SetIgnoreScaler(true);          
			} else {
				printf(DBLUE  "    option: " DYELLOW "%s " DBLUE "passed but not understood." RESET_COLOR "\n",temp.c_str());
			}
		} else if (sargv[0] != '-' && sargv[0] != '+') { //files and directories!
			long size;
			long id, flags, modtime;
			char *dir = gSystem->ExpandPathName(argv[i]);
			if (!gSystem->GetPathInfo(dir, &id, &size, &flags, &modtime)) {   
				if ((flags & 2)) {                                                      
					//I am not sur what to do with directorys right now.                           
					//if (pwd == "") {
					pwd = argv[i]; 
					argv[i]= null;
					//} 
					printf("\tOption %s is a directory, ignoring for now.\n",pwd.c_str());
				} else if (size > 0) {
					// if file add to list of files to be processed
					FileAutoDetect(argv[i],size);
					argv[i] = null;
				} else {
					printf("file %s has size 0, skipping\n", dir);
				}
			} else {
				//file does not exist... complain to the user about this
				if(!FileAutoDetect(argv[i],-1)) {
					printf(DRED "File %s does not exist, ignoring it!" RESET_COLOR "\n",argv[i]);
				}
				argv[i] = null;
			}   
		}
	} 
}



void TGRSIint::LoadGROOTGraphics() {
	if (gROOT->IsBatch()) return;
	// force Canvas to load, this ensures global GUI Factory ptr exists.
	gROOT->LoadClass("TCanvas", "Gpad");
	gGuiFactory =  new GROOTGuiFactory();  
}


void TGRSIint::PrintHelp(bool print) {
	if(print) {
		printf( DRED BG_WHITE "     Sending Help!!     " RESET_COLOR  "\n");
		new TGHtmlBrowser(gSystem->ExpandPathName("${GRSISYS}/README.html"));
	}
	return;
}

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
		if(TGRSIRunInfo::ReadInfoFile(filename.c_str()))
			return true;
		else {
			printf("Problem reading run-info file %s\n",filename.c_str());
			return false;
		}
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


bool TGRSIInterruptHandler::Notify() {
	static int abort_times =0;
	if(abort_times>3) {
		printf("\n" DRED BG_WHITE  "   Control-c was pressed harder.   " RESET_COLOR  SHOW_CURSOR "\n");
		abort();
	}
	printf("\n" DRED BG_WHITE  "   Control-c was pressed.   " RESET_COLOR  SHOW_CURSOR "\n");
	abort_times++;
	gApplication->Terminate();

	return true;
}










