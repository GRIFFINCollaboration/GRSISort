
#include <TROOT.h>
#include <TFolder.h>

#include <GRootObjectManager.h>
//#include <TCanvas.h>


GPadObj::GPadObj(TObject*obj,Int_t pnum,Option_t *opt) {
  fObject    = obj;
  fPadNumber = pnum;
  fOption    = opt;
}

GPadObj::~GPadObj() { fObject=0; }



GRootObjectManager *GRootObjectManager::fGRootObjectManager = 0;

GRootObjectManager *GRootObjectManager::Instance() {
   if(!fGRootObjectManager)
     fGRootObjectManager = new GRootObjectManager;
}

std::map<TCanvas*,std::vector<GPadObj*> > GRootObjectManager::fCanvasMap;

GRootObjectManager::GRootObjectManager() { }

GRootObjectManager::~GRootObjectManager() { }

void GRootObjectManager::Update() { }

void GRootObjectManager::AddCanvas(GCanvas *c) {
  //printf("c = 0x%08x\n",c); fflush(stdout);
  if(!c)
     return;
  //printf("here.\n");
  if(fCanvasMap.count(c)<1) {
    printf("\n  canvas 0x%08x has been added to map.\n",c);
    std::vector<GPadObj*> tmp;
    fCanvasMap.insert(std::make_pair(c,tmp));
  } else {
     printf("\nCanvas already exists...");
  }
}  

void GRootObjectManager::RemoveCanvas(GCanvas *c) {
  
  //printf("c = 0x%08x\n",c); fflush(stdout);
  if(!c)
     return;
  //printf("here.\n");
  if(fCanvasMap.count(c)>0) {
    printf("\n  canvas 0x%08x has been removed from map.\n",c);
    //std::vector<GPadObj*> tmp;
    fCanvasMap.erase(c);
    //fCanvasMap.insert(std::make_pair((TVirtualPad*)c,tmp));
  } else {
     printf("Trying to remove canvas not in map...\n");
  }
  
};

void GRootObjectManager::UpdateLists() {
  //gROOT->GetRootFolder();
  TFolder *mem  = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Memory");
  TFolder *disk = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Files");

  TIter mem_iter(mem->GetListOfFolders());
  //while(TObject *obj = FolderIter(mem_iter)) { 
    //handle the object;   
  //}


}



