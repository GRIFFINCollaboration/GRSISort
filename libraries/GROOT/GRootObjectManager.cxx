
#include <TROOT.h>
#include <TFolder.h>
#include <TClass.h>

#include <GRootObjectManager.h>
//#include <TCanvas.h>


GPadObj::GPadObj(TObject*obj,Int_t pnum, Option_t *opt) {
  fObject    = obj;
  fParent    = 0;
  fPadNumber = pnum;
  fOption    = opt;
}

GPadObj::~GPadObj() { fObject=0; }


GMemObj::GMemObj(TObject *obj,TObject *par,TFile *file,Option_t *opt) {
  fThis      = obj;
  fParent    = par;
  fFile      = file;
  fOption    = opt;
}

GMemObj::~GMemObj() { } //fObject=0; }




ClassImp(GRootObjectManager)


GRootObjectManager *GRootObjectManager::fGRootObjectManager = 0;

GRootObjectManager *GRootObjectManager::Instance() {
   if(!fGRootObjectManager)
     fGRootObjectManager = new GRootObjectManager();
   return fGRootObjectManager;
}

std::map<TCanvas*,std::vector<GPadObj> > GRootObjectManager::fCanvasMap;

std::map<TObject*,GMemObj > GRootObjectManager::fObjectsMap;

GRootObjectManager::GRootObjectManager() { 
   printf("GRootObjectManager created.\n"); 
   fCanvasList = (TList*)gROOT->GetListOfCanvases();
   //fObjectList = (TList*)gROOT->GetList
}

GRootObjectManager::~GRootObjectManager() {
   
}


void GRootObjectManager::AddCanvas(GCanvas *c) { AddCanvas((TCanvas*)c); }

void GRootObjectManager::AddCanvas(TCanvas *c) {
  //printf("c = 0x%08x\n",c); fflush(stdout);
  if(!c)
     return;
  //printf("here.\n");
  if(fCanvasMap.count(c)<1) {
    //printf("\n  canvas 0x%08x has been added to map.\n",c);
    std::vector<GPadObj> tmp;
    fCanvasMap.insert(std::make_pair(c,tmp));
  } else {
    //printf("\nCanvas already exists...");
  }
}  


void GRootObjectManager::RemoveCanvas(GCanvas *c) { RemoveCanvas((TCanvas*)c); }

void GRootObjectManager::RemoveCanvas(TCanvas *c) {
  
  //printf("c = 0x%08x\n",c); fflush(stdout);
  if(!c)
     return;
  //printf("here.\n");
  if(fCanvasMap.count(c)>0) {
    //printf("\n  canvas 0x%08x has been removed from map.\n",c);
    //std::vector<GPadObj*> tmp = fCanvasMap.at(c);
    fCanvasMap.erase(c);
    //fCanvasMap.insert(std::make_pair((TVirtualPad*)c,tmp));
  } else {
     //printf("Trying to remove canvas not in map...\n");
  }
  
};


void GRootObjectManager::AddObject(TObject *object,TObject *par,TFile *file,Option_t *opt) { 
  if(!object)
     return;
  if(object->InheritsFrom("TH1")   || // all histograms.
     object->InheritsFrom("TGraph") || // all graphs (include TCutG's)
     object->InheritsFrom("TCut")) {    // string cuts (esm:  "x<10")
     if(!fObjectsMap.count(object)) {
        fObjectsMap.insert(std::pair<TObject*,GMemObj>(object,GMemObj(object,par,file,opt))) ;
     }
  }
}

void GRootObjectManager::RemoveObject(TObject *object) {
  if(!object)
     return;
  if(fObjectsMap.count(object)>0) {
     fObjectsMap.erase(object);
  }
}

bool GRootObjectManager::SetParent(TObject *object,TObject *parent) {
  if(fObjectsMap.count(object)<1)
     return false;
  fObjectsMap.at(object).SetParent(parent);
  return true;
}


bool GRootObjectManager::SetFile(TObject *object,TFile *file) {
  if(fObjectsMap.count(object)<1)
     return false;
  fObjectsMap.at(object).SetFile(file);
  return true;
}

