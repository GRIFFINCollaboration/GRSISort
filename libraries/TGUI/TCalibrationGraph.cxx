#include "TCalibrationGraph.h"

#include <iostream>
#include <algorithm>

#include "TMath.h"
#include "TPad.h"
#include "TF1.h"
#include "TBuffer.h"
#include "TH1F.h"

Int_t TCalibrationGraph::RemovePoint()
{
   if(TCalibrationGraphSet::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   Int_t px = gPad->GetEventX();
   Int_t py = gPad->GetEventY();
   if(TCalibrationGraphSet::VerboseLevel() > EVerbosity::kBasicFlow) { std::cout << "px, py " << px << ",  " << py << " on gPad " << gPad->GetName() << std::endl; }
   if(fIsResidual) { return fParent->RemoveResidualPoint(px, py); }
   return fParent->RemovePoint(px, py);
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 26, 0)
void TCalibrationGraph::Scale(const double& scale)
{
   Double_t* y  = GetY();
   Double_t* ey = GetEY();

   for(int i = 0; i < GetN(); ++i) {
      y[i] *= scale;
      ey[i] *= scale;
   }
}
#endif

EVerbosity TCalibrationGraphSet::fVerboseLevel = EVerbosity::kQuiet;

TCalibrationGraphSet::TCalibrationGraphSet(TGraphErrors* graph, const std::string& label)
   : fTotalGraph(new TGraphErrors), fTotalResidualGraph(new TGraphErrors)
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " fTotalGraph " << fTotalGraph << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(graph != nullptr) {
      Add(graph, label);
      if(fVerboseLevel > EVerbosity::kBasicFlow) { Print(); }
   }
}

TCalibrationGraphSet::TCalibrationGraphSet(std::string xAxisLabel, std::string yAxisLabel)
   : fTotalGraph(new TGraphErrors), fTotalResidualGraph(new TGraphErrors), fXAxisLabel(std::move(xAxisLabel)), fYAxisLabel(std::move(yAxisLabel))
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " fTotalGraph " << fTotalGraph << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
}

TCalibrationGraphSet::~TCalibrationGraphSet()
{
   delete fTotalGraph;
   delete fTotalResidualGraph;
}

int TCalibrationGraphSet::Add(TGraphErrors* graph, const std::string& label)
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) {
      std::cout << __PRETTY_FUNCTION__ << ", fTotalGraph " << fTotalGraph << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      Print();
   }
   if(graph->GetN() == 0) {
      std::cout << __PRETTY_FUNCTION__ << ": graph \"" << graph->GetName() << "\", label \"" << label << "\" is empty, not adding it" << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return -1;
   }

   graph->GetListOfFunctions()->Clear();   // we don't want to include any fits

   // get points and error bars from our calibration
   double* x  = fTotalGraph->GetX();
   double* y  = fTotalGraph->GetY();
   double* ex = fTotalGraph->GetEX();
   double* ey = fTotalGraph->GetEY();

   // get points and error bars from graph
   double* rhsX  = graph->GetX();
   double* rhsY  = graph->GetY();
   double* rhsEX = graph->GetEX();
   double* rhsEY = graph->GetEY();

   // create one vector with x, y, ex, ey, index of graph, and index of point that we can use to sort the data
   // TODO: create a (private?) enum to reference to x, y, ex, ey, graph index, and point index
   std::vector<std::tuple<double, double, double, double, size_t, size_t>> data(fTotalGraph->GetN() + graph->GetN());
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Filling vector of size " << data.size() << " with " << fTotalGraph->GetN() << " and " << graph->GetN() << " entries" << std::endl; }
   for(int i = 0; i < fTotalGraph->GetN(); ++i) {
      data[i] = std::make_tuple(x[i], y[i], ex[i], ey[i], fGraphIndex[i], fPointIndex[i]);
   }
   for(int i = 0; i < graph->GetN(); ++i) {
      data[fTotalGraph->GetN() + i] = std::make_tuple(rhsX[i], rhsY[i], rhsEX[i], rhsEY[i], fGraphs.size(), i);
   }

   std::sort(data.begin(), data.end());

   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "sorted vector, setting graph sizes" << std::endl; }

   fTotalGraph->Set(static_cast<Int_t>(data.size()));
   fTotalResidualGraph->Set(static_cast<Int_t>(data.size()));
   fGraphIndex.resize(data.size());
   fPointIndex.resize(data.size());

   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Filling fTotalGraph, fGraphIndex, and fPointIndex with " << data.size() << " points" << std::endl; }
   for(size_t i = 0; i < data.size(); ++i) {
      fTotalGraph->SetPoint(static_cast<Int_t>(i), std::get<0>(data[i]), std::get<1>(data[i]));
      fTotalGraph->SetPointError(static_cast<Int_t>(i), std::get<2>(data[i]), std::get<3>(data[i]));
      fGraphIndex[i] = std::get<4>(data[i]);
      fPointIndex[i] = std::get<5>(data[i]);
   }
   fMinimumX = std::get<0>(data[0]);
   fMinimumY = std::get<1>(data[0]);
   fMaximumX = std::get<0>(data.back());
   fMaximumY = std::get<1>(data.back());
   // doesn't really make sense to calculate the residual here, as we don't have a fit of all the data yet
   fResidualSet = false;

   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Adding new calibration graph and label to vectors" << std::endl; }
   // add graph and label to our vectors
   fGraphs.emplace_back(this, graph);
   fResidualGraphs.emplace_back(this, 0, true);
   fLabel.push_back(label);
   // set initial color
   fGraphs.back().SetLineColor(static_cast<Color_t>(fGraphs.size()));
   fGraphs.back().SetMarkerColor(static_cast<Color_t>(fGraphs.size()));
   fGraphs.back().SetMarkerStyle(static_cast<Style_t>(fGraphs.size()));
   fResidualGraphs.back().SetLineColor(static_cast<Color_t>(fResidualGraphs.size()));
   fResidualGraphs.back().SetMarkerColor(static_cast<Color_t>(fResidualGraphs.size()));
   fResidualGraphs.back().SetMarkerStyle(static_cast<Style_t>(fResidualGraphs.size()));

   if(fVerboseLevel > EVerbosity::kBasicFlow) {
      std::cout << "done" << std::endl;
      Print();
   }
   return static_cast<int>(fGraphs.size() - 1);
}

