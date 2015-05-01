
#include <TROOT.h>
#include <TFolder.h>
#include <TClass.h>
#include <TKey.h>
#include <TString.h>

#include <GRootObjectManager.h>
//#include <TCanvas.h>

ClassImp(GPadObj)

GPadObj::GPadObj(TObject*obj,Int_t pnum, Option_t *opt) {
  SetName(Form("%s_%s",obj->GetName(),"padobj"));
  fObject    = obj;
  fParent    = 0;
  fPadNumber = pnum;
  fOption    = opt;
}

GPadObj::~GPadObj() { fObject=0; }


ClassImp(GMemObj)

GMemObj::GMemObj(TObject *obj,TObject *par,TFile *file,Option_t *opt) {
  SetName(Form("%s_%s",obj->GetName(),"memobj")); 
  fThis      = obj;
  fParent    = par;
  fFile      = file;
  fObjName.assign(obj->GetName()); 
  fOption.assign(opt);
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
TList *GRootObjectManager::fCanvasList = 0;

//std::map<const char*,GMemObj > GRootObjectManager::fObjectsMap;
TList *GRootObjectManager::fObjectsMap = new TList;

GRootObjectManager::GRootObjectManager() { 
   printf("GRootObjectManager created.\n"); 
   fCanvasList = (TList*)gROOT->GetListOfCanvases();
   //fObjectList = (TList*)gROOT->GetList
   //this->Connect("Closed()","TCanvas",this,"RemoveCanvas()"
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
  if(!c) {
    std::map<TCanvas*,std::vector<GPadObj> >::iterator it;
    for(it = fCanvasMap.begin();it!=fCanvasMap.end();it++) {
      if(!fCanvasList->FindObject(it->first)) {
        //printf("\n  canvas 0x%08x has been removed from map.\n",it->first);
        fCanvasMap.erase(it->first);
      }  
    }
    return;
  }   
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
  if(strlen(object->GetName())<1)
     return; 
  if(object->InheritsFrom("TH1")   || // all histograms.
     object->InheritsFrom("TGraph") || // all graphs (include TCutG's)
     object->InheritsFrom("TCut")) {    // string cuts (esm:  "x<10")
     //if(fObjectsMap->count(object)<1) {
     //   fObjectsMap->insert(std::pair<TObject*,GMemObj>(object,GMemObj(object,par,file,opt))) ;
     //}
     if(!fObjectsMap->FindObject(Form("%s_%s",object->GetName(),"memobj")))
        fObjectsMap->Add(new GMemObj(object,par,file,opt));
  }
}

void GRootObjectManager::RemoveObject(TObject *object) {
  if(!object)
     return;
  if(strlen(object->GetName())<1)
     return; 
  TObject *obj = fObjectsMap->FindObject(Form("%s_%s",object->GetName(),"memobj"));
  if(obj) {
    fObjectsMap->Remove(obj);
    delete obj;
  }  
    //fObjectsMap->erase(object);
}

void GRootObjectManager::RemoveObject(const char *name) {
  if(strlen(name)<1)
     return; 
  TObject *obj = fObjectsMap->FindObject(Form("%s_%s",name,"memobj"));
  if(obj) {
    fObjectsMap->Remove(obj);
    delete obj;
  }  
}


bool GRootObjectManager::SetParent(TObject *object,TObject *parent) {
  if(!object)
     return false;
  if(strlen(object->GetName())<1)
     return false; 
  TObject *obj = fObjectsMap->FindObject(Form("%s_%s",object->GetName(),"memobj"));
  if(obj) {
     ((GMemObj*)obj)->SetParent(parent);
     return true;
  }   
  return false;
}


bool GRootObjectManager::SetFile(TObject *object,TFile *file) {
  if(!object)
     return false;
  if(strlen(object->GetName())<1)
     return false; 
  TObject *obj = fObjectsMap->FindObject(Form("%s_%s",object->GetName(),"memobj"));
  if(obj) {
     ((GMemObj*)obj)->SetFile(file);
     return true;
  }   
  return false;
}


TH1 *GRootObjectManager::GetNext1D(TObject *object) {
  //Returns the next TH1 currently managed from
  //from the key object. If no object is passed,
  //returns the first TH1 from the beginning of 
  //the map.
  
  if(fObjectsMap->GetSize()<2)
     return 0;
  if(!object) { 
    TIter iter(fObjectsMap);
    while(GMemObj *mobj = (GMemObj*)iter.Next()) { 
      TObject *s = mobj->GetObject();
      if(s->InheritsFrom("TH1") &&
        !s->InheritsFrom("TH2") &&
        !s->InheritsFrom("TH3")   ) {
        return (TH1*)s;
      }   
    }
    return 0;
  }

  if(strlen(object->GetName())<1)
     return 0; 
  const char *name = Form("%s_%s",object->GetName(),"memobj");
  GMemObj *mobj_start = (GMemObj*)fObjectsMap->FindObject(name);
  GMemObj *mobj_cur   = (GMemObj*)fObjectsMap->After(mobj_start);
  if(!mobj_cur)
     mobj_cur = (GMemObj*)fObjectsMap->First();
  while(mobj_cur != mobj_start) {
     TObject *s = mobj_cur->GetObject();
     if(s->InheritsFrom("TH1") &&
        !s->InheritsFrom("TH2") &&
        !s->InheritsFrom("TH3")   ) {
        return (TH1*)s;
     }   
     mobj_cur   = (GMemObj*)fObjectsMap->After(mobj_cur);
     if(!mobj_cur)
        mobj_cur = (GMemObj*)fObjectsMap->First();
  }
  return 0;
}


TH1 *GRootObjectManager::GetLast1D(TObject *object) {
  //Returns the last TH1 currently managed from
  //from the key object. If no object is passed,
  //returns the first TH1 from the beginning of 
  //the map.

  if(fObjectsMap->GetSize()<2)
     return 0;
  if(!object) { 
    TIter iter(fObjectsMap,kIterBackward);
    while(GMemObj *mobj = (GMemObj*)iter.Next()) { 
      TObject *s = mobj->GetObject();
      if(s->InheritsFrom("TH1") &&
         !s->InheritsFrom("TH2") &&
         !s->InheritsFrom("TH3")   ) {
         return (TH1*)s;
      }   
    }
    return 0;
  }

  if(strlen(object->GetName())<1)
     return 0; 
  const char *name = Form("%s_%s",object->GetName(),"memobj");
  GMemObj *mobj_start = (GMemObj*)fObjectsMap->FindObject(name);
  GMemObj *mobj_cur   = (GMemObj*)fObjectsMap->Before(mobj_start);
  if(!mobj_cur)
     mobj_cur = (GMemObj*)fObjectsMap->Last();
  while(mobj_cur != mobj_start) {
     TObject *s = mobj_cur->GetObject();
     if(s->InheritsFrom("TH1") &&
        !s->InheritsFrom("TH2") &&
        !s->InheritsFrom("TH3")   ) {
        return (TH1*)s;
     }   
     mobj_cur   = (GMemObj*)fObjectsMap->Before(mobj_cur);
     if(!mobj_cur)
        mobj_cur = (GMemObj*)fObjectsMap->Last();
  }
  return 0;
}







TH2    *GRootObjectManager::GetNext2D(TObject *object) { return 0; }
TH2    *GRootObjectManager::GetLast2D(TObject *object) { return 0; }
//TH3    *GRootObjectManager::GetNext3D(TObject *object) { }
//TH3    *GRootObjectManager::GetLast3D(TObject *object) { }
TGraph *GRootObjectManager::GetNextGraph(TObject *object) { return 0; }
TGraph *GRootObjectManager::GetLastGraph(TObject *object) { return 0; }



void GRootObjectManager::Update(Option_t *opt) {
  TString option = opt;
  bool u_mem    = option.Contains("MEM",TString::ECaseCompare::kIgnoreCase);
  bool u_file   = option.Contains("FILE",TString::ECaseCompare::kIgnoreCase);
  bool u_all    = option.Contains("ALL",TString::ECaseCompare::kIgnoreCase);
  bool u_clean  = option.Contains("CLEAN",TString::ECaseCompare::kIgnoreCase);
  

  if(fCanvasList && (fCanvasList->GetSize() != fCanvasMap.size())) {
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
  if(u_mem || u_all) {
    TFolder *folder = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Memory");
    ExtractObjects(folder->GetListOfFolders());
  }
  //Scan loaded files...
  if(u_file || u_all) {
    TFolder *folder = (TFolder*)gROOT->GetRootFolder()->FindObject("ROOT Files");
    ExtractObjects(folder->GetListOfFolders());
  }

  //finally, lets look for cuts...
  if(u_all) {
    TList *list = (TList*)gROOT->GetListOfSpecials()->FindObject("ROOT Files");
    ExtractObjects((TCollection*)list);
  }


  //I need to clean up still...
  if(u_clean) {
    TIter iter(fObjectsMap);
    while(GMemObj *mobj = ((GMemObj*)iter.Next())) {
      if(!gROOT->FindObjectAny(mobj->GetObjName()))
         RemoveObject(mobj->GetObjName());
    }
  }
  fObjectsMap->Sort();
  return;
}



void GRootObjectManager::ExtractObjects(TCollection *list) {
  TIter iter(list);
  TObject *object = iter.Next(); //iter.Next() returns nullptr if there is no next iterator
  while(object) {
    //printf("object->GetName() = %s \n",object->GetName());
    if(object->InheritsFrom("TFolder")) {
       ExtractObjects(((TFolder*)object)->GetListOfFolders());
    } else if(object->InheritsFrom("TDirectoryFile")) {
       ExtractObjectsFromFile((TDirectoryFile*)object);
    } else if(object->InheritsFrom("TDirectory")) {
       ExtractObjects(((TDirectory*)object)->GetList());
    } else {
       AddObject(object);
    }
	object = iter.Next();
  }
  return;
}

void GRootObjectManager::ExtractObjectsFromFile(TDirectoryFile *file) {
  TIter iter(file->GetListOfKeys());
  TKey *key=(TKey*)(iter.Next());//iter.Next() returns nullptr if there is no next iterator
  while(key) {
    TClass *rclass = gROOT->GetClass(key->GetClassName());
    //printf("rclasss->GetName() = %s\n",rclass->GetName());
    if(rclass->InheritsFrom("TFolder")) {
       ExtractObjects(((TFolder*)key->ReadObj())->GetListOfFolders());
    } else if(rclass->InheritsFrom("TDirectoryFile")) { 
       ExtractObjectsFromFile((TDirectoryFile*)key->ReadObj());
    } else if(rclass->InheritsFrom("TDirectory")) {
       ExtractObjects(((TDirectory*)key->ReadObj())->GetList());
    } else if((rclass->InheritsFrom("TH1")     ||
               rclass->InheritsFrom("TGraph")) &&
               !rclass->InheritsFrom("TH3")    ){
       AddObject(key->ReadObj());
    }
	key = (TKey*)(iter.Next());

  }
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
   printf("Canvases currently tracked:  %lu\n",fCanvasMap.size());
   printf("fCanvasMapList Content:\n");
   //fCanvasList->Print();
   std::map<TCanvas*,std::vector<GPadObj> >::iterator iter;
   int counter = 0;
   for(iter=fCanvasMap.begin();iter!=fCanvasMap.end();iter++)
       printf("\t% 2i.\t%s, contains %lu objects.\n",counter++,iter->first->GetName(),iter->second.size());

   printf("Objects currently tracked:  %i\n",fObjectsMap->GetSize());
   printf("fObjectsMap Content:\n");
   //fCanvasList->Print();
   //std::map<TObject*,GMemObj >::iterator iter2;
   counter = 0;
   //for(iter2=fObjectsMap->begin();iter2!=fObjectsMap->end();iter2++)
   //    printf("\t% 2i.\t%s [%s], parent[0x%08x] | file[0c%08x].\n",counter++,iter2->first->GetName(),iter2->first->IsA()->GetName(),
   //                                                                          iter2->second.GetParent(),iter2->second.GetFile());
   TIter Oiter(fObjectsMap);
   while(GMemObj *mobj = (GMemObj*)Oiter.Next()) {
      printf("\t% 2i.\t%s [%s], parent[%p] | file[%p].\n",counter++,mobj->GetObject()->GetName(),
                                                                            mobj->GetObject()->IsA()->GetName(),
                                                                            mobj->GetParent(),mobj->GetFile());

   }
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






