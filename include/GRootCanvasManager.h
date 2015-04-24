#ifndef GROOTCANVASMANAGER__H
#define GROOTCANVASMANAGER__H

#include <TObject.h>
#include <TList.h>
#include <TVirtualPad.h>
#include <TCanvas.h>

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
      //std::vector<std::string> fSource;
      std::string  fOption;
};


class GRootCanvasManager {
  public:
    static GRootCanvasManager *Instance();
    virtual ~GRootCanvasManager();

    static void AddCanvas(TCanvas*);
    static void RemoveCanvas(TCanvas*);
    void Update();

  private:
    GRootCanvasManager();
    static GRootCanvasManager *fGRootCanvasManager;

    static std::map<TCanvas*,std::vector<GPadObj*> > fCanvasMap;

    //TList fOneD;
    //TList fTwoD;

  //ClassDef(GRootCanvasManager,0)
};

#endif
