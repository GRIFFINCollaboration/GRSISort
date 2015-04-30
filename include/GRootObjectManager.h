#ifndef GROOTOBJECTMANAGER__H
#define GROOTOBJECTMANAGER__H

#include <TObject.h>
#include <TList.h>
#include <TVirtualPad.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TGraph.h>

#include <GCanvas.h>

#include <string>
#include <map>

class GPadObj {
  public:
     GPadObj(TObject*obj,Int_t pnum=0,Option_t *opt="");
     ~GPadObj();

   private:
      //PadObj(); 
      TObject*   fObject;
      TObject*   fParent;
      int        fPadNumber;
      GCanvas*   fCanvas;
      //std::vector<std::string> fSource;
      std::string  fOption;
};

class GMemObj {
  public:
    GMemObj(TObject *obj, TObject *par=0,TFile *file=0,Option_t *opt="");
    ~GMemObj();

    TObject *GetParent() { return fParent; }
    TFile   *GetFile()   { return fFile;   }
    void     SetParent(TObject *parent) { fParent = parent; }
    void     SetFile(TFile *file)       { fFile   = file;   }

  private:
    TObject *fThis;
    TObject *fParent;
    TFile   *fFile;
    std::string fOption;
};



class GRootObjectManager {
  public:
    static GRootObjectManager *Instance();
    virtual ~GRootObjectManager();

    static void AddCanvas(GCanvas*);
    static void AddCanvas(TCanvas*);
    static void RemoveCanvas(GCanvas*);
    static void RemoveCanvas(TCanvas*);

    static void AddObject(TObject *obj,TObject *par=0,TFile *file=0,Option_t *opt="");
    static void RemoveObject(TObject*);
  
    static bool SetParent(TObject *object,TObject *parent);
    static bool SetFile(TObject *object,TFile *file);

    TH1    *GetNext1D(TObject *object =0);
    TH2    *GetNext2D(TObject *object =0);
    TGraph *GetNextGraph(TObject *object=0);


    void Update();
    void Print();

  private:
    GRootObjectManager();
    static GRootObjectManager *fGRootObjectManager;

    static std::map<TCanvas*,std::vector<GPadObj> > fCanvasMap;
    TList *fCanvasList;


    static std::map<TObject*,GMemObj > fObjectsMap;


    //static std::map<TObject*,std::vector<GMemObj*> > f1DHistsMap;
    //static std::map<TObject*,std::vector<GMemObj*> > f2DHistsMap;
    //static std::map<TObject*,std::vector<GMemObj*> > f3DHistsMap;
    //static std::map<TObject*,std::vector<GMemObj*> > fGraphsMap;
    //static std::map<TObject*,std::vector<GMemObj*> > fCutsMap;
    //static std::map<TObject*,std::vector<GMemObj*> > fMiscMap;

    //void Cleanup();
    void ExtractObjects(TCollection*);

  ClassDef(GRootObjectManager,0)
};

#endif
