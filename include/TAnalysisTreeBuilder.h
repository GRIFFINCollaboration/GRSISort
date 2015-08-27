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
#include "TPPG.h"

#include "TTigress.h" 
#include "TSharc.h"     
#include "TTriFoil.h"   
//#include "TRf.h"        
#include "TCSM.h"       
//#include "TSpice.h"     
//#include "TS3.h"        
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
// This Class builds events out of TDetectorHits. These       //
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
      virtual ~TEventQueue() { std::cout << std::endl << "In event queue dstor." << std::endl; }

   private:
      TEventQueue();
      TEventQueue(const TEventQueue&) { MayNotUse(__PRETTY_FUNCTION__); }
      void operator=(const TEventQueue&){ MayNotUse(__PRETTY_FUNCTION__); }
      static TEventQueue *fPtrToQue;

      
      void Add_Instance(std::vector<TFragment> *event); 
      std::vector<TFragment> *PopEntry_Instance();
      int Size_Instance();
      
      std::queue<std::vector<TFragment>*> fEventQueue;
      #ifndef __CINT__
      std::mutex m_event;
      #endif 
      bool elock;
      void SetLock() {  printf(BLUE "settting event lock" RESET_COLOR  "\n");  elock = true;}
      void UnsetLock() {  printf(RED "unsettting event lock" RESET_COLOR  "\n");  elock = false;}

      ClassDef(TEventQueue,0)

};

//class TWriteQueue : public TObject {
class TWriteQueue {
   public:
      static TWriteQueue *Get();
      static void Add(std::map<std::string, TDetector*> *event); 
      static std::map<std::string, TDetector*> *PopEntry();
      static int Size();
      virtual ~TWriteQueue()  { } //std::cout << std::endl << "In write queue dstor." << std::endl; }

   private:
      TWriteQueue();
      static TWriteQueue *fPtrToQue;

      
      void Add_Instance(std::map<std::string, TDetector*> *event); 
      std::map<std::string, TDetector*> *PopEntry_Instance();
      int Size_Instance();
      
      std::queue<std::map<std::string, TDetector*>*> fWriteQueue;
      #ifndef __CINT__
      std::mutex m_write;
      #endif
      bool wlock;         
      void SetLock()   {  wlock = true; }  // printf(BLUE "settting write lock" RESET_COLOR  "\n");    }
      void UnsetLock() {  wlock = false;}  // printf(RED "unsettting write lock" RESET_COLOR  "\n");   }

	//ClassDef(TWriteQueue,0)

};

class TAnalysisTreeBuilder : public TObject {

   public:
      //virtual ~TAnalysisTreeBuilder();
      virtual ~TAnalysisTreeBuilder() { std::cout << std::endl << "In analysis tree builder dstor." << std::endl; }

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

      void FillWriteQueue(std::map<std::string, TDetector*>*);

      void FillAnalysisTree(std::map<std::string, TDetector*>*);
      void WriteAnalysisTree();
      void CloseAnalysisFile();

      void StartMakeAnalysisTree(int argc=1, char **argv=0);

      void ClearActiveAnalysisTreeBranches();
      void ResetActiveAnalysisTreeBranches();
		  void BuildActiveAnalysisTreeBranches(std::map<std::string, TDetector*>*);

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
      TPPG *fCurrentPPG;

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
      //TRf         *rf;      
      TCSM        *csm;                                     //A pointer to the CSM Mother Class
      //TSpice      *spice;  
      //TS3         *s3;
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