bool TCalibrationGraphSet::SetResidual(const bool& force)
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " gpad " << gPad->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   TF1* calibration = FitFunction();
   if(calibration != nullptr && (!fResidualSet || force)) {
      if(fVerboseLevel > EVerbosity::kBasicFlow) {
         std::cout << "calibration " << calibration << ", fResidualSet " << (fResidualSet ? "true" : "false") << ", force " << (force ? "true" : "false") << ", calibration: " << std::endl;
         calibration->Print();
      }
      double* x  = fTotalGraph->GetX();
      double* y  = fTotalGraph->GetY();
      double* ex = fTotalGraph->GetEX();
      double* ey = fTotalGraph->GetEY();
      fTotalResidualGraph->Set(fTotalGraph->GetN());
      for(int i = 0; i < fTotalGraph->GetN(); ++i) {
         fTotalResidualGraph->SetPoint(i, y[i] - calibration->Eval(x[i]), y[i]);
         fTotalResidualGraph->SetPointError(i, TMath::Sqrt(TMath::Power(ey[i], 2) + TMath::Power(ex[i] * calibration->Derivative(x[i]), 2)), ey[i]);
      }
      if(fVerboseLevel > EVerbosity::kBasicFlow) {
         std::cout << "Done calculating total residual graph with " << fTotalResidualGraph->GetN() << " points" << std::endl;
      }
      for(size_t g = 0; g < fGraphs.size(); ++g) {
         x  = fGraphs[g].GetX();
         y  = fGraphs[g].GetY();
         ex = fGraphs[g].GetEX();
         ey = fGraphs[g].GetEY();
         fResidualGraphs[g].Set(fGraphs[g].GetN());
         for(int i = 0; i < fGraphs[g].GetN(); ++i) {
            fResidualGraphs[g].SetPoint(i, y[i] - calibration->Eval(x[i]), y[i]);
            fResidualGraphs[g].SetPointError(i, TMath::Sqrt(TMath::Power(ey[i], 2) + TMath::Power(ex[i] * calibration->Derivative(x[i]), 2)), ey[i]);
         }
      }
      if(fVerboseLevel > EVerbosity::kBasicFlow) {
         std::cout << "Done calculating all " << fGraphs.size() << " individual residual graphs" << std::endl;
      }
      fResidualSet = true;
      auto* mother = gPad->GetMother();
      int   pad    = 0;
      while(mother->GetPad(pad) != nullptr) {
         if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "pad " << pad << " = " << std::flush << mother->GetPad(pad) << " = " << std::flush << mother->GetPad(pad)->GetName() << ": modifying, " << std::flush; }
         mother->GetPad(pad)->Modified();
         if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "updating, " << std::flush; }
         mother->GetPad(pad)->Update();
         if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "changing to pad, " << std::flush; }
         mother->cd(pad);
         if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "done!" << std::endl; }
         pad++;
      }
   } else {
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << __PRETTY_FUNCTION__ << ": didn't find calibration (" << calibration << "), or the residual was already set (" << (fResidualSet ? "true" : "false") << ") and we don't force it (" << (force ? "true" : "false") << ")" << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      if(calibration == nullptr) { fResidualSet = false; }
   }
   if(fVerboseLevel > EVerbosity::kSubroutines) { Print(); }
   return fResidualSet;
}

