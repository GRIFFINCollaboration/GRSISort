#ifndef GRUTCANVAS_H
#define GRUTCANVAS_H

#include "TCanvas.h"
#include "TRootCanvas.h"
#include "TPeak.h"

#include "TLine.h"

class GMarker {
  public:
    GMarker():x(-1),y(-1),linex(0),liney(0),localx(0.0),localy(0.0) { }
    ~GMarker() { if(linex) linex->Delete(); if(liney) liney->Delete(); }
    int x;
    int y;
    double localx;
    double localy;
    TLine *linex;
    TLine *liney;
    ClassDef(GMarker,0)
};



class GCanvas : public TCanvas {
   public:
      GCanvas(Bool_t build = kTRUE);
      GCanvas(const char* name, const char* title = "", Int_t form = 1);
      GCanvas(const char* name, const char* title, Int_t ww, Int_t wh);
      GCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid);
      GCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh);
      virtual ~GCanvas();

      //void ProcessEvent(Int_t event,Int_t x,Int_t y,TObject *obj);
      //void CatchEvent(Int_t event,Int_t x,Int_t y,TObject *obj);

      void HandleInput(Int_t event,Int_t x,Int_t y);

      void Draw(Option_t *opt="");

      static GCanvas *MakeDefCanvas(); 
      Int_t  GetNMarkers() { return fMarkers.size(); }
      void SetMarkerMode(bool flag=true) {fMarkerMode = flag;}

      static void SetBackGroundSubtractionType();

      TF1 *GetLastFit();

   private:
      void GCanvasInit();

      void UpdateStatsInfo(int,int);

      ClassDef(GCanvas,1);

      static int lastx;
      static int lasty;

      bool fStatsDisplayed;
      bool fMarkerMode;
      std::vector<GMarker*> fMarkers;
      void AddMarker(int,int,int dim=1);
      void RemoveMarker();

      std::vector<TH1*> Find1DHists();

      //void HandleKeyPress(int event,int x,int y,TObject *obj);
      bool HandleArrowKeyPress(Event_t *event,UInt_t *keysym);
      bool HandleKeyboardPress(Event_t *event,UInt_t *keysym);
      bool HandleMousePress(Int_t event,Int_t x,Int_t y);

      bool SetLinearBG(GMarker *m1=0,GMarker *m2=0);
      bool GausFit(GMarker *m1=0,GMarker *m2=0);
      bool GausBGFit(GMarker *m1=0,GMarker *m2=0);
      bool PeakFit(GMarker *m1=0,GMarker *m2=0);
      bool PeakFitQ(GMarker *m1=0,GMarker *m2=0);

      static int fBGSubtraction_type;


      Window_t fCanvasWindowID;
      TRootCanvas *fRootCanvas;

};

#endif
