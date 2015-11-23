#include "GTree.h"

/// \cond CLASSIMP
ClassImp(GTree)
/// \endcond

GTree::GTree()
      :TTree() {  InitGTree();  }

GTree::GTree(const char* name, const char* title, Int_t splitlevel)
      :TTree(name,title,splitlevel) {  InitGTree();  }
     
GTree::~GTree()  {  }

void GTree::InitGTree() {  }