void TCalibrationGraphSet::SetAxisTitle(const char* title)
{
   if(fTotalGraph == nullptr) {
      std::cerr << "Trying to set axis title to \"" << title << "\" failed because no total graph is present for this title to be applied to!" << std::endl;
      return;
   }
   fTotalGraph->SetTitle(Form("%s;%s", fTotalGraph->GetTitle(), title));
}

void TCalibrationGraphSet::DrawCalibration(Option_t* opt, TLegend* legend)
{
   TString options = opt;
   options.ToLower();
   if(options.Contains("same")) {
      options.Remove(options.Index("same"), 4);
      opt = options.Data();
   } else {
      options.Append("a");
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " drawing total graph with option \"" << options.Data() << "\" on gPad " << gPad->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fTotalGraph->Draw(options.Data());

   if(fTotalGraph->GetHistogram() != nullptr) {
      fTotalGraph->GetHistogram()->GetXaxis()->CenterTitle();
      fTotalGraph->GetHistogram()->GetXaxis()->SetTitle(fXAxisLabel.c_str());
      fTotalGraph->GetHistogram()->GetYaxis()->CenterTitle();
      fTotalGraph->GetHistogram()->GetYaxis()->SetTitle(fYAxisLabel.c_str());
   }

   for(size_t i = 0; i < fGraphs.size(); ++i) {
      if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " drawing " << i << ". graph with option \"" << opt << "\", marker color " << fGraphs[i].GetMarkerColor() << " on gPad " << gPad->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fGraphs[i].Draw(opt);
      if(legend != nullptr) {
         legend->AddEntry(&(fGraphs[i]), fLabel[i].c_str());
      }
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " drawing legend " << legend << " on gPad " << gPad->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(legend != nullptr) {
      legend->Draw();
   }
}

void TCalibrationGraphSet::DrawResidual(Option_t* opt, TLegend* legend)
{
   TString options = opt;
   options.ToLower();
   options.Append("a");
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " drawing total residual graph with option \"" << options.Data() << "\" on gPad " << gPad->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   fTotalResidualGraph->Draw(options.Data());
   auto* hist = fTotalResidualGraph->GetHistogram();
   if(hist != nullptr) {
      hist->GetXaxis()->SetLabelSize(0.06);
   } else {
      std::cout << "Failed to get histogram for graph:" << std::endl;
      fTotalResidualGraph->Print();
   }

   for(size_t i = 0; i < fResidualGraphs.size(); ++i) {
      if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << " drawing " << i << ". residual graph with option \"" << opt << "\", marker color " << fResidualGraphs[i].GetMarkerColor() << " on gPad " << gPad->GetName() << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      fResidualGraphs[i].Draw(opt);
      if(legend != nullptr) {
         legend->AddEntry(&(fResidualGraphs[i]), fLabel[i].c_str());
      }
   }
   if(legend != nullptr) {
      legend->Draw();
   }
}

Int_t TCalibrationGraphSet::RemovePoint(const Int_t& px, const Int_t& py)
{
   /// This function calls RemovePoint on the total graph.

   if(fVerboseLevel > EVerbosity::kBasicFlow) {
      std::cout << __PRETTY_FUNCTION__ << ": point " << px << ", " << py << "; gPad " << gPad->GetName() << ": " << gPad->AbsPixeltoX(px) << ", " << gPad->AbsPixeltoY(py) << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }

   return RemovePoint(fTotalGraph, px, py);
}

Int_t TCalibrationGraphSet::RemoveResidualPoint(const Int_t& px, const Int_t& py)
{
   /// This function calls RemovePoint on the total graph.

   if(fVerboseLevel > EVerbosity::kBasicFlow) {
      std::cout << __PRETTY_FUNCTION__ << ": point " << px << ", " << py << "; gPad " << gPad->GetName() << ": " << gPad->AbsPixeltoX(px) << ", " << gPad->AbsPixeltoY(py) << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }

   return RemovePoint(fTotalResidualGraph, px, py);
}

Int_t TCalibrationGraphSet::RemovePoint(TGraphErrors* graph, const Int_t& px, const Int_t& py)
{
   /// This function is primarily a copy of TGraph::RemovePoint with some added bits to remove a point that has been selected in the residual graph from it and the corresponding point from the calibration graph and the total graphs

   if(fVerboseLevel > EVerbosity::kBasicFlow) { Print(); }

   // localize point to be deleted
   Int_t point = -2;
   Int_t i     = 0;
   // start with a small window (in case the mouse is very close to one point)
   double* x = graph->GetX();
   double* y = graph->GetY();
   for(i = 0; i < graph->GetN(); i++) {
      Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(x[i]));
      Int_t dpy = py - gPad->YtoAbsPixel(gPad->YtoPad(y[i]));
      // TODO replace 100 with member variable?
      if(dpx * dpx + dpy * dpy < 100) {
         if(fVerboseLevel > EVerbosity::kSubroutines) {
            std::cout << i << ": dpx = " << dpx << " = " << px << " - " << gPad->XtoAbsPixel(gPad->XtoPad(x[i])) << ", dpy = " << dpy << " = " << py << " - " << gPad->YtoAbsPixel(gPad->YtoPad(y[i])) << " this is the point we're looking for at " << x[i] << ", " << y[i] << std::endl;
         }
         point = i;
         break;
      }
      if(fVerboseLevel > EVerbosity::kSubroutines) {
         std::cout << i << ": dpx = " << dpx << " = " << px << " - " << gPad->XtoAbsPixel(gPad->XtoPad(x[i])) << ", dpy = " << dpy << " = " << py << " - " << gPad->YtoAbsPixel(gPad->YtoPad(y[i])) << " not the point we're looking for" << std::endl;
      }
   }

   if(point < 0) {
      if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << "Failed to find point close to " << px << ", " << py << std::endl; }
      if(fVerboseLevel > EVerbosity::kSubroutines) {
         std::cout << __PRETTY_FUNCTION__ << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
         Print();
      }
      return point;
   }
   if(fVerboseLevel > EVerbosity::kSubroutines) {
      Print();
   }

   // now that we have the point we want to delete, remove it from the total graphs
   fTotalGraph->RemovePoint(point);
   if(fTotalResidualGraph->RemovePoint(point) < 0) {
      // we failed to remove the point in the residual, so we assume it's out of whack
      fResidualSet = false;
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << point << " didn't removed residual point" << std::endl; }
   } else if(fVerboseLevel > EVerbosity::kSubroutines) {
      std::cout << point << " removed residual point" << std::endl;
   }

   // need to find which of the graphs we have to remove this point from -> use fGraphIndex[point]
   // and also which point this is of the graph -> use fPointIndex[point]
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << point << " - " << fGraphIndex[point] << ", " << fPointIndex[point] << std::endl; }
   if(fGraphs[fGraphIndex[point]].RemovePoint(fPointIndex[point]) == -1 || fResidualGraphs[fGraphIndex[point]].RemovePoint(fPointIndex[point]) == -1) {
      std::cout << "point " << point << " out of range?" << std::endl;
   }

   // now we need to update the indices by removing this one, and updating all that come after it
   auto oldGraph = fGraphIndex[point];
   auto oldPoint = fPointIndex[point];
   fGraphIndex.erase(fGraphIndex.begin() + point);
   fPointIndex.erase(fPointIndex.begin() + point);
   for(size_t p = 0; p < fGraphIndex.size(); ++p) {
      if(fGraphIndex[p] == oldGraph && fPointIndex[p] >= oldPoint) {
         if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << fGraphIndex[p] << " == " << oldGraph << " && " << fPointIndex[p] << " >= " << oldPoint << ": decrementing index" << std::endl; }
         --fPointIndex[p];
      } else if(fVerboseLevel > EVerbosity::kSubroutines) {
         std::cout << fGraphIndex[p] << " != " << oldGraph << " || " << fPointIndex[p] << " < " << oldPoint << ": decrementing index" << std::endl;
      }
   }

   // update the graphics
   auto* mother = gPad->GetMother();
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Got mother pad " << mother->GetName() << " from pad " << gPad->GetName() << std::endl; }
   int pad = 0;
   while(mother->GetPad(pad) != nullptr) {
      mother->GetPad(pad)->Modified();   // one version also used Update and cd(pad)?
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Modified pad " << pad << " = " << mother->GetPad(pad)->GetName() << std::endl; }
      pad++;
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { Print(); }

   // emit signal that this point has been removed
   std::array<Longptr_t, 2> args = {static_cast<int64_t>(oldGraph), static_cast<int64_t>(oldPoint)};
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Emitting RemovePoint(Int_t, Int_t) with " << args[0] << ", " << args[1] << " - " << args.data() << std::endl; }
   Emit("RemovePoint(Int_t, Int_t)", args.data());

   return point;
}

