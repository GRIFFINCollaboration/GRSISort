#ifndef GRUTCANVAS_H
#define GRUTCANVAS_H

#include "TROOT.h"
#include "TCanvas.h"
#include "TRootCanvas.h"

#include "TH1.h"
#include "TLine.h"
#include "TCutG.h"

#include "GH2I.h"

/** \addtogroup GROOT
 *  @{
 */

///////////////////////////////////////////////////////////////////////////
///
/// \class GMarker
///
///////////////////////////////////////////////////////////////////////////

class GMarker : public TObject {
public:
   GMarker() = default;
   GMarker(int tmpX, int tmpY, TH1* hist);
   GMarker(const GMarker& marker) : TObject(marker) { static_cast<const GMarker>(marker).Copy(*this); }
   ~GMarker() override
   {
      if(fLineX != nullptr) { fLineX->Delete(); }
      if(fLineY != nullptr) { fLineY->Delete(); }
   }
   void Draw(Option_t* opt = "") override
   {
      if(fLineX != nullptr) {
         fLineX->Draw(opt);
      }
      if(fLineY != nullptr) {
         fLineY->Draw(opt);
      }
   }
   void Print(Option_t* option = "") const override
   {
      TObject::Print(option);
      std::cout << "fLineX = " << fLineX << ", fLineY = " << fLineY << std::endl;
      TString opt = option;
      opt.ToLower();
      if(opt.Contains("a")) {
         if(fLineX != nullptr) { fLineX->Print(); }
         if(fLineY != nullptr) { fLineY->Print(); }
      }
   }

   void SetColor(Color_t color)
   {
      if(fLineX != nullptr) {
         fLineX->SetLineColor(color);
      }
      if(fLineY != nullptr) {
         fLineY->SetLineColor(color);
      }
   }

   void SetStyle(Style_t style)
   {
      if(fLineX != nullptr) {
         fLineX->SetLineStyle(style);
      }
      if(fLineY != nullptr) {
         fLineY->SetLineStyle(style);
      }
   }
   void Update(Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax)
   {
      if(fLineX != nullptr) {
         fLineX->SetY1(ymin);
         fLineX->SetY2(ymax);
      }
      if(fLineY != nullptr) {
         fLineY->SetX1(xmin);
         fLineY->SetX2(xmax);
      }
   }

   void SetLineX(double x1, double x2, double y1, double y2)
   {
      if(fLineX == nullptr) { fLineX = new TLine(x1, y1, x2, y2); }
      else {
         fLineX->SetX1(x1);
         fLineX->SetX2(x2);
         fLineX->SetY1(y1);
         fLineX->SetY2(y2);
      }
   }

   void SetLineY(double x1, double x2, double y1, double y2)
   {
      if(fLineY == nullptr) { fLineY = new TLine(x1, y1, x2, y2); }
      else {
         fLineY->SetX1(x1);
         fLineY->SetX2(x2);
         fLineY->SetY1(y1);
         fLineY->SetY2(y2);
      }
   }

   void SetLocalX(const double& val)
   {
      if(fLineX != nullptr) {
         fLineX->SetX1(val);
         fLineX->SetX2(val);
      }
   }
   void SetLocalY(const double& val)
   {
      if(fLineY != nullptr) {
         fLineY->SetY1(val);
         fLineY->SetY2(val);
      }
   }
   void SetBinX(const int& val) { SetLineX(fHist->GetXaxis()->GetBinLowEdge(val), gPad->GetUymin(), fHist->GetXaxis()->GetBinLowEdge(val), gPad->GetUymax()); }
   void SetBinY(const int& val) { SetLineX(gPad->GetUxmin(), fHist->GetYaxis()->GetBinLowEdge(val), gPad->GetUxmax(), fHist->GetYaxis()->GetBinLowEdge(val)); }

