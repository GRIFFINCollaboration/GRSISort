#include "GRootCommands.h"
#include "Globals.h"
#include <cstdio>
//#include <string>
#include <sstream>
#include <fstream>

#include "TRint.h"
#include "TTree.h"
#include "Getline.h"
#include "TAxis.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TPolyMarker.h"
#include "TSpectrum.h"
#include "TText.h"
#include "TExec.h"
#include "TKey.h"
#include "TObject.h"
#include "TObjArray.h"
#include "TH1.h"
#ifdef HAS_CORRECT_PYTHON_VERSION
#include "TPython.h"
#endif
#include "TTimer.h"
#include "TF1.h"

#include "GCanvas.h"
#include "GPeak.h"
#include "TPeak.h"
#include "GGaus.h"
#include "GH2D.h"
#include "GH1D.h"
#include "TGRSIOptions.h"
#include "GNotifier.h"

void Help()
{
   std::cout << "This is helpful information." << std::endl;
}

void Commands()
{
   std::cout << "this is a list of useful commands." << std::endl;
}

void Prompt()
{
   Getlinem(EGetLineMode::kInit, (static_cast<TRint*>(gApplication))->GetPrompt());
}

void Version()
{
   int ret = system(Form("%s/bin/grsi-config --version", getenv("GRSISYS")));
   if(ret == -1) {
      std::cout << "Failed to call grsi-config!" << std::endl;
   }
}

bool GetProjection(GH2D* hist, double low, double high, double bg_low, double bg_high)
{
   if(hist == nullptr) {
      return false;
   }
   GCanvas* C_projections = nullptr;
   GCanvas* C_gammagamma  = nullptr;
   if(gROOT->GetListOfCanvases()->FindObject("C_projections")) {
      C_projections = static_cast<GCanvas*>(gROOT->GetListOfCanvases()->FindObject("C_projections"));
   } else {
      C_projections = new GCanvas("C_projections", "Projection Canvas", 0, 0, 1450, 600);
      C_projections->Divide(2, 1);
   }

   if(gROOT->GetListOfCanvases()->FindObject("C_gammagamma")) {
      C_gammagamma = static_cast<GCanvas*>(gROOT->GetListOfCanvases()->FindObject("C_gammagamma"));
   } else {
      C_gammagamma = new GCanvas("C_gammagamma", "Gamma-Gamma Canvas", 1700, 0, 650, 650);
   }

   C_gammagamma->cd();
   hist->Draw();

   C_projections->cd(1);
   GH1D* Proj_x = hist->ProjectionX("Gamma_Gamma_xProjection");

   GH1D* Proj_x_Clone = static_cast<GH1D*>(Proj_x->Clone());
   GH1D* Proj_gated   = nullptr;

   if(bg_high > 0 && bg_low > 0) {
      Proj_x->SetTitle(
         Form("Projection with Gate From [%.01f,%.01f] and Background [%.01f,%.01f]", low, high, bg_low, bg_high));
   } else {
      Proj_x->SetTitle(Form("Projection with Gate From [%.01f,%.01f] NO background", low, high));
   }
   Proj_x->GetXaxis()->SetTitle("Energy [keV]");
   Proj_x->GetYaxis()->SetTitle("Counts ");

   double Grace    = 300;
   double ZoomHigh = high + Grace;
   double ZoomLow  = low - Grace;
   if(bg_high > 0 && bg_high > high) {
      ZoomHigh = bg_high + Grace;
   }
   if(bg_low > 0 && bg_low < low) {
      ZoomLow = bg_low - Grace;
   }

   Proj_x->GetXaxis()->SetRangeUser(ZoomLow, ZoomHigh);
   Proj_x->Draw();
   double Projx_Max = Proj_x->GetMaximum();
   double Projx_Min = Proj_x->GetMinimum();

   auto* CutLow  = new TLine(low, Projx_Min, low, Projx_Max);
   auto* CutHigh = new TLine(high, Projx_Min, high, Projx_Max);
   auto* BGLow   = new TLine(bg_low, Projx_Min, bg_low, Projx_Max);
   auto* BGHigh  = new TLine(bg_high, Projx_Min, bg_high, Projx_Max);
   CutLow->SetLineColor(kRed);
   CutHigh->SetLineColor(kRed);
   CutLow->SetLineWidth(2);
   CutHigh->SetLineWidth(2);
   BGLow->SetLineColor(kBlue);
   BGHigh->SetLineColor(kBlue);
   BGLow->SetLineWidth(2);
   BGHigh->SetLineWidth(2);
   BGLow->SetLineStyle(kDashed);
   BGHigh->SetLineStyle(kDashed);
   CutLow->Draw("same");
   CutHigh->Draw("same");
   if(bg_low > 0 && bg_high > 0) {
      BGHigh->Draw("same");
      BGLow->Draw("same");
      Proj_gated = Proj_x_Clone->Project_Background(low, high, bg_low, bg_high, EBackgroundSubtraction::kRegionBackground);
   } else {
      Proj_gated = Proj_x_Clone->Project(low, high);
   }

   if(bg_high > 0 && bg_low > 0) {
      Proj_gated->SetTitle(Form("Gate From [%.01f,%.01f] with Background [%.01f,%.01f]", low, high, bg_low, bg_high));
   } else {
      Proj_gated->SetTitle(Form("Gate From [%.01f,%.01f] NO Background", low, high));
   }
   Proj_gated->GetXaxis()->SetTitle("Energy [keV]");
   Proj_gated->GetYaxis()->SetTitle("Counts");

   C_projections->cd(2);
   Proj_gated->Draw();
   return true;
}

