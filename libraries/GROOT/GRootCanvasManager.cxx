
#include <GRootCanvasManager.h>
//#include <TCanvas.h>


GPadObj::GPadObj(TObject*obj,Int_t pnum,Option_t *opt) {
  fObject    = obj;
  fPadNumber = pnum;
  fOption    = opt;
}

GPadObj::~GPadObj() { fObject=0; }



GRootCanvasManager *GRootCanvasManager::fGRootCanvasManager = 0;

GRootCanvasManager *GRootCanvasManager::Instance() {
   if(!fGRootCanvasManager)
     fGRootCanvasManager = new GRootCanvasManager;
}

std::map<TCanvas*,std::vector<GPadObj*> > GRootCanvasManager::fCanvasMap;

GRootCanvasManager::GRootCanvasManager() { }

GRootCanvasManager::~GRootCanvasManager() { }

void GRootCanvasManager::Update() { }

void GRootCanvasManager::AddCanvas(TCanvas *c) {
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

void GRootCanvasManager::RemoveCanvas(TCanvas *c) {
  /*
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
  */
};