TH1 *GRootObjectManager::GetNext1D(TObject *object) {
  //Returns the next TH1 currently managed from
  //from the key object. If no object is passed,
  //returns the first TH1 from the beginning of 
  //the map.
  std::map<TObject*,GMemObj>::iterator it;
  std::map<TObject*,GMemObj>::iterator start;
  if(!object) {
    it=fObjectsMap.begin();
    for(;it!=fObjectsMap.end();it++) {
      //printf("it->first->GetName() = %s\n",it->first->GetName());
      if(it->first->InheritsFrom("TH1") &&
         !it->first->InheritsFrom("TH2") &&
         !it->first->InheritsFrom("TH3")   ) {
         return (TH1*)it->first;
      }   
    }
  } else {
    it=fObjectsMap.find(object);
    start = it;
    it++;
    for(;it!=start;it++) { 
      if(it==fObjectsMap.end())
        it=fObjectsMap.begin();
      //printf("it->first->GetName() = %s\n",it->first->GetName());
      if(  it->first->InheritsFrom("TH1") &&
        !it->first->InheritsFrom("TH2") &&
        !it->first->InheritsFrom("TH3")   ) {
        return (TH1*)it->first;
      }
    }
  }
  return 0;
}

TH2    *GRootObjectManager::GetNext2D(TObject *object) { }
//TH3    *GRootObjectManager::GetNext3D(TObject *object) { }
TGraph *GRootObjectManager::GetNextGraph(TObject *object) { }



void GRootObjectManager::Update() {
  if(fCanvasList->GetSize() != fCanvasMap.size()) {
    if(fCanvasList->GetSize()>fCanvasMap.size()) {
      TIter iter(fCanvasList);
      while(TCanvas *canvas = (TCanvas*)iter.Next()) {
        if(!fCanvasMap.count(canvas))
           AddCanvas(canvas);
      }
    } else {
     std::map<TCanvas*,std::vector<GPadObj> >::iterator iter;
     for(iter=fCanvasMap.begin();iter!=fCanvasMap.end();iter++)
       if(!fCanvasList->FindObject(iter->first))
          RemoveCanvas(iter->first);
    }
  }
  
  //Scan memory for "useful" objects.
  TFolder *folder = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Memory");
  ExtractObjects(folder->GetListOfFolders());

  //Scan loaded files...
  folder = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Files");
  ExtractObjects(folder->GetListOfFolders());

  //finally, lets look for cuts...
  TList *list = (TList*)gROOT->GetListOfSpecials()->FindObject("ROOT Files");
  ExtractObjects((TCollection*)list);

  return;
}



void GRootObjectManager::ExtractObjects(TCollection *list) {
  TIter iter(list);
  TObject *object = 0;
  while(object = iter.Next()) {
    if(object->InheritsFrom("TFolder")) {
       ExtractObjects(((TFolder*)object)->GetListOfFolders());
    } else if(object->InheritsFrom("TDirectory")) {
       ExtractObjects(((TDirectory*)object)->GetList());
    } else {
       AddObject(object);
    }
  }
  return;
}

/*

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
*/


void GRootObjectManager::Print() {
   Update();
   printf("Canvases currently tracked:  %i\n",fCanvasMap.size());
   printf("fCanvasMapList Content:\n");
   //fCanvasList->Print();
   std::map<TCanvas*,std::vector<GPadObj> >::iterator iter;
   int counter = 0;
   for(iter=fCanvasMap.begin();iter!=fCanvasMap.end();iter++)
       printf("\t% 2i.\t%s, contains %i objects.\n",counter++,iter->first->GetName(),iter->second.size());

   printf("Objects currently tracked:  %i\n",fObjectsMap.size());
   printf("fObjectsMap Content:\n");
   //fCanvasList->Print();
   std::map<TObject*,GMemObj >::iterator iter2;
   counter = 0;
   for(iter2=fObjectsMap.begin();iter2!=fObjectsMap.end();iter2++)
       printf("\t% 2i.\t%s [%s], parent[0x%08x] | file[0c%08x].\n",counter++,iter2->first->GetName(),iter2->first->IsA()->GetName(),
                                                                             iter2->second.GetParent(),iter2->second.GetFile());


}


   //int Size1D = fOneDHists.GetSize();
   //int Size2D = fTwoDHists.GetSize();
   //int Size3D = fThreeDHists.GetSize();
   //int SizeGr = fGraphs.GetSize();
   //int SizeMc = fMisc.GetSize();
   //printf("TH1's managed:    \t%i\n",Size1D);
   //printf("TH2's managed:    \t%i\n",Size2D);
   //printf("TH3's managed:    \t%i\n",Size3D);
   //printf("TGraphs's managed:\t%i\n",SizeGr);
   //printf("Misc's managed:   \t%i\n",SizeMc);
   //printf("Sum:              \t%i\n",Size1D+Size2D+Size3D+SizeGr+SizeMc);
   //printf("GCanvas tracking: \t%i\n",fCanvasMap.size());
//}






