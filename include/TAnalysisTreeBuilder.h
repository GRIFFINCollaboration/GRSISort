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

      void SetUpFragmentChain(TChain *chain) { };
      void SetUpFragmentChain(std::vector<std::string>) { };

      void SortFragmentChain() { };
      void SortFragmentTree() { };

      void InitChannels() { };

      void SetupOutFile() { };
      void SetupAnalysisTree() { };
      void FillAnalysisTree() { };
      void CloseOutFile() { };

      void ProcessEvent() { };

   private:
      TAnalysisTreeBuilder(); 

      static TChain *fFragmentChain;

};



#endif


