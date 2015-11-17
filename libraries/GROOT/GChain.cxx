
#include "GChain.h"

/// \cond CLASSIMP
ClassImp(GChain)
/// \endcond

GChain::GChain()
      :TChain() {  InitGChain();  }

GChain::GChain(const char* name, const char* title)
      :TChain(name,title) {  InitGChain();  }
     
GChain::~GChain()  {  }


void GChain::InitGChain() {  }





