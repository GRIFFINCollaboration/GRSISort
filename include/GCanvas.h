#ifndef GRUTCANVAS_H
#define GRUTCANVAS_H

#include "TROOT.h"
#include "TCanvas.h"
#include "TRootCanvas.h"

#include "TH1.h"
#include "TLine.h"
#include "TCutG.h"

#include "GH2I.h"

class GMarker : public TObject {
public:
   GMarker() : fLineX(nullptr), fLineY(nullptr) {}
	GMarker(int tmpX, int tmpY, TH1* hist);
   GMarker(const GMarker& m) : TObject(m) { ((GMarker&)m).Copy(*this); }
   ~GMarker() override
   {
		if(fLineX != nullptr) fLineX->Delete();
		if(fLineY != nullptr) fLineY->Delete();
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
		std::cout<<"fLineX = "<<fLineX<<", fLineY = "<<fLineY<<std::endl;
		TString opt = option;
		opt.ToLower();
		if(opt.Contains("a")) {
			if(fLineX != nullptr) fLineX->Print();
			if(fLineY != nullptr) fLineY->Print();
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
		if(fLineX == nullptr) fLineX = new TLine(x1, y1, x2, y2);
		else {
			fLineX->SetX1(x1);
			fLineX->SetX2(x2);
			fLineX->SetY1(y1);
			fLineX->SetY2(y2);
		}
	}

	void SetLineY(double x1, double x2, double y1, double y2)
	{
		if(fLineY == nullptr) fLineY = new TLine(x1, y1, x2, y2);
		else {
			fLineY->SetX1(x1);
			fLineY->SetX2(x2);
			fLineY->SetY1(y1);
			fLineY->SetY2(y2);
		}
	}

	void SetLocalX(const double& val) { if(fLineX != nullptr) { fLineX->SetX1(val); fLineX->SetX2(val); } }
	void SetLocalY(const double& val) { if(fLineY != nullptr) { fLineY->SetY1(val); fLineY->SetY2(val); } }
	void SetBinX(const int& val) { SetLineX(fHist->GetXaxis()->GetBinLowEdge(val), gPad->GetUymin(), fHist->GetXaxis()->GetBinLowEdge(val), gPad->GetUymax()); }
	void SetBinY(const int& val) { SetLineX(gPad->GetUxmin(), fHist->GetYaxis()->GetBinLowEdge(val), gPad->GetUxmax(), fHist->GetYaxis()->GetBinLowEdge(val)); }

	double GetLocalX() const { if(fLineX == nullptr) return 0.; return fLineX->GetX1(); }
	double GetLocalY() const { if(fLineY == nullptr) return 0.; return fLineY->GetY1(); }
	int GetBinX() const { if(fLineX == nullptr) return -1; return fHist->GetXaxis()->FindBin(fLineX->GetX1()); }
	int GetBinY() const { if(fLineY == nullptr) return -1; return fHist->GetYaxis()->FindBin(fLineY->GetY1()); }

	void SetHist(const TH1* val) { fHist = val; }

private:
	const TH1* fHist;
	TLine* fLineX{nullptr};
	TLine* fLineY{nullptr};
public:
	void Copy(TObject& object) const override;
	bool operator<(const GMarker& rhs) const { if(fLineX != nullptr && rhs.fLineX != nullptr) return fLineX->GetX1() < rhs.fLineX->GetX1(); return false; }
	ClassDefOverride(GMarker, 0)
};

class GCanvas : public TCanvas {
public:
	GCanvas(Bool_t build = kTRUE);
	GCanvas(const char* name, const char* title = "", Int_t form = 1);
	GCanvas(const char* name, const char* title, Int_t ww, Int_t wh);
	GCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid);
	GCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh, bool gui = false);
	~GCanvas() override;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
	void HandleInput(int event, Int_t x, Int_t y);
#pragma GCC diagnostic pop
	void Draw(Option_t* opt = "") override;

	static GCanvas* MakeDefCanvas();

	Int_t GetNMarkers() { return fMarkers.size(); }
	void SetMarkerMode(bool flag = true) { fMarkerMode = flag; }

	TF1* GetLastFit();

private:
	void GCanvasInit();

	void UpdateStatsInfo(int, int);

	static double gLastX;
	static double gLastY;

	bool fGuiEnabled{false};

	bool                   fMarkerMode{false};
	std::vector<GMarker*>  fMarkers;
	std::vector<GMarker*>  fBackgroundMarkers;
	EBackgroundSubtraction fBackgroundMode;
	std::vector<TCutG*>    fCuts;
	char*                  fCutName;
	void AddMarker(int, int, TH1* hist);
	void RemoveMarker(Option_t* opt = "");
	void OrderMarkers();
	void RedrawMarkers();
	bool SetBackgroundMarkers();
	bool CycleBackgroundSubtraction();

	std::vector<TH1*> FindHists(int dim = 1);
	std::vector<TH1*> FindAllHists();

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
	bool Process1DKeyboardPress(Event_t* event, UInt_t* keysym);
	bool Process1DMousePress(Int_t event, Int_t x, Int_t y);

	bool Process2DArrowKeyPress(Event_t* event, UInt_t* keysym);
	bool Process2DKeyboardPress(Event_t* event, UInt_t* keysym);
	bool Process2DMousePress(Int_t event, Int_t x, Int_t y);

private:
	TRootCanvas* fRootCanvas{nullptr};

	ClassDefOverride(GCanvas, 2);
};

#endif
