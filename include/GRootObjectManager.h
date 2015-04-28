#ifndef GROOTOBJECTMANAGER__H
#define GROOTOBJECTMANAGER__H

#include <TObject.h>
#include <TList.h>
#include <TVirtualPad.h>

#include <GCanvas.h>

#include <string>
#include <map>

class GPadObj {
  public:
     GPadObj(TObject*obj,Int_t pnum,Option_t *opt="");
     ~GPadObj();

   private:
      //PadObj(); 
      TObject*   fObject;
      int        fPadNumber;
      GCanvas*   fCanvas;
      //std::vector<std::string> fSource;
      std::string  fOption;
};


class GRootObjectManager {
  public:
    static GRootObjectManager *Instance();
    virtual ~GRootObjectManager();

    static void AddCanvas(GCanvas*);
    static void RemoveCanvas(GCanvas*);

   
    //bool AddObject(TObject*);    
    //bool RemoveObject(TObject*);    

    void Update();

    //TH1    *GetNext1D(TH1*);
    //TH2    *GetNext2D(TH2*);
    //TH3    *GetNext3D(TH3*);
    //TGraph *GetNextGraph(TGraph*);

  private:
    GRootObjectManager();
    static GRootObjectManager *fGRootObjectManager;

    static std::map<TCanvas*,std::vector<GPadObj*> > fCanvasMap;

    TList fCanvas;

    TList fOneDHists;
    TList fTwoDHists;
    TList fThreeDHists;
    TList fGraphs;
    TList fMisc;

    void UpdateLists();    

  //ClassDef(GRootObjectManager,0)
};

#endif