void TCalibrationGraphSet::RemovePoint(const int& point)
{
   /// This function removes the indicated point from the total graph.

   if(fVerboseLevel > EVerbosity::kBasicFlow) { Print(); }

   // now that we have the point we want to delete, remove it from the total graphs
   fTotalGraph->RemovePoint(point);
   if(fTotalResidualGraph->RemovePoint(point) < 0) {
      // we failed to remove the point in the residual, so we assume it's out of whack
      fResidualSet = false;
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << point << " didn't removed residual point" << std::endl; }
   } else if(fVerboseLevel > EVerbosity::kSubroutines) {
      std::cout << point << " removed residual point" << std::endl;
   }

   // need to find which of the graphs we have to remove this point from -> use fGraphIndex[point]
   // and also which point this is of the graph -> use fPointIndex[point]
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Going to remove " << point << " - " << fGraphIndex[point] << ", " << fPointIndex[point] << std::endl; }
   if(fGraphs[fGraphIndex[point]].RemovePoint(fPointIndex[point]) == -1 || fResidualGraphs[fGraphIndex[point]].RemovePoint(fPointIndex[point]) == -1) {
      std::cout << "point " << point << " out of range?" << std::endl;
   }

   // now we need to update the indices by removing this one, and updating all that come after it
   auto oldGraph = fGraphIndex[point];
   auto oldPoint = fPointIndex[point];
   fGraphIndex.erase(fGraphIndex.begin() + point);
   fPointIndex.erase(fPointIndex.begin() + point);
   for(size_t p = 0; p < fGraphIndex.size(); ++p) {
      if(fGraphIndex[p] == oldGraph && fPointIndex[p] >= oldPoint) {
         if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << fGraphIndex[p] << " == " << oldGraph << " && " << fPointIndex[p] << " >= " << oldPoint << ": decrementing index" << std::endl; }
         --fPointIndex[p];
      } else if(fVerboseLevel > EVerbosity::kSubroutines) {
         std::cout << fGraphIndex[p] << " != " << oldGraph << " || " << fPointIndex[p] << " < " << oldPoint << ": decrementing index" << std::endl;
      }
   }

   // update the graphics
   auto* mother = gPad->GetMother();
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Got mother pad " << mother->GetName() << " from pad " << gPad->GetName() << std::endl; }
   int pad = 0;
   while(mother->GetPad(pad) != nullptr) {
      mother->GetPad(pad)->Modified();   // one version also used Update and cd(pad)?
      if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Modified pad " << pad << " = " << mother->GetPad(pad)->GetName() << std::endl; }
      pad++;
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { Print(); }

   // emit signal that this point has been removed
   std::array<Longptr_t, 2> args = {static_cast<int64_t>(oldGraph), static_cast<int64_t>(oldPoint)};
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Emitting RemovePoint(Int_t, Int_t) with " << args[0] << ", " << args[1] << " - " << args.data() << std::endl; }
   Emit("RemovePoint(Int_t, Int_t)", args.data());
}

