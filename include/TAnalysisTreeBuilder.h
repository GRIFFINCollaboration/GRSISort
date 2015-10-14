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

#include "TObject.h"
#include "TFile.h"
#include "TMemFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TList.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"

#include "TTigress.h" 
#include "TSharc.h"     
#include "TTriFoil.h"   
#include "TRF.h"        
#include "TCSM.h"       
#include "TSiLi.h"     
#include "TS3.h"        
#include "TTip.h"       
   
#include "TGriffin.h"   
#include "TSceptar.h"   
#include "TPaces.h"   
//#include "TDante.h"     
//#include "TZeroDegree.h"
#include "TDescant.h"

////////////////////////////////////////////////////////////////
//                                                            //
// TAnalysisTreeBuilder                                       //
//                                                            //
// This Class builds events out of TGRSIDetectorHits. These   //
// events then get written out to the analysis tree for post  //
// processing. When a new detector class is added to the code //
// it must also be added here for coincidence building        //
//                                                            //
////////////////////////////////////////////////////////////////

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
      void ResetActiveAnalysisTreeBranches();
		  void BuildActiveAnalysisTreeBranches(std::map<const char*, TGRSIDetector*>*);

      void Print(Option_t *opt ="") const;

      void Status();

   private:
      TAnalysisTreeBuilder(); 

      static const size_t MEM_SIZE;                      //Sets the minimum amount of memory used to hold the frament tree

      static TAnalysisTreeBuilder* fAnalysisTreeBuilder; //Pointer to the AnalysisTreeBuilder

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
      std::thread *fReadThread;                             //The thread used to read fragments out of the fragment tree
      std::thread *fProcessThread;                          //The thread used to process and build events
      std::thread *fWriteThread;                            //The thread used to process the write Queue
      std::thread *fStatusThread;                           //The thread used to display the status during sorting
#endif

   private:
     
      TFragment *fCurrentFragPtr;

      //TigAux detectors
      TTigress    *tigress;                                 //A pointer to the Tigress Mother Class
      TSharc      *sharc;                                   //A pointer to the Sharc Mother Class
      TTriFoil    *triFoil;                                 //A pointer to the TriFoil Mother Class
      TRF         *rf;                                      //A pointer to the TRF Mother Class 
      TCSM        *csm;                                     //A pointer to the CSM Mother Class
      TSiLi       *sili;  
      TS3         *s3;
      TTip        *tip;    
       
      //GrifAux detectors
      TGriffin    *griffin;                                 //A pointer to the Griffin Mother Class
      TSceptar    *sceptar;                                 //A pointer to the Sceptar Mother Class
      TPaces      *paces;                                   //A pointer to the Paces Mother Class
      //TDante      *dante;  
      //TZeroDegree *zeroDegree;
      
      //Aux Detectors
      TDescant    *descant;                                 //A pointer to the Descant Mother Class

	ClassDef(TAnalysisTreeBuilder,0) //Builds the Analysis Tree out of TFragments

};

#endif


