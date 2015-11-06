#ifndef GRUTCANVAS_H
#define GRUTCANVAS_H

/** \addtogroup GROOT
 *  @{
 */

#include "TROOT.h"
#include "TCanvas.h"
#include "TRootCanvas.h"
#include "TPeak.h"

#include "TLine.h"

class GMarker : public TObject {
	public:
	   GMarker() : fX(-1), fY(-1), fLocalX(0.0), fLocalY(0.0), fLineX(0), fLineY(0) { }
      GMarker(const GMarker &m) : TObject() { m.Copy(*this); }
		virtual ~GMarker() { if(fLineX) fLineX->Delete(); if(fLineY) fLineY->Delete(); }
		int fX;
		int fY;
		double fLocalX;
		double fLocalY;
		TLine* fLineX;
		TLine* fLineY;
		void Copy(TObject &object) const;
		bool operator<(const GMarker &rhs) const { return fX < rhs.fX; }

/// \cond CLASSIMP
		ClassDef(GMarker,0)
/// \endcond
};

class GCanvas : public TCanvas {
   public:
      GCanvas(Bool_t build = kTRUE);
      GCanvas(const char* name, const char* title = "", Int_t form = 1);
      GCanvas(const char* name, const char* title, Int_t ww, Int_t wh);
      GCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid);
      GCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh);
      virtual ~GCanvas();

      //void ProcessEvent(Int_t event,Int_t x,Int_t y,TObject* obj);
      //void CatchEvent(Int_t event,Int_t x,Int_t y,TObject* obj);

      void HandleInput(EEventType event,Int_t x,Int_t y);

      void Draw(Option_t* opt="");

      static GCanvas* MakeDefCanvas(); 
      Int_t  GetNMarkers()    { return fMarkers.size(); }
      Int_t  GetNBG_Markers() { return fBGMarkers.size(); }
      void SetMarkerMode(bool flag=true) { fMarkerMode = flag; }

      static void SetBackGroundSubtractionType();

      TF1* GetLastFit();
      static void Prompt() { gROOT->ProcessLine("Getlinem(kInit,TGRSIint::instance()->GetPrompt())"); }

   private:
      void GCanvasInit();

      void UpdateStatsInfo(int,int);

/// \cond CLASSIMP
      ClassDef(GCanvas,1);
/// \endcond

      static int fLastX;
      static int fLastY;

      bool fStatsDisplayed;
      bool fMarkerMode;
      std::vector<GMarker*> fMarkers;
      void AddMarker(int,int,int dim=1);
      void RemoveMarker();
      void OrderMarkers();

      std::vector<GMarker*> fBGMarkers;
      void AddBGMarker(GMarker* mark);
      void RemoveBGMarker();
      void ClearBGMarkers();
      void OrderBGMarkers();



      std::vector<TH1*> Find1DHists();
      std::vector<TH1*> FindAllHists();

      //void HandleKeyPress(int event,int x,int y,TObject* obj);
      bool HandleArrowKeyPress(Event_t* event,UInt_t* keysym);
      bool HandleKeyboardPress(Event_t* event,UInt_t* keysym);
      bool HandleMousePress(Int_t event,Int_t x,Int_t y);


      bool SetBackGround(GMarker* m1=0,GMarker* m2=0,GMarker* m3=0,GMarker* m4=0);
      bool SetLinearBG(GMarker* m1=0,GMarker* m2=0);
      bool SetConstantBG(); //GMarker* m1=0,GMarker* m2=0);
      bool SetBGGate(GMarker* m1,GMarker* m2,GMarker* m3=0,GMarker* m4=0);
      
      TH1* GetBackGroundHist(GMarker* addlow,GMarker* addhigh);
      
      bool GausFit(GMarker* m1=0,GMarker* m2=0);
      bool GausBGFit(GMarker* m1=0,GMarker* m2=0);
      bool PeakFit(GMarker* m1=0,GMarker* m2=0);
      bool PeakFitQ(GMarker* m1=0,GMarker* m2=0);

      bool Integrate(GMarker* m1=0,GMarker* m2=0);
      bool IntegrateBG(GMarker* m1=0,GMarker* m2=0);

      static int fBGSubtraction_type;

      Window_t fCanvasWindowID;
      TRootCanvas* fRootCanvas;
};
/*! @} */
#endif