void TCalibrationGraphSet::Sort()
{
   for(auto& graph : fGraphs) {
      graph.Sort();
   }
   for(auto& graph : fResidualGraphs) {
      // residuals plot energy vs. residual, so we want to sort by y, not x!
      graph.Sort(&TGraph::CompareY);
   }
}

void TCalibrationGraphSet::Scale(bool useAllPrevious)
{
   /// Scale all graphs to fit each other (based on the first "previous" graph found or just the first graph).
   /// If no overlap is being found between the graph that is being scaled and the first graph (or all graphs before this one),
   /// the current graph isn't being scaled and we continue with the next graph.
   if(fVerboseLevel > EVerbosity::kBasicFlow) {
      std::cout << __PRETTY_FUNCTION__ << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      Print();
   }
   Sort();
   for(size_t g = 1; g < fGraphs.size(); ++g) {
      auto& graph = fGraphs[g];
      if(graph.GetN() == 0) {
         std::cout << "No entries in " << g << ". graph" << std::endl;
      }
      double* x = graph.GetX();
      double* y = graph.GetY();
      // loop over all other graphs before this one and use the first one with an overlap (should this be extended to use all possible ones before this one?)
      size_t g2 = 0;
      for(g2 = 0; (useAllPrevious ? g2 < g : g2 < 1); ++g2) {
         double minRef = fGraphs[g2].GetX()[0];
         double maxRef = fGraphs[g2].GetX()[fGraphs[g2].GetN() - 1];
         if(fVerboseLevel > EVerbosity::kBasicFlow) {
            std::cout << "Checking overlap between " << g2 << ". graph (" << fGraphs[g2].GetN() << ": " << minRef << " - " << maxRef << ") and " << g << ". graph (" << graph.GetN() << std::flush << ": " << x[0] << " - " << x[graph.GetN() - 1] << ")" << std::endl;
         }
         if(maxRef < x[0] || x[graph.GetN() - 1] < minRef) {
            // no overlap between the two graphs, for now we just skip this one, but we could try and compare it to all the other ones?
            std::cout << "No overlap between " << g2 << ". graph (" << fGraphs[g2].GetN() << ": " << minRef << " - " << maxRef << ") and " << g << ". graph (" << graph.GetN() << ": " << x[0] << " - " << x[graph.GetN() - 1] << ")" << std::endl;
            continue;
         }
         // we have an overlap, so we calculate the scaling factor for each point and take the average (maybe should add some weight from the errors bars)
         int    count = 0;
         double sum   = 0.;
         for(int p = 0; p < graph.GetN(); ++p) {
            if(minRef < x[p] && x[p] < maxRef) {
               sum += fGraphs[g2].Eval(x[p]) / y[p];
               ++count;
               if(fVerboseLevel > EVerbosity::kLoops) { std::cout << g << ", " << p << ": " << count << " - " << sum << ", " << fGraphs[g2].Eval(x[p]) / y[p] << std::endl; }
            }
         }
         sum /= count;
         if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << g << ": scaling with " << sum << std::endl; }
         graph.Scale(sum);
         break;
      }
      if(g2 == g && fVerboseLevel > EVerbosity::kQuiet) {
         std::cout << "No overlap(s) between 0. to " << g2 - 1 << ". graph and " << g << ". graph (" << graph.GetN() << ": " << x[0] << " - " << x[graph.GetN() - 1] << "), not scaling this one!" << std::endl;
      }
   }
   if(fVerboseLevel > EVerbosity::kBasicFlow) { Print(); }
   ResetTotalGraph();
}

