#ifndef TAnalysisTreeBuilder_H
#define TAnalysisTreeBuilder_H

#include "Globals.h"

#include <cstdio>
#include <vector>
#include <queue>

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





#endif


