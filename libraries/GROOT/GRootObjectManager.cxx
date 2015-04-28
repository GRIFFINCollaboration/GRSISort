
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


ClassImp(GRootObjectManager)


GRootObjectManager *GRootObjectManager::fGRootObjectManager = 0;

GRootObjectManager *GRootObjectManager::Instance() {
   if(!fGRootObjectManager)
     fGRootObjectManager = new GRootObjectManager;
}

std::map<TCanvas*,std::vector<GPadObj*> > GRootObjectManager::fCanvasMap;

GRootObjectManager::GRootObjectManager() { }

GRootObjectManager::~GRootObjectManager() { }


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

/*
void GRootObjectManager::Update() {
  //gROOT->GetRootFolder();
  TFolder *mem  = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Memory");
  //TFolder *disk = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Files");
  ExtractObjects(mem->GetListOfFolders());

}

void GRootObjectManager::ExtractObjects(TCollection *list) {
  TIter iter(list);
  TObject *object = 0;
  while(Object = iter.Next()) {
    if(object->InheritsFrom("TFolder")) {
       ExtractObjects(((TFolder*)object)->GetListOfFolders());
    } else if(object->InheritsFrom("TDirectory")) {
       ExtractObjects(((TDirectory*)object)->GetList());
    } else if(object->InheritsFrom("TH3")) {
       if(!fThreeDHists.FindObject(object))
         fThreeDHists.Add(object);
    } else if(object->InheritsFrom("TH2")) {
       if(!fTwoDHists.FindObject(object))
         fTwoDHists.Add(object);
    } else if(object->InheritsFrom("TH1")) {
       if(!fOneDHists.FindObject(object))
         fOneDHists.Add(object);
    } else if(object->InheritsFrom("TGraph")) {
       if(!fGraphs.FindObject(object))
         fGraphs.Add(object);
    } else {
       if(!fMisc.FindObject(object))
         fMisc->Add(object);
    }
  }
}

void GRootObjectManager::Cleanup() { 
  TIter iter1(&fOneDHists);
  while(TObject *object = iter1.Next()) {
     if(!gROOT->FindObjectAny(object))
        fOneDHist->Remove(object);
  } 
  TIter iter2(&fTwoDHists);
  while(TObject *object = iter2.Next()) {
     if(!gROOT->FindObjectAny(object))
        fTwoDHist->Remove(object);
  } 
  TIter iter3(&fThreeDHists);
  while(TObject *object = iter3.Next()) {
     if(!gROOT->FindObjectAny(object))
        fThreeDHist->Remove(object);
  } 
  TIter iter4(&fGraphs);
  while(TObject *object = iter4.Next()) {
     if(!gROOT->FindObjectAny(object))
        fGraphs->Remove(object);
  } 
  TIter iter5(&fMisc);
  while(TObject *object = iter5.Next()) {
     if(!gROOT->FindObjectAny(object))
        fMisc->Remove(object);
  } 
}



void GRootObjectManager::Print() {
   int Size1D = fOneDHists.GetSize();
   int Size2D = fTwoDHists.GetSize();
   int Size3D = fThreeDHists.GetSize();
   int SizeGr = fGraphs.GetSize();
   int SizeMc = fMisc.GetSize();
   printf("TH1's managed:    \t%i\n",Size1D);
   printf("TH2's managed:    \t%i\n",Size2D);
   printf("TH3's managed:    \t%i\n",Size3D);
   printf("TGraphs's managed:\t%i\n",SizeGr);
   printf("Misc's managed:   \t%i\n",SizeMc);
   printf("Sum:              \t%i\n",Size1D+Size2D+Size3D+SizeGr+SizeMc);
   printf("GCanvas tracking: \t%i\n",fCanvasMap.size());
}

*/