void TCalibrationGraphSet::ResetTotalGraph()
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) { std::cout << __PRETTY_FUNCTION__ << std::endl; }   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   if(fGraphs.empty()) {
      std::cerr << "No graphs added yet, makes no sense to reset total graph?" << std::endl;
      return;
   }

   size_t newSize = 0;
   for(auto& graph : fGraphs) {
      newSize += graph.GetN();
   }
   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "creating graph with " << newSize << " points" << std::endl; }
   // create one vector with x, y, ex, ey, index of graph, and index of point that we can use to sort the data
   std::vector<std::tuple<double, double, double, double, size_t, size_t>> data(newSize);
   size_t                                                                  counter = 0;
   for(size_t i = 0; i < fGraphs.size(); ++i) {
      // get points and error bars from graph
      double* x  = fGraphs[i].GetX();
      double* y  = fGraphs[i].GetY();
      double* eX = fGraphs[i].GetEX();
      double* eY = fGraphs[i].GetEY();
      for(int p = 0; p < fGraphs[i].GetN(); ++p, ++counter) {
         if(fVerboseLevel > EVerbosity::kLoops) { std::cout << "filling point " << counter << " of vector of size " << newSize << std::endl; }
         data[counter] = std::make_tuple(x[p], y[p], eX[p], eY[p], i, p);
      }
   }

   std::sort(data.begin(), data.end());

   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "sorted vector, setting graph sizes" << std::endl; }

   fTotalGraph->Set(data.size());
   fTotalResidualGraph->Set(data.size());
   fGraphIndex.resize(data.size());
   fPointIndex.resize(data.size());

   if(fVerboseLevel > EVerbosity::kSubroutines) { std::cout << "Filling fTotalGraph, fGraphIndex, and fPointIndex with " << data.size() << " points" << std::endl; }
   for(size_t i = 0; i < data.size(); ++i) {
      fTotalGraph->SetPoint(i, std::get<0>(data[i]), std::get<1>(data[i]));
      fTotalGraph->SetPointError(i, std::get<2>(data[i]), std::get<3>(data[i]));
      fGraphIndex[i] = std::get<4>(data[i]);
      fPointIndex[i] = std::get<5>(data[i]);
   }
   // doesn't really make sense to calculate the residual here, as we don't have a fit of all the data yet
   fResidualSet = false;
   // set the new minima and maxima (always assuming the first and last points are minima and maxima, respectively)
   fMinimumX = std::get<0>(data[0]);
   fMinimumY = std::get<1>(data[0]);
   fMaximumX = std::get<0>(data.back());
   fMaximumY = std::get<1>(data.back());
}

