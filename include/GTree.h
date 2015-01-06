#ifndef GTREE__H
#define GTREE__H

#include <TTree.h>

class GTree : public TTree { 

   public:
      GTree();
      GTree(const char* name, const char* title, Int_t splitlevel = 99);
      virtual ~GTree();


   private:

      void InitGTree();

      ClassDef(GTree,1)
  
};


#endif
