#ifndef TCALIBRATIONGRAPH_H
#define TCALIBRATIONGRAPH_H

#include <vector>
#include <string>
#include <iostream>

#include "TNamed.h"
#include "TGraphErrors.h"
#include "TFitResultPtr.h"
#include "TList.h"
#include "TLegend.h"

class TCalibrationGraphSet;

class TCalibrationGraph : public TGraphErrors {
public:
   TCalibrationGraph() = default;
   TCalibrationGraph(TCalibrationGraphSet* parent, const int& size, const bool& isResidual = false) : TGraphErrors(size), fParent(parent), fIsResidual(isResidual) {}
   TCalibrationGraph(TCalibrationGraphSet* parent, TGraphErrors* graph) : TGraphErrors(*graph), fParent(parent) {}
   ~TCalibrationGraph()                                       = default;
   TCalibrationGraph(const TCalibrationGraph&)                = default;
   TCalibrationGraph(TCalibrationGraph&&) noexcept            = default;
   TCalibrationGraph& operator=(const TCalibrationGraph&)     = default;
   TCalibrationGraph& operator=(TCalibrationGraph&&) noexcept = default;

   using TGraph::RemovePoint;      // to use the function with integer index as argument
   Int_t RemovePoint() override;   // *MENU*

   void IsResidual(bool val) { fIsResidual = val; }
   bool IsResidual() const { return fIsResidual; }

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 26, 0)
   void Scale(const double& scale);
#endif

private:
   TCalibrationGraphSet* fParent{nullptr};     ///< pointer to the set this graph belongs to
   bool                  fIsResidual{false};   ///< flag to indicate that this graph is for residuals

   /// \cond CLASSIMP
   ClassDefOverride(TCalibrationGraph, 1)   // NOLINT(readability-else-after-return)
   /// \endcond
};

class TCalibrationGraphSet : public TNamed {
public:
   explicit TCalibrationGraphSet(TGraphErrors* graph = nullptr, const std::string& label = "");
   TCalibrationGraphSet(const std::string& xAxisLabel, const std::string& yAxisLabel);
   ~TCalibrationGraphSet();
   TCalibrationGraphSet(const TCalibrationGraphSet&)     = default;
   TCalibrationGraphSet(TCalibrationGraphSet&&) noexcept = default;
   TCalibrationGraphSet& operator=(const TCalibrationGraphSet& rhs)
   {
      /// Assignment operator that takes care of properly cloning all the pointers to objects.
      fGraphs.resize(rhs.fGraphs.size());
      for(size_t i = 0; i < fGraphs.size(); ++i) {
         fGraphs[i] = rhs.fGraphs[i];
      }
      fResidualGraphs.resize(rhs.fResidualGraphs.size());
      for(size_t i = 0; i < fResidualGraphs.size(); ++i) {
         fResidualGraphs[i] = rhs.fResidualGraphs[i];
      }
      fLabel              = rhs.fLabel;
      fTotalGraph         = static_cast<TGraphErrors*>(rhs.fTotalGraph->Clone());
      fTotalResidualGraph = static_cast<TGraphErrors*>(rhs.fTotalResidualGraph->Clone());
      fGraphIndex         = rhs.fGraphIndex;
      fPointIndex         = rhs.fPointIndex;
      fResidualSet        = rhs.fResidualSet;
      fName               = rhs.fName;
      return *this;
   }
   TCalibrationGraphSet& operator=(TCalibrationGraphSet&&) noexcept = default;

   bool SetResidual(const bool& force = false);
   int  Add(TGraphErrors*, const std::string& label);   ///< Add new graph to set, using the label when creating legends during plotting

   void SetLineColor(int index, Color_t color)
   {
      /// Set the line color of the graph and residuals at index
      fGraphs[index].SetLineColor(color);
      fResidualGraphs[index].SetLineColor(color);
   }
   void SetMarkerColor(int index, Color_t color)
   {
      /// Set the marker color of the graph and residuals at index
      if(fVerboseLevel > 3) { std::cout << "setting marker color of graph " << index << " to " << color << std::endl; }
      fGraphs[index].SetMarkerColor(color);
      fResidualGraphs[index].SetMarkerColor(color);
   }
   void SetMarkerStyle(int index, Style_t style)
   {
      /// Set the marker style of the graph and residuals at index
      fGraphs[index].SetMarkerStyle(style);
      fResidualGraphs[index].SetMarkerStyle(style);
   }
   void SetColor(int index, Color_t color)
   {
      /// Set the line and marker color of the graph and residuals at index
      SetLineColor(index, color);
      SetMarkerColor(index, color);
   }
   void SetColorStyle(int index, int val)
   {
      /// Set the line and marker color and marker style of the graph and residuals at index
      SetLineColor(index, static_cast<Color_t>(val));
      SetMarkerColor(index, static_cast<Color_t>(val));
      SetMarkerStyle(index, static_cast<Style_t>(val));
   }

