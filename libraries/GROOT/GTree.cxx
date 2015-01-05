
#include "GTree.h"

ClassImp(GTree)

GTree::GTree()
      :TTree() {  InitGTree();  }

GTree::GTree(const char* name, const char* title, Int_t splitlevel)
      :TTree(name,title,splitlevel) {  InitGTree();  }
     
GTree::~GTree()  {  }




void GTree::InitGTree() {  }





