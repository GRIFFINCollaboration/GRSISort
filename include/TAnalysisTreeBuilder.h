#ifndef TAnalysisTreeBuilder_H
#define TAnalysisTreeBuilder_H

#include "Globals.h"

#include <cstdio>
#include <vector>
#include <string>
#include <queue>
#include <stdint.h>
#ifndef __CINT__
#define _GLIBCXX_USE_NANOSLEEP 1
   #include <thread>
   #include <mutex>
   #include <chrono>
#endif

#include <TObject.h>
#include <TFile.h>
#include <TMemFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TList.h>

#include <TFragment.h>
#include <TChannel.h>
#include <TGRSIRunInfo.h>

#include <TTigress.h> 
#include <TSharc.h>     
#include <TTriFoil.h>   
//#include <TRf.h>        
#include <TCSM.h>       
//#include <TSpice.h>     
//#include <TS3.h>        
//#include <TTip.h>       
   
#include <TGriffin.h>   
#include <TSceptar.h>   
#include <TPaces.h>     
//#include <TDante.h>     
//#include <TZeroDegree.h>
//#include <TDescant.h>   


class TEventQueue : public TObject {
   public:
      static TEventQueue *Get();
      static void Add(std::vector<TFragment> *event); 
      static std::vector<TFragment> *PopEntry();
      static int Size();
      virtual ~TEventQueue();

   private:
      TEventQueue();
      static TEventQueue *fPtrToQue;
      static std::queue<std::vector<TFragment>*> fEventQueue;
      #ifndef __CINT__
      static std::mutex m_event;
      #endif 
      static bool elock;
      static void SetLock() {  printf(BLUE "settting event lock" RESET_COLOR  "\n");  elock = true;}
      static void UnsetLock() {  printf(RED "unsettting event lock" RESET_COLOR  "\n");  elock = false;}

   //ClassDef(TEventQueue,0)

};

class TWriteQueue : public TObject {
   public:
      static TWriteQueue *Get();
      static void Add(std::map<const char*, TGRSIDetector*> *event); 
      static std::map<const char*, TGRSIDetector*> *PopEntry();
      static int Size();
      virtual ~TWriteQueue();

   private:
      TWriteQueue();
      static TWriteQueue *fPtrToQue;
      static std::queue<std::map<const char*, TGRSIDetector*>*> fWriteQueue;
      #ifndef __CINT__
      static std::mutex m_write;
      #endif
      static bool wlock;         
      static void SetLock()   {  wlock = true; }  // printf(BLUE "settting write lock" RESET_COLOR  "\n");    }
      static void UnsetLock() {  wlock = false;}  // printf(RED "unsettting write lock" RESET_COLOR  "\n");   }

	//ClassDef(TWriteQueue,0)

};

class TAnalysisTreeBuilder : public TObject {

   public:
      virtual ~TAnalysisTreeBuilder();


      static TAnalysisTreeBuilder* Get();
      void ProcessEvent();

      void SetUpFragmentChain(TChain *chain);
      void SetUpFragmentChain(std::vector<std::string>);
      void SetupFragmentTree();

      void SortFragmentChain();
      void SortFragmentTree();
      void SortFragmentTreeByTimeStamp();

      void InitChannels();

      void SetupOutFile();
      void SetupAnalysisTree();

      void FillWriteQueue(std::map<const char*, TGRSIDetector*>*);

      void FillAnalysisTree(std::map<const char*, TGRSIDetector*>*);
      void WriteAnalysisTree();
      void CloseAnalysisFile();

      void StartMakeAnalysisTree(int argc=1, char **argv=0);

      void ClearActiveAnalysisTreeBranches();
		void BuildActiveAnalysisTreeBranches(std::map<const char*, TGRSIDetector*>*);

      void Print(Option_t *opt ="");

      void Status();

   private:
      TAnalysisTreeBuilder(); 

      static const size_t MEM_SIZE;

      static TAnalysisTreeBuilder* fAnalysisTreeBuilder;

      TChain *fFragmentChain;
      TTree  *fCurrentFragTree;
      TFile  *fCurrentFragFile;
      TTree  *fCurrentAnalysisTree;
      TFile  *fCurrentAnalysisFile;
      TGRSIRunInfo *fCurrentRunInfo;

      static long fEntries;
      static int fFragmentsIn;
      static int fAnalysisIn;
      static int fAnalysisOut;

#ifndef __CINT__
      bool fSortFragmentDone;
      bool fPrintStatus;
      std::thread *fReadThread;
      std::thread *fProcessThread;
      std::thread *fWriteThread;
      std::thread *fStatusThread;
#endif

   private:
     
      TFragment *fCurrentFragPtr;

      TTigress    *tigress;  
      TSharc      *sharc;   
      TTriFoil    *triFoil;
      //TRf         *rf;     
      TCSM        *csm;    
      //TSpice      *spice;  
      //TS3         *s3;
      //TTip        *tip;    
       
      TGriffin    *griffin;
      TSceptar    *sceptar;
      TPaces      *paces;  
      //TDante      *dante;  
      //TZeroDegree *zeroDegree;
      //TDescant    *descant;

	ClassDef(TAnalysisTreeBuilder,0)

};



#endif


