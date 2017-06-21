#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <netdb.h>
#include <iostream>
#include <stdexcept>
#include <pwd.h>

#include "TEnv.h"
#include "TPluginManager.h"
#include "TGRSIint.h"

#include "GVersion.h"
#include "TThread.h"

#ifdef __APPLE__
#define HAVE_UTMPX_H
#define UTMP_NO_ADDR
#ifndef ut_user
#   define ut_user ut_name
#endif
#ifndef UTMP_FILE
#define UTMP_FILE "/etc/utmp"
#endif
#endif


# ifdef HAVE_UTMPX_H
# include <utmpx.h>
# define STRUCT_UTMP struct utmpx
# else
# if defined(__linux) && defined(__powerpc) && (__GNUC__ == 2) && (__GNUC_MINOR__ < 90)
   extern "C" {
# endif
# include <utmp.h>
# define STRUCT_UTMP struct utmp
# endif


static STRUCT_UTMP* gUtmpContents;

void SetGRSIEnv();
void SetGRSIPluginHandlers();
static int ReadUtmp();
static STRUCT_UTMP *SearchEntry(int /*n*/, const char* /*tty*/);
static void SetDisplay();

int main(int argc, char **argv) {
   auto stopwatch = new TStopwatch;
   try {
     TThread::Initialize();
     TObject::SetObjectStat(false);
     
      //Find the grsisort environment variable so that we can read in .grsirc
      SetDisplay();
      SetGRSIEnv();
      SetGRSIPluginHandlers();
      TGRSIint* input = nullptr;
   
      //Create an instance of the grsi interpreter so that we can run root-like interpretive mode
      input = TGRSIint::instance(argc,argv);
      //Run the code!
      input->Run(true);
   } catch(grsi::exit_exception& e) {
      std::cerr << e.message << std::endl;
      //Close files and clean up properly here
   }

   // Be polite when you leave.
   double realTime = stopwatch->RealTime();
   int    hour     = static_cast<int>(realTime / 3600);
   realTime -= hour * 3600;
   int min = static_cast<int>(realTime / 60);
   realTime -= min * 60;
	std::cout<<DMAGENTA<<std::endl<<"bye,bye\t"<<DCYAN<<getpwuid(getuid())->pw_name<<RESET_COLOR<<" after "<<hour<<":"<<std::setfill('0')<<std::setw(2)<<min<<":"<<std::setprecision(3)<<std::fixed<<realTime<<" h:m:s"<<std::endl;

	return 0;
}

void SetGRSIEnv() {
   std::string grsi_path = getenv("GRSISYS"); //Finds the GRSISYS path to be used by other parts of the grsisort code
   if(grsi_path.length()>0) {
      grsi_path += "/";
   }
   //Read in grsirc in the GRSISYS directory to set user defined options on grsisort startup
   grsi_path +=  ".grsirc"; 
   gEnv->ReadFile(grsi_path.c_str(),kEnvChange);
}

void SetGRSIPluginHandlers() {
   //gPluginMgr->AddHandler("GRootCanvas","grsi","GRootCanvas"
   gPluginMgr->AddHandler("TGuiFactory","root","GROOTGuiFactory","Gui","GROOTGuiFactory()");
   gPluginMgr->AddHandler("TBrowserImp","GRootBrowser","GRootBrowser",
                          "Gui","NewBrowser(TBrowser *,const char *,Int_t,Int_t,UInt_t,UInt_t");
   gPluginMgr->AddHandler("TBrowserImp","GRootBrowser","GRootBrowser",
                          "Gui","NewBrowser(TBrowser *,const char *,Int_t,Int_t");
}

static int ReadUtmp() {
	FILE  *utmp;
	struct stat file_stats{};
	size_t n_read, size;

	gUtmpContents = nullptr;

	utmp = fopen(UTMP_FILE, "r");
	if(utmp == nullptr) { 
		return 0;
	}

	fstat(fileno(utmp), &file_stats);
	size = file_stats.st_size;
	if(size <= 0) {
		fclose(utmp);
		return 0;
	}

	gUtmpContents = static_cast<STRUCT_UTMP *>( malloc(size));
	if(gUtmpContents == nullptr) {
		fclose(utmp);
		return 0;
	}

	n_read = fread(gUtmpContents, 1, size, utmp);
	if(ferror(utmp) == 0) {
		if(fclose(utmp) != EOF && n_read == size) {
			return size / sizeof(STRUCT_UTMP);
		}
	} else {
		fclose(utmp);
	}

	free(gUtmpContents);
	gUtmpContents = nullptr;
	return 0;
}

static STRUCT_UTMP* SearchEntry(int n, const char *tty)  {
	STRUCT_UTMP* ue = gUtmpContents;
	while((n--) != 0) {
		if((ue->ut_name[0] != 0) && (strncmp(tty, ue->ut_line, sizeof(ue->ut_line)) == 0)) { 
			return ue;
		}
		ue++;
	}
	return nullptr;
}


static void SetDisplay()  {
	// Set DISPLAY environment variable.

	if(getenv("DISPLAY") == nullptr) {
		char *tty = ttyname(0);  // device user is logged in on
		if(tty != nullptr) {
			tty += 5;             // remove "/dev/"
			STRUCT_UTMP* utmp_entry = SearchEntry(ReadUtmp(), tty);
			if(utmp_entry != nullptr) {
				auto* display = new char[sizeof(utmp_entry->ut_host) + 15];
				auto* host =    new char[sizeof(utmp_entry->ut_host) + 1];
				strncpy(host, utmp_entry->ut_host, sizeof(utmp_entry->ut_host));
				host[sizeof(utmp_entry->ut_host)] = 0;
				if(host[0] != 0) {
					if(strchr(host, ':') != nullptr) {
						sprintf(display, "DISPLAY=%s", host);
						fprintf(stderr, "*** DISPLAY not set, setting it to %s\n",
								host);
					} else {
						sprintf(display, "DISPLAY=%s:0.0", host);
						fprintf(stderr, "*** DISPLAY not set, setting it to %s:0.0\n",
								host);
					}
					putenv(display);
#ifndef UTMP_NO_ADDR
				} else if(utmp_entry->ut_addr) {
					struct hostent *he;
					if((he = gethostbyaddr((const char*)&utmp_entry->ut_addr,
									sizeof(utmp_entry->ut_addr), AF_INET))) {
						sprintf(display, "DISPLAY=%s:0.0", he->h_name);
						fprintf(stderr, "*** DISPLAY not set, setting it to %s:0.0\n",
								he->h_name);
						putenv(display);
					}
#endif
				}
				delete [] host;
				// display cannot be deleted otherwise the env var is deleted too
			}
			free(gUtmpContents);
		}
	}
}

