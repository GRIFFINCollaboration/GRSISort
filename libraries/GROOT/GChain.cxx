
#include "GChain.h"

ClassImp(GChain)

GChain::GChain()
      :TChain() {  InitGChain();  }

GChain::GChain(const char* name, const char* title)
      :TChain(name,title) {  InitGChain();  }
     
GChain::~GChain()  {  }


void GChain::InitGChain() {  }