   void SetAxisTitle(const char* title);   ///< Set axis title for the graph (form "x-axis title;y-axis title")

   int     GetN() { return (fTotalGraph != nullptr ? fTotalGraph->GetN() : -1); }          ///< Returns GetN(), i.e. number of points of the total graph.
   double* GetX() { return (fTotalGraph != nullptr ? fTotalGraph->GetX() : nullptr); }     ///< Returns an array of x-values of the total graph.
   double* GetY() { return (fTotalGraph != nullptr ? fTotalGraph->GetY() : nullptr); }     ///< Returns an array of y-values of the total graph.
   double* GetEX() { return (fTotalGraph != nullptr ? fTotalGraph->GetEX() : nullptr); }   ///< Returns an array of x-errors of the total graph.
   double* GetEY() { return (fTotalGraph != nullptr ? fTotalGraph->GetEY() : nullptr); }   ///< Returns an array of y-errors of the total graph.

   double GetMinimumX() const { return fMinimumX; }   ///< Return minimum x-value.
   double GetMaximumX() const { return fMaximumX; }   ///< Return maximum x-value.
   double GetMinimumY() const { return fMinimumY; }   ///< Return minimum y-value.
   double GetMaximumY() const { return fMaximumY; }   ///< Return maximum y-value.

   void               Fit(TF1* function, Option_t* opt = "") { fTotalGraph->Fit(function, opt); }                                      ///< Fits the provided function to the total graph.
   TF1*               FitFunction() { return reinterpret_cast<TF1*>(fTotalGraph->GetListOfFunctions()->FindObject("fitfunction")); }   ///< Gets the calibration from the total graph (might be nullptr!).
   TGraphErrors*      TotalGraph() { return fTotalGraph; }
   size_t             NumberOfGraphs() { return fGraphs.size(); }
   TCalibrationGraph* Graph(size_t index) { return &(fGraphs.at(index)); }
   TCalibrationGraph* Residual(size_t index) { return &(fResidualGraphs.at(index)); }

   void DrawCalibration(Option_t* opt = "", TLegend* legend = nullptr);
   void DrawResidual(Option_t* opt = "", TLegend* legend = nullptr);

   void RemoveGraph(int64_t index)
   {
      fGraphs.erase(fGraphs.begin() + index);
      ResetTotalGraph();
   }
   Int_t RemovePoint();
   Int_t RemoveResidualPoint();

	void XAxisLabel(const std::string& xAxisLabel) { fXAxisLabel = xAxisLabel; }
	void YAxisLabel(const std::string& yAxisLabel) { fYAxisLabel = yAxisLabel; }

	std::string XAxisLabel() { return fXAxisLabel; }
	std::string YAxisLabel() { return fYAxisLabel; }

   void Scale(bool useAllPrevious = true);

   void Print(Option_t* opt = "") const override;

   void ResetTotalGraph();   ///< reset the total graph and add the individual ones again (used e.g. after scaling of individual graphs is done)

   static void VerboseLevel(int val) { fVerboseLevel = val; }
   static int  VerboseLevel() { return fVerboseLevel; }

	void Clear(Option_t* option = "") override;

private:
   std::vector<TCalibrationGraph> fGraphs;                        ///< These are the graphs used for plotting the calibration points per source.
   std::vector<TCalibrationGraph> fResidualGraphs;                ///< These are the graphs used for plotting the residuals per source.
   std::vector<std::string>       fLabel;                         ///< The labels for the different graphs.
   TGraphErrors*                  fTotalGraph{nullptr};           ///< The sum of the other graphs, used for fitting.
   TGraphErrors*                  fTotalResidualGraph{nullptr};   ///< The sum of the residuals. Not really used apart from plotting (but overlayed with the individual graphs).
   std::vector<size_t>            fGraphIndex;                    ///< Index of the graph this point belongs to.
   std::vector<size_t>            fPointIndex;                    ///< Index of the point within the graph this point corresponds to.
   bool                           fResidualSet{false};            ///< Flag to indicate if the residual has been set correctly.
   double                         fMinimumX{0.};                  ///< Minimum x-value
   double                         fMaximumX{0.};                  ///< Maximum x-value
   double                         fMinimumY{0.};                  ///< Minimum y-value
   double                         fMaximumY{0.};                  ///< Maximum y-value
   std::string                    fXAxisLabel;                    ///< The label of the x-axis.
   std::string                    fYAxisLabel;                    ///< The label of the y-axis.

   static int fVerboseLevel;   ///< Changes verbosity from 0 (quiet) to 4 (very verbose)

   /// \cond CLASSIMP
   ClassDefOverride(TCalibrationGraphSet, 3)   // NOLINT(readability-else-after-return)
   /// \endcond
};
#endif