int LabelPeaks(TH1* hist, double sigma, double thresh, Option_t*)
{
   TSpectrum::StaticSearch(hist, sigma, "Qnodraw", thresh);
   auto* polyMarker = static_cast<TPolyMarker*>(hist->GetListOfFunctions()->FindObject("TPolyMarker"));
   if(polyMarker == nullptr) {
      // something has gone wrong....
      return 0;
   }
   auto* array = static_cast<TObjArray*>(hist->GetListOfFunctions()->FindObject("PeakLabels"));
   if(array != nullptr) {
      hist->GetListOfFunctions()->Remove(static_cast<TObject*>(array));
      array->Delete();
   }
   array = new TObjArray();
   array->SetName("PeakLabels");
   int n = polyMarker->GetN();
   if(n == 0) {
      return n;
   }
   double* markerX = polyMarker->GetX();
   for(int i = 0; i < n; i++) {
      double y = 0;
      for(double i_x = markerX[i] - 3; i_x < markerX[i] + 3; i_x++) {
         if((hist->GetBinContent(hist->GetXaxis()->FindBin(i_x))) > y) {
            y = hist->GetBinContent(hist->GetXaxis()->FindBin(i_x));
         }
      }
      y += y * 0.1;
      auto* text = new TText(markerX[i], y, Form("%.1f", markerX[i]));
      text->SetTextSize(0.025);
      text->SetTextAngle(90);
      text->SetTextAlign(12);
      text->SetTextFont(42);
      text->SetTextColor(hist->GetLineColor());
      array->Add(text);
   }
   hist->GetListOfFunctions()->Remove(polyMarker);
   polyMarker->Delete();
   hist->GetListOfFunctions()->Add(array);
   return n;
}

bool ShowPeaks(TH1** hists, unsigned int nhists, double sigma, double thresh)
{
   int num_found = 0;
   for(unsigned int i = 0; i < nhists; i++) {
      if(TObject* obj = hists[i]->GetListOfFunctions()->FindObject("PeakLabels")) {
         hists[i]->GetListOfFunctions()->Remove(obj);
         (static_cast<TObjArray*>(obj))->Delete();
      }
      num_found += LabelPeaks(hists[i], sigma, thresh, "");
   }
   return num_found != 0;
}

bool RemovePeaks(TH1** hists, unsigned int nhists)
{
   bool flag = false;
   for(unsigned int i = 0; i < nhists; i++) {
      if(TObject* obj = hists[i]->GetListOfFunctions()->FindObject("PeakLabels")) {
         hists[i]->GetListOfFunctions()->Remove(obj);
         (static_cast<TObjArray*>(obj))->Delete();
         flag = true;
      }
   }
   return flag;
}

std::vector<TH1*> FindHists(int dim)
{
   std::vector<TH1*> tempVec;
   for(auto* obj : *(gPad->GetListOfPrimitives())) {
      if(obj->InheritsFrom(TH1::Class())) {
         TH1* hist = static_cast<TH1*>(obj);
         if(hist->GetDimension() == dim) {
            tempVec.push_back(hist);
         }
      }
   }
   return tempVec;
}