void TCalibrationGraphSet::Print(Option_t* opt) const
{
   if(fVerboseLevel > EVerbosity::kBasicFlow) {
      std::cout << __PRETTY_FUNCTION__ << ", fTotalGraph " << fTotalGraph << std::endl;   // NOLINT(cppcoreguidelines-pro-type-const-cast, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
   }

   std::cout << "TCalibrationGraphSet " << this << " - " << GetName() << ": " << fGraphs.size() << " calibration graphs, " << fResidualGraphs.size() << " residual graphs, " << fLabel.size() << " labels, ";
   if(fTotalGraph != nullptr) {
      std::cout << fTotalGraph->GetN() << " calibration points, and ";
   } else {
      std::cout << " no calibration points, and ";
   }
   if(fTotalResidualGraph != nullptr) {
      std::cout << fTotalResidualGraph->GetN() << " residual points" << std::endl;
   } else {
      std::cout << " no residual points" << std::endl;
   }
   TString options = opt;
   bool    errors  = options.Contains("e", TString::ECaseCompare::kIgnoreCase);
   for(const auto& g : fGraphs) {
      double* x  = g.GetX();
      double* y  = g.GetY();
      double* ex = g.GetEX();
      double* ey = g.GetEY();
      for(int p = 0; p < g.GetN(); ++p) {
         if(errors) {
            std::cout << p << " - " << x[p] << "(" << ex[p] << "), " << y[p] << "(" << ey[p] << "); ";
         } else {
            std::cout << p << " - " << x[p] << ", " << y[p] << "; ";
         }
      }
      std::cout << std::endl;
   }
   std::cout << fGraphIndex.size() << " graph indices: ";
   for(const auto& i : fGraphIndex) { std::cout << std::setw(3) << i << " "; }
   std::cout << std::endl;
   std::cout << fPointIndex.size() << " point indices: ";
   for(const auto& i : fPointIndex) { std::cout << std::setw(3) << i << " "; }
   std::cout << std::endl;
   std::cout << "---- total graph ----" << std::endl;
   double* x = fTotalGraph->GetX();
   double* y = fTotalGraph->GetY();
   for(int p = 0; p < fTotalGraph->GetN(); ++p) {
      std::cout << p << " - " << x[p] << ", " << y[p] << "; ";
   }
   std::cout << std::endl;
   std::cout << "---------------------" << std::endl;
}

void TCalibrationGraphSet::Clear(Option_t* option)
{
   fGraphs.clear();
   fResidualGraphs.clear();
   fLabel.clear();
   fGraphIndex.clear();
   fPointIndex.clear();
   fResidualSet = false;
   fMinimumX    = 0.;
   fMaximumX    = 0.;
   fMinimumY    = 0.;
   fMaximumY    = 0.;
   TNamed::Clear(option);
   fTotalGraph->Set(0);
   fTotalResidualGraph->Set(0);
}
