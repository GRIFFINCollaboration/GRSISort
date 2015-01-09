#ifndef GChain__H
#define GChain__H

#include "TChain.h"

class GChain : public TChain { 

   public:
      GChain();
      GChain(const char* name, const char* title="");
      virtual ~GChain();


   private:

      void InitGChain();

      ClassDef(GChain,1)
  
};


#endif
