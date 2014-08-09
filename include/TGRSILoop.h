#ifndef TGRSILOOP_H
#define TGRSILOOP_H

#include <cstdio>
#include <string>
#include <fstream>

#ifndef __CINT__
#include <thread>
#endif

#include <TObject.h>

#include "TGRSIint.h"
#include "TMidasFile.h"
#include "TMidasEvent.h"
#include "TXMLOdb.h"
#include "TChannel.h"

#include "Globals.h"

class TGRSILoop : public TObject {
   
   public:
      static TGRSILoop *Get();
      virtual ~TGRSILoop();

   private:
      static TGRSILoop *fTGRSILoop;
      TGRSILoop();

   public:
      void Initialize();
      void BeginRun(int transition,int runnumber, int time);
      void EndRun(int transition,int runnumber, int time);
      void Finalize();

   private:
      bool fTestMode;
      bool fOffline;   

      TXMLOdb *fOdb;

      int fFragsSentToTree;

   #ifndef __CINT__
      std::thread *fMidasThread;
      std::thread *fFillTreeThread;
   #endif

   public:
      bool fMidasThreadRunning;
      bool fFillTreeThreadRunning;
      
   
      bool IsOnline()   { return !fOffline;  }
      bool IsOffline()  { return fOffline;   }

      bool SortMidas();
      void ProcessMidasFile(TMidasFile*);
      void FillFragmentTree(TMidasFile*);
      bool ProcessMidasEvent(TMidasEvent*);
      bool ProcessTIGRESS(int *ptr,int &dsizei,unsigned int mserial=0,unsigned int mtime=0);
      bool ProcessGRIFFIN(int *ptr,int &dsize,unsigned int mserial=0,unsigned int mtime=0);
      bool ProcessEPICS();

      void SetFileOdb(char *data,int size);
      void SetTIGOdb();
      void SetGRIFFOdb();

      void Clear(Option_t *opt="");
      void Print(Option_t *opt="");

   ClassDef(TGRSILoop,0)
};

#endif