bool Move1DHistogram(const Int_t& key, TH1* histogram)
{
   /// Moves displayed 1D histograms by 50% of the visible range left, right.
   /// For "normal" TH1 histograms up/down scales the y-axis up/down by a factor of 2.
   /// For GH1D histograms up/down selects the next (up) or previous (down) GH1D histogram.
   bool              edited = false;
   std::vector<TH1*> hists;
   if(histogram != nullptr) {
      hists.push_back(histogram);
   } else {
      hists = FindHists(1);
   }
   if(hists.empty()) {
      return edited;
   }

   // get first and last bin in current range
   int first = hists.at(0)->GetXaxis()->GetFirst();
   int last  = hists.at(0)->GetXaxis()->GetLast();

   // first is 1 if no range is defined but can be 0, last is fNbins if no range is defined but can be 0
   // so min will always be 0, and max will always be fNbins+1
   int min = std::min(first, 0);
   int max = std::max(last, hists.at(0)->GetXaxis()->GetNbins() + 1);

   int xdiff = last - first;
   int mdiff = max - min - 2;   // this will always be fNbins-1

   // try and cast histogram to GH1D, will be a null pointer if the histogram is not a GH1D
   GH1D* gHist = dynamic_cast<GH1D*>(hists.at(0));

   // get current y-range
   double yMin = hists.at(0)->GetMinimum();
   double yMax = hists.at(0)->GetMaximum();

   switch(key) {
   case kGRSIArrowLeft:
      if(mdiff > xdiff) {
         // try and move left by half the current range
         if(first == (min + 1)) {
            // if first is 1 we can't go any further left
         } else if((first - (xdiff / 2)) < min) {
            first = min + 1;
            last  = min + (xdiff) + 1;
         } else {
            first = first - (xdiff / 2);
            last  = last - (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(first, last);
      }
      edited = true;
      break;
   case kGRSIArrowRight:
      if(mdiff > xdiff) {
         // try and move right by half the current range
         if(last == (max - 1)) {
            // last is fNbins so we can't move further right
         } else if((last + (xdiff / 2)) > max) {
            first = max - 1 - (xdiff);
            last  = max - 1;
         } else {
            last  = last + (xdiff / 2);
            first = first + (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(first, last);
      }

      edited = true;
      break;
   case kGRSIArrowUp:
      if(gHist != nullptr) {
         TH1* next = gHist->GetNext();
         if(next != nullptr) {
            next->GetXaxis()->SetRange(first, last);
            next->Draw("");
            edited = true;
         }
      } else {
         for(auto* hist : hists) {
            hist->GetYaxis()->SetRangeUser(yMin, yMin + (yMax - yMin) / 2.);
         }
      }
      break;

   case kGRSIArrowDown:
      if(gHist != nullptr) {
         TH1* prev = gHist->GetPrevious();
         if(prev != nullptr) {
            prev->GetXaxis()->SetRange(first, last);
            prev->Draw("");
            edited = true;
         }
      } else {
         for(auto* hist : hists) {
            hist->GetYaxis()->SetRangeUser(yMin, yMin + (yMax - yMin) * 2.);
         }
      }
      break;
   default:
      std::cout << "Move1DHistogram: unknown key = " << key << hex(key) << std::endl;
      break;
   }
   return edited;
}

bool Move2DHistogram(const Int_t& key, TH2* histogram)
{
   /// Moves displayed 2D histograms by 50% of the visible range left, right, up, or down

   bool              edited = false;
   std::vector<TH1*> hists;
   if(histogram != nullptr) {
      hists.push_back(histogram);
   } else {
      hists = FindHists(2);
   }

   int firstX = hists.at(0)->GetXaxis()->GetFirst();
   int lastX  = hists.at(0)->GetXaxis()->GetLast();
   int firstY = hists.at(0)->GetYaxis()->GetFirst();
   int lastY  = hists.at(0)->GetYaxis()->GetLast();

   int minX = std::min(firstX, 0);
   int maxX = std::max(lastX, hists.at(0)->GetXaxis()->GetNbins() + 1);
   int minY = std::min(firstY, 0);
   int maxY = std::max(lastY, hists.at(0)->GetYaxis()->GetNbins() + 1);

   int xdiff  = lastX - firstX;
   int mxdiff = maxX - minX - 2;
   int ydiff  = lastY - firstY;
   int mydiff = maxY - minY - 2;

   switch(key) {
   case kGRSIArrowLeft:
      if(mxdiff > xdiff) {
         if(firstX == (minX + 1)) {
         } else if((firstX - (xdiff / 2)) < minX) {
            firstX = minX + 1;
            lastX  = minX + (xdiff) + 1;
         } else {
            firstX = firstX - (xdiff / 2);
            lastX  = lastX - (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(firstX, lastX);
      }

      edited = true;
      break;
   case kGRSIArrowRight:
      if(mxdiff > xdiff) {
         if(lastX == (maxX - 1)) {
         } else if((lastX + (xdiff / 2)) > maxX) {
            firstX = maxX - 1 - (xdiff);
            lastX  = maxX - 1;
         } else {
            lastX  = lastX + (xdiff / 2);
            firstX = firstX + (xdiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetXaxis()->SetRange(firstX, lastX);
      }

      edited = true;
      break;
   case kGRSIArrowUp:
      if(mydiff > ydiff) {
         if(lastY == (maxY - 1)) {
         } else if((lastY + (ydiff / 2)) > maxY) {
            firstY = maxY - 1 - ydiff;
            lastY  = maxY - 1;
         } else {
            firstY = firstY + (ydiff / 2);
            lastY  = lastY + (ydiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetYaxis()->SetRange(firstY, lastY);
      }

      edited = true;
      break;
   case kGRSIArrowDown:
      if(mydiff > ydiff) {
         if(firstY == (minY + 1)) {
            //
         } else if((firstY - (ydiff / 2)) < minY) {
            firstY = minY + 1;
            lastY  = minY + (ydiff) + 1;
         } else {
            firstY = firstY - (ydiff / 2);
            lastY  = lastY - (ydiff / 2);
         }
      }
      for(auto* hist : hists) {
         hist->GetYaxis()->SetRange(firstY, lastY);
      }

      edited = true;
      break;
   default:
      std::cout << "Move2DHistogram: unknown key = " << key << hex(key) << std::endl;
      break;
   }
   return edited;
}

// bool PeakFit(TH1 *hist,Double_t xlow, Double_t xhigh,Option_t *opt) {
//  if(!hist)
//   return;
//  TString option = opt;
//}

GGaus* GausFit(TH1* hist, double xlow, double xhigh, Option_t* opt)
{
   // bool edit = false;
   if(hist == nullptr) {
      return nullptr;
   }
   if(xlow > xhigh) {
      std::swap(xlow, xhigh);
   }

   // std::cout<<"here."<<std::endl;

   auto*       mypeak  = new GGaus(xlow, xhigh);
   std::string options = opt;
   options.append("Q+");
   mypeak->Fit(hist, options.c_str());
   // mypeak->Background()->Draw("SAME");
   auto* bg = new TF1(*mypeak->Background());
   hist->GetListOfFunctions()->Add(bg);
   // edit = true;

   return mypeak;
}

TF1* DoubleGausFit(TH1* hist, double, double, double xlow, double xhigh, Option_t* opt)
{
   if(hist == nullptr) {
      return nullptr;
   }
   if(xlow > xhigh) {
      std::swap(xlow, xhigh);
   }

   // std::cout<<"here."<<std::endl;

   auto*       mypeak  = new GGaus(xlow, xhigh);
   std::string options = opt;
   options.append("Q+");
   mypeak->Fit(hist, options.c_str());
   // mypeak->Background()->Draw("SAME");
   auto* bg = new TF1(*mypeak->Background());
   hist->GetListOfFunctions()->Add(bg);
   // edit = true;

   return mypeak;
}

GPeak* PhotoPeakFit(TH1* hist, double xlow, double xhigh, Option_t* opt)
{
   return PhotoPeakFit(hist, xlow, (xlow + xhigh) / 2., xhigh, opt);
}

GPeak* PhotoPeakFit(TH1* hist, double xlow, double centroid, double xhigh, Option_t* opt)
{
   if(hist == nullptr) {
      return nullptr;
   }
   if(xlow > xhigh) {
      std::swap(xlow, xhigh);
   }
   if(centroid < xlow || xhigh < centroid) {
      std::cout << "Centroid " << centroid << " out of range " << xlow << " - " << xhigh << std::endl;
      return nullptr;
   }

   auto*       mypeak  = new GPeak(centroid, xlow, xhigh);
   std::string options = opt;
   options.append("+");
   mypeak->Fit(hist, options.c_str());
   auto* bg = new TF1(*mypeak->Background());
   hist->GetListOfFunctions()->Add(bg);

   return mypeak;
}

TPeak* AltPhotoPeakFit(TH1* hist, double xlow, double xhigh, Option_t* opt)
{
   // bool edit = 0;
   if(hist == nullptr) {
      return nullptr;
   }
   if(xlow > xhigh) {
      std::swap(xlow, xhigh);
   }

   // std::cout<<"here."<<std::endl;

   auto* mypeak = new TPeak((xlow + xhigh) / 2.0, xlow, xhigh);
   mypeak->Fit(hist, opt);
   // mypeak->Background()->Draw("SAME");
   auto* bg = new TF1(*mypeak->Background());
   hist->GetListOfFunctions()->Add(bg);
   // edit = true;

   return mypeak;
}

std::string MergeStrings(const std::vector<std::string>& strings, char split)
{
   std::ostringstream ss;
   for(auto it = strings.begin(); it != strings.end(); it++) {
      ss << *it;

      auto next = it;
      next++;
      if(next != strings.end()) {
         ss << split;
      }
   }
   return ss.str();
}

TH1* GrabHist(int i)
{
   // return the histogram from the current canvas, pad i.
   TH1* hist = nullptr;
   if(!gPad) {
      return hist;
   }
   TIter iter(gPad->GetListOfPrimitives());
   int   j = 0;
   while(TObject* obj = iter.Next()) {
      if(obj->InheritsFrom(TH1::Class())) {
         if(j == i) {
            hist = static_cast<TH1*>(obj);
            break;
         }
         j++;
      }
   }
   return hist;
}

TF1* GrabFit(int i)
{
   // return the histogram from the current canvas, pad i.
   TH1* hist = nullptr;
   TF1* fit  = nullptr;
   if(!gPad) {
      return fit;
   }
   TIter iter(gPad->GetListOfPrimitives());
   int   j = 0;
   while(TObject* obj = iter.Next()) {
      if(obj->InheritsFrom(TH1::Class())) {
         hist = static_cast<TH1*>(obj);
         TIter iter2(hist->GetListOfFunctions());
         while(TObject* obj2 = iter2.Next()) {
            if(obj2->InheritsFrom(TF1::Class())) {
               if(j == i) {
                  fit = static_cast<TF1*>(obj2);
                  return fit;
               }
               j++;
            }
         }
      }
   }
   return fit;
}

namespace {
bool gui_is_running = false;
}

#ifdef HAS_CORRECT_PYTHON_VERSION
void StartGUI()
{
   std::string   script_filename = Form("%s/pygui/grut-view.py", getenv("GRSISYS"));
   std::ifstream script(script_filename);
   std::string   script_text((std::istreambuf_iterator<char>(script)), std::istreambuf_iterator<char>());
   TPython::Exec(script_text.c_str());

   auto* gui_timer = new TTimer(R"lit(TPython::Exec("update()");)lit", 10, true);
   gui_timer->TurnOn();

   gui_is_running = true;
   for(int i = 0; i < gROOT->GetListOfFiles()->GetSize(); i++) {
      TPython::Bind(static_cast<TFile*>(gROOT->GetListOfFiles()->At(i)), "tdir");
      gROOT->ProcessLine(R"lit(TPython::Exec("window.AddDirectory(tdir)");)lit");
   }
}
#else
void StartGUI()
{
   std::cout << "Cannot start gui, requires ROOT compiled against python 2.7" << std::endl;
}
#endif

bool GUIIsRunning()
{
   return gui_is_running;
}

#ifdef HAS_CORRECT_PYTHON_VERSION
void AddFileToGUI(TFile* file)
{
   // Pass the TFile to the python GUI.
   if((file != nullptr) && GUIIsRunning()) {
      TPython::Bind(file, "tdir");
      gROOT->ProcessLine(R"lit(TPython::Exec("window.AddDirectory(tdir)");)lit");
   }
}
#else
void AddFileToGUI(TFile*)
{
}
#endif

TH2* AddOffset(TH2* mat, double offset, EAxis axis)
{
   TH2* toreturn = nullptr;
   if(mat == nullptr) {
      return toreturn;
   }
   // int dim = mat->GetDimension();
   int xmax = mat->GetXaxis()->GetNbins() + 1;
   int ymax = mat->GetYaxis()->GetNbins() + 1;
   toreturn = static_cast<TH2*>(mat->Clone(Form("%s_offset", mat->GetName())));
   toreturn->Reset();

   for(int x = 0; x < xmax; x++) {
      for(int y = 0; y < ymax; y++) {
         double newx = mat->GetXaxis()->GetBinCenter(x);
         double newy = mat->GetYaxis()->GetBinCenter(y);
         ;
         double bcont = mat->GetBinContent(x, y);
         if((axis & EAxis::kXAxis) != static_cast<EAxis>(0)) {
            newx += offset;
         }
         if((axis & EAxis::kYAxis) != static_cast<EAxis>(0)) {
            newy += offset;
         }
         toreturn->Fill(newx, newy, bcont);
      }
   }
   return toreturn;
}

EAxis operator&(EAxis lhs, EAxis rhs)
{
   return static_cast<EAxis>(
      static_cast<std::underlying_type<EAxis>::type>(lhs) &
      static_cast<std::underlying_type<EAxis>::type>(rhs));
}
