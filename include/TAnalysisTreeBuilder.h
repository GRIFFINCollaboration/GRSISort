#ifndef TAnalysisTreeBuilder_H
#define TAnalysisTreeBuilder_H

/** \addtogroup Sorting
 *  @{
 */

#include <cstdio>
#include <vector>
#include <string>
#include <queue>
#if !defined (__CINT__) && !defined (__CLING__)
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

#include "Globals.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"
#include "TPPG.h"

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
#include "TLaBr.h"
#include "TTAC.h"
#include "TZeroDegree.h"
#include "TDescant.h"

/////////////////////////////////////////////////////////////////
///
/// \class TEventQueue
///
/// This Class stores pointers to vectors of TFragments in 
/// a std::queue. The queue is filled in
/// TAnalysisTreeBuilder::SortFragmentTree,
/// TAnalysisTreeBuilder::SortFragmentChain, or
/// TAnalysisTreeBuilder::SortFragmentTreeByTimeStamp.
/// The events are taking out of the queue and built into events
/// in TAnalysisTreeBuilder::ProcessEvent.
///
/////////////////////////////////////////////////////////////////

class TEventQueue : public TObject {
  public:
	static TEventQueue* Get();
	static void Add(std::vector<TFragment>* event); 
	static std::vector<TFragment>* PopEntry();
	static int Size();
	virtual ~TEventQueue() { std::cout << std::endl << "In event queue dstor." << std::endl; }

  private:
	TEventQueue();
   TEventQueue(const TEventQueue&) : TObject() { MayNotUse(__PRETTY_FUNCTION__); }
	void operator=(const TEventQueue&) { MayNotUse(__PRETTY_FUNCTION__); }
	static TEventQueue* fPtrToQue;

      
	void AddInstance(std::vector<TFragment>* event); 
	std::vector<TFragment>* PopEntryInstance();
	int SizeInstance();
      
	std::queue<std::vector<TFragment>*> fEventQueue;
#if !defined (__CINT__) && !defined (__CLING__)
	std::mutex m_event;
#endif 
	bool fELock;
	void SetLock()   { printf(BLUE "settting event lock" RESET_COLOR  "\n");  fELock = true;  }
	void UnsetLock() { printf(RED "unsettting event lock" RESET_COLOR  "\n"); fELock = false; }

/// \cond CLASSIMP
	ClassDef(TEventQueue,0)
/// \endcond
};

/////////////////////////////////////////////////////////////////
///
/// \class TWriteQueue
///
/// This Class stores a map of detector pointers in a std::queue
/// The queue is filled in TAnalysisTreeBuilder::ProcessEvent via 
/// TAnalysisTreeBuilder::FillWriteQueue.
/// The events are taking out of the queue and written to the
/// AnalysisTree in TAnalysisTreeBuilder::WriteAnalysisTree.
///
/////////////////////////////////////////////////////////////////

class TWriteQueue {
   public:
      static TWriteQueue* Get();
      static void Add(std::map<std::string, TDetector*>* event); 
      static std::map<std::string, TDetector*>* PopEntry();
      static int Size();
      virtual ~TWriteQueue() { }

   private:
      TWriteQueue();
      static TWriteQueue* fPtrToQue;

      
      void AddInstance(std::map<std::string, TDetector*>* event); 
      std::map<std::string, TDetector*>* PopEntryInstance();
      int SizeInstance();
      
      std::queue<std::map<std::string, TDetector*>*> fWriteQueue;
#if !defined (__CINT__) && !defined (__CLING__)
      std::mutex m_write;
#endif
      bool fWLock;         
      void SetLock()   {  fWLock = true;  }
      void UnsetLock() {  fWLock = false; }
};

/////////////////////////////////////////////////////////////////
///
/// \class TAnalysisTreeBuilder
///
/// This Class builds events out of TFragments. These
/// events then get written out to the analysis tree for post
/// processing. When a new detector class is added to the code
/// it must also be added here for coincidence building.
///
/////////////////////////////////////////////////////////////////

class TAnalysisTreeBuilder : public TObject {
   public:
      virtual ~TAnalysisTreeBuilder() { std::cout << std::endl << "In analysis tree builder dstor." << std::endl; }

      static TAnalysisTreeBuilder* Get();
      void ProcessEvent();

      void SetUpFragmentChain(TChain* chain);
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

      void StartMakeAnalysisTree(int argc=1, char** argv=0);

      void ClearActiveAnalysisTreeBranches();
      void ResetActiveAnalysisTreeBranches();
		void BuildActiveAnalysisTreeBranches(std::map<std::string, TDetector*>*);

      void Print(Option_t *opt ="") const;

      void Status();

   private:
      TAnalysisTreeBuilder(); 

      static const size_t MEM_SIZE;                         ///< Sets the minimum amount of memory used to hold the frament tree

      static TAnalysisTreeBuilder* fAnalysisTreeBuilder;    ///< Pointer to the AnalysisTreeBuilder

      TChain* fFragmentChain;
      TTree*  fCurrentFragTree;
      TFile*  fCurrentFragFile;
      TTree*  fCurrentAnalysisTree;
      TFile*  fCurrentAnalysisFile;
      TGRSIRunInfo* fCurrentRunInfo;
      TPPG* fCurrentPPG;

      static long fEntries;
      static int fFragmentsIn;
      static int fAnalysisIn;
      static int fAnalysisOut;
      static double fLastStatusTime;                        ///< the run time the last time a status was created
      static int fLastAnalysisOut;                          ///< number of built events the last time a status was created

#if !defined (__CINT__) && !defined (__CLING__)
      bool fSortFragmentDone;
      bool fPrintStatus;
      std::thread* fReadThread;                             ///< The thread used to read fragments out of the fragment tree
      std::thread* fProcessThread;                          ///< The thread used to process and build events
      std::thread* fWriteThread;                            ///< The thread used to process the write Queue
      std::thread* fStatusThread;                           ///< The thread used to display the status during sorting
#endif

   private:
     
      TFragment* fCurrentFragPtr;

      //TigAux detectors
      TTigress*    fTigress;                                 ///< A pointer to the Tigress Mother Class
      TSharc*      fSharc;                                   ///< A pointer to the Sharc Mother Class
      TTriFoil*    fTriFoil;                                 ///< A pointer to the TriFoil Mother Class
      TRF*         fRf;                                      ///< A pointer to the TRF Mother Class 
      TCSM*        fCsm;                                     ///< A pointer to the CSM Mother Class
      TSiLi*       fSiLi;  
      TS3*         fS3;
      TTip*        fTip;    
       
      //GrifAux detectors
      TGriffin*    fGriffin;                                 ///< A pointer to the Griffin Mother Class
      TSceptar*    fSceptar;                                 ///< A pointer to the Sceptar Mother Class
      TPaces*      fPaces;                                   ///< A pointer to the Paces Mother Class
      TLaBr*       fLaBr; 
      TTAC*        fTAC;
      TZeroDegree* fZeroDegree;                              ///< A pointer to the ZeroDegree mother class
      
      //Aux Detectors
      TDescant*    fDescant;                                 ///< A pointer to the Descant Mother Class

/// \cond CLASSIMP
	ClassDef(TAnalysisTreeBuilder,0) //Builds the Analysis Tree out of TFragments
/// \endcond
};
/*! @} */
#endif
