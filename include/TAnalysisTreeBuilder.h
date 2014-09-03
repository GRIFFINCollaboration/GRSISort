#ifndef TAnalysisTreeBuilder_H
#define TAnalysisTreeBuilder_H

#include "Globals.h"

#include <cstdio>
#include <vector>
#include <string>
#include <queue>

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TList.h>

#include <TFragment.h>
#include <TChannel.h>
#include <TGRSIRunInfo.h>

class TEventQueue {
   public:
      static TEventQueue *Get();
      static void Add(std::vector<TFragment> *event); 
      static std::vector<TFragment> *Pop();
      static int Size();
      virtual ~TEventQueue();

   private:
      TEventQueue();
      static TEventQueue *fPtrToQue;
      static std::queue<std::vector<TFragment>*> fEventQueue;
      static bool lock;
      static void SetLock() {lock = true;}
      static void UnsetLock() {lock = false;}

};

class TAnalysisTreeBuilder {

   public:
      virtual ~TAnalysisTreeBuilder();

      void ProcessEvent(std::vector<TFragment>*) { };

      static void SetUpFragmentChain(TChain *chain);
      static void SetUpFragmentChain(std::vector<std::string>);
      static void SetupFragmentTree();

      static void SortFragmentChain();
      static void SortFragmentTree() { };

      static void InitChannels();

      static void SetupOutFile() { };
      static void SetupAnalysisTree() { };
      static void FillAnalysisTree() { };
      static void CloseOutFile() { };

      void ProcessEvent() { };

      static void StartMakeAnalysisTree(int argc=1, char **argv=0);

   private:
      TAnalysisTreeBuilder(); 

      static TChain *fFragmentChain;
      static TTree  *fCurrentFragTree;
      static TFile  *fCurrentFragFile;
      static TGRSIRunInfo *fCurrentRunInfo;
};



#endif