   double GetLocalX() const
   {
      if(fLineX == nullptr) { return 0.; }
      return fLineX->GetX1();
   }
   double GetLocalY() const
   {
      if(fLineY == nullptr) { return 0.; }
      return fLineY->GetY1();
   }
   int GetBinX() const
   {
      if(fLineX == nullptr) { return -1; }
      return fHist->GetXaxis()->FindBin(fLineX->GetX1());
   }
   int GetBinY() const
   {
      if(fLineY == nullptr) { return -1; }
      return fHist->GetYaxis()->FindBin(fLineY->GetY1());
   }

   void SetHist(const TH1* val) { fHist = val; }

private:
   const TH1* fHist{nullptr};
   TLine*     fLineX{nullptr};
   TLine*     fLineY{nullptr};

public:
   void Copy(TObject& object) const override;
   bool operator<(const GMarker& rhs) const
   {
      if(fLineX != nullptr && rhs.fLineX != nullptr) { return fLineX->GetX1() < rhs.fLineX->GetX1(); }
      return false;
   }
   ClassDefOverride(GMarker, 0)
};

///////////////////////////////////////////////////////////////////////////
///
/// \class GCanvas
///
/// Reimplementation of TCanvas. Adds different actions for mouse clicks:
/// - single click: stores position (only for TLevelScheme).
/// - double click: add GMarker if the last histogram found in the gPad is 1D or 2D.
/// - shift click: draw the parent of GH1D histogram on new canvas or draw all
///   histograms on a new canvas, or (for 2D histograms) create new GH2D from
///   histogram and draw it on a new canvas (using "colz").
/// - control click: doesn't do anything right now, code for TCutG is commented out.
///
/// For TLevelSchemes we also have:
/// - mouse wheel to change zoom level
/// - mouse drag to zoom in
/// - u unzooms.
///
/// Also adds keyboard controls for 1D histograms:
/// - left/right arrow moves the range left/right by 50%.
/// - up/down arrow on GH1D histograms selects the next/previous histogram and draws it.
/// - F2 opens editor.
/// - F9 shows crosshairs.
/// - b	Set the background, how it is set depends on B.
/// - B	Cycle through types of automatic background subtraction used when projecting with p. Current types include: No subtraction, Fraction of the total, subtract gate from the 3rd marker (gate size set to the distance between marker 1 and 2).
/// - d	Open popup.
/// - e	Expand the x-axis range between the last two markers.
/// - E	Bring up dialogue box used to set desired x-axis range.
/// - f	If markers have been set, do a GPeak fit between last two markers on the last histogram (skewed gaus for gamma-rays with automatic bg).
/// - F	TPeak Fit (skewed gaus for gamma-rays with automatic bg).
/// - g	Simple Gaus fit between the last to marks, displays results of the fit RESULTS STILL NEED TO BE VERIFIED
/// - i	Raw integral of counts between the two markers
/// - I	TODO: Background subtracted integral of counts between the two markers
/// - l/y	Toggle y-axis from linear to logarithmic and vice versa.
/// - m	Toggle on marker mode; when on, the histogram will remember and display the last four clicks as marks on the histogram.
/// - M	Toggle off marker mode.
/// - n	Remove all markers / functions drawn on the histogram.
/// - N	Remove all markers and the LAST function drawn on the histogram.
/// - o	Unzoom the entire histogram.
/// - p	If the 1d hist was made using the global ProjectionX/ProjectionY; gating the original 2D matrix this histogram came from is possible by placing markers around the gate and pressing p. The gates spectra is immediately drawn.
/// - P	Draws parent histogram???
/// - q	If markers have been set, fit a GPeak between the last two markers on the first histogram (skewed gaus for gamma-rays with automatic bg).
/// - r	Expand the y-axis range between the last two markers.
/// - R	Bring up the dialogue box used to set the desired y-axis range.
/// - s	Show peak values.
/// - S	Remove peak values.
///
/// And for 2D histograms these keyboard controls are added:
/// - left/right arrow moves the range left/right by 50%.
/// - up/down arrow moves the range up/down by 50%.
/// - c	Add the initialized cut to the list of cuts (see i).
/// - e	Expand the x- and y-axis between the last two markers.
/// - E	Bring up dialogue box used to set desired x-axis range.
/// - g	Create a cut from the last two markers and add it to the histogram.
/// - i	Initialize a new cut.
/// - l/z	Toggle z-axis from linear to logarithmic and vice versa.
/// - n	Remove all markers / functions drawn on the histogram.
/// - o	Unzoom the entire histogram.
/// - P	Get projections from this histogram and draw the first one if it exists.
/// - r	Expand the y-axis between the last two markers.
/// - R	Bring up the dialogue box to set the desired y-axis range.
/// - s	Save the cuts that have been created.
/// - x	Create projection of the histogram onto the x-axis and draw in a new canvas.
/// - X	Create "y summary" of the histogram, i.e. a projection of the first y-bin onto the x-axis that yields a non-empty histogram and draw it on a new canvas.
/// - y	Create projection of the histogram onto the x-axis and draw in a new canvas.
/// - Y	Create "x summary" of the histogram, i.e. a projection of the first x-bin onto the y-axis that yields a non-empty histogram and draw it on a new canvas.
///
///////////////////////////////////////////////////////////////////////////

