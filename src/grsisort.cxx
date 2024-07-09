#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <netdb.h>
#include <iostream>
#include <stdexcept>
#include <pwd.h>

#include "TPluginManager.h"
#include "TGRSIint.h"

#include "GVersion.h"
#include "Globals.h"
#include "TThread.h"

#ifdef __APPLE__
#define HAVE_UTMPX_H
#define UTMP_NO_ADDR
#ifndef ut_user
#define ut_user ut_name
#endif
#ifndef UTMP_FILE
#define UTMP_FILE "/etc/utmp"
#endif
#endif

#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#define STRUCT_UTMP struct utmpx
#else
#if defined(__linux) && defined(__powerpc) && (__GNUC__ == 2) && (__GNUC_MINOR__ < 90)
extern "C" {
#endif
#include <utmp.h>
#define STRUCT_UTMP struct utmp
#endif

static STRUCT_UTMP* gUtmpContents;

void                SetGRSIEnv();
void                SetGRSIPluginHandlers();
static int          ReadUtmp();
static STRUCT_UTMP* SearchEntry(int /*n*/, const char* /*tty*/);
static void         SetDisplay();

TStopwatch* gStopwatch;

void atexitHandler()
{
   // Be polite when you leave.
   double realTime = gStopwatch->RealTime();
   int    hour     = static_cast<int>(realTime / 3600);
   realTime -= hour * 3600;
   int min = static_cast<int>(realTime / 60);
   realTime -= min * 60;
   std::cout << DMAGENTA << std::endl
             << "bye,bye\t" << DCYAN << getpwuid(getuid())->pw_name << RESET_COLOR << " after " << hour << ":"
             << std::setfill('0') << std::setw(2) << min << ":" << std::setprecision(3) << std::fixed << realTime
             << " h:m:s" << std::endl;
}

int main(int argc, char** argv)
{
   gStopwatch = new TStopwatch;
   std::atexit(atexitHandler);

   try {
      TThread::Initialize();
      TObject::SetObjectStat(false);

      SetDisplay();
      grsi::SetGRSIEnv();
      SetGRSIPluginHandlers();
      TGRSIint* input = nullptr;

      // Create an instance of the grsi interpreter so that we can run root-like interpretive mode
      input = TGRSIint::instance(argc, argv);
      input->SetReturnFromRun(true);
      // Run the code!
      input->Run(true);
   } catch(grsi::exit_exception& e) {
      std::cerr << e.message << std::endl;
      // Close files and clean up properly here
   } catch(std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      std::cout << "Don't know how to handle this error, exiting " << argv[0] << "!" << std::endl;
   }

   return 0;
}

void SetGRSIPluginHandlers()
{
   // gPluginMgr->AddHandler("GRootCanvas","grsi","GRootCanvas"
   gPluginMgr->AddHandler("TGuiFactory", "root", "GROOTGuiFactory", "Gui", "GROOTGuiFactory()");
   gPluginMgr->AddHandler("TBrowserImp", "GRootBrowser", "GRootBrowser", "Gui",
                          "NewBrowser(TBrowser *,const char *,Int_t,Int_t,UInt_t,UInt_t");
   gPluginMgr->AddHandler("TBrowserImp", "GRootBrowser", "GRootBrowser", "Gui",
                          "NewBrowser(TBrowser *,const char *,Int_t,Int_t");
}

static int ReadUtmp()
{
   FILE*       utmp = nullptr;
   struct stat file_stats {};
   size_t      n_read = 0;
   size_t      size   = 0;

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

   gUtmpContents = static_cast<STRUCT_UTMP*>(malloc(size));
   if(gUtmpContents == nullptr) {
      fclose(utmp);
      return 0;
   }

   n_read = fread(gUtmpContents, 1, size, utmp);
   if(ferror(utmp) == 0) {
      if(fclose(utmp) != EOF && n_read == size) {
         return static_cast<int>(size / sizeof(STRUCT_UTMP));
      }
   } else {
      fclose(utmp);
   }

   free(gUtmpContents);
   gUtmpContents = nullptr;
   return 0;
}

static STRUCT_UTMP* SearchEntry(int n, const char* tty)
{
   STRUCT_UTMP* ue = gUtmpContents;
   while((n--) != 0) {
      if((ue->ut_name[0] != 0) && (strncmp(tty, ue->ut_line, sizeof(ue->ut_line)) == 0)) {
         return ue;
      }
      ue++;
   }
   return nullptr;
}

static void SetDisplay()
{
   // Set DISPLAY environment variable.

   if(getenv("DISPLAY") == nullptr) {
      char* tty = ttyname(0);   // device user is logged in on
      if(tty != nullptr) {
         tty += 5;   // remove "/dev/"
         STRUCT_UTMP* utmp_entry = SearchEntry(ReadUtmp(), tty);
         if(utmp_entry != nullptr) {
            size_t      length = sizeof(utmp_entry->ut_host);
            std::string display;
            auto*       host = new char[length + 1];
            strncpy(host, utmp_entry->ut_host, length);   // instead of using size of utmp_entry->ut_host to prevent warning from gcc 9.1
            host[sizeof(utmp_entry->ut_host)] = 0;
            if(host[0] != 0) {
               display = host;
               if(strchr(host, ':') != nullptr) {
                  fprintf(stderr, "*** DISPLAY not set, setting it to %s\n", host);
               } else {
                  fprintf(stderr, "*** DISPLAY not set, setting it to %s:0.0\n", host);
                  display += ":0.0";
               }
               setenv("DISPLAY", display.c_str(), 0);
#ifndef UTMP_NO_ADDR
            } else if(utmp_entry->ut_addr != 0) {
               struct hostent* he = gethostbyaddr(reinterpret_cast<const char*>(&utmp_entry->ut_addr), sizeof(utmp_entry->ut_addr), AF_INET);
               if(he != nullptr) {
                  fprintf(stderr, "*** DISPLAY not set, setting it to %s:0.0\n", he->h_name);
                  display = he->h_name;
                  display += ":0.0";
                  setenv("DISPLAY", display.c_str(), 0);
               }
#endif
            }
            delete[] host;
         }
         free(gUtmpContents);
      }
   }
}