class GCanvas : public TCanvas {
public:
   GCanvas(Bool_t build = kTRUE);
   GCanvas(const char* name, const char* title = "", Int_t form = 1);
   GCanvas(const char* name, const char* title, Int_t winw, Int_t winh);
   GCanvas(const char* name, Int_t winw, Int_t winh, Int_t winid);
   GCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t winw, Int_t winh, bool gui = false);
   ~GCanvas() override;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
   void HandleInput(int event, Int_t x, Int_t y);
#pragma GCC diagnostic pop
   void Draw(Option_t* opt = "") override;

   static GCanvas* MakeDefCanvas();

   Int_t GetNMarkers() { return static_cast<Int_t>(fMarkers.size()); }
   void  SetMarkerMode(bool flag = true) { fMarkerMode = flag; }

   static TF1* GetLastFit();

private:
   void GCanvasInit();

   void UpdateStatsInfo(int, int);

   static double gLastX;
   static double gLastY;

   bool fGuiEnabled{false};

   bool                   fMarkerMode{false};
   std::vector<GMarker*>  fMarkers;
   std::vector<GMarker*>  fBackgroundMarkers;
   EBackgroundSubtraction fBackgroundMode = EBackgroundSubtraction::kNoBackground;
   std::vector<TCutG*>    fCuts;
   char*                  fCutName{nullptr};
   void                   AddMarker(int, int, TH1* hist);
   void                   RemoveMarker(Option_t* opt = "");
   void                   OrderMarkers();
   void                   RedrawMarkers();
   bool                   SetBackgroundMarkers();
   bool                   CycleBackgroundSubtraction();

   static std::vector<TH1*> FindHists(int dim = 1);
   static std::vector<TH1*> FindAllHists();

public:
   bool HandleArrowKeyPress(Event_t* event, UInt_t* keysym);
   bool HandleKeyboardPress(Event_t* event, UInt_t* keysym);
   bool HandleMousePress(Int_t event, Int_t x, Int_t y);
   bool HandleMouseShiftPress(Int_t event, Int_t x, Int_t y);
   bool HandleMouseControlPress(Int_t event, Int_t x, Int_t y);
   bool HandleWheel(Int_t event, Int_t x, Int_t y);
   bool StorePosition(Int_t event, Int_t x, Int_t y);
   bool Zoom(Int_t event, Int_t x, Int_t y);

private:
   bool ProcessNonHistKeyboardPress(Event_t* event, UInt_t* keysym);
   bool Process1DArrowKeyPress(Event_t* event, UInt_t* keysym);
   bool Process1DKeyboardPress(Event_t* event, const UInt_t* keysym);
   bool Process1DMousePress(Int_t event, Int_t x, Int_t y);

   bool Process2DArrowKeyPress(Event_t* event, const UInt_t* keysym);
   bool Process2DKeyboardPress(Event_t* event, const UInt_t* keysym);
   bool Process2DMousePress(Int_t event, Int_t x, Int_t y);

   TRootCanvas* fRootCanvas{nullptr};

   ClassDefOverride(GCanvas, 2);
};
/*! @} */
#endif
