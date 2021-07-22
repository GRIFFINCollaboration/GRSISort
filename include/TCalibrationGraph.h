#ifndef TCALIBRATIONGRAPH_H
#define TCALIBRATIONGRAPH_H

#include <vector>
#include <string>
#include <iostream>

#include "TObject.h"
#include "TGraphErrors.h"
#include "TFitResultPtr.h"
#include "TList.h"
#include "TLegend.h"

class TCalibrationGraphSet;

class TCalibrationGraph : public TGraphErrors {
public:
	TCalibrationGraph(TCalibrationGraphSet* parent, const int& size, const bool& isResidual = false) : TGraphErrors(size), fParent(parent), fIsResidual(isResidual) {}
	TCalibrationGraph(TCalibrationGraphSet* parent, TGraphErrors* graph) : TGraphErrors(*graph), fParent(parent), fIsResidual(false) {}
	~TCalibrationGraph() {}

	using TGraph::RemovePoint;
	Int_t RemovePoint() override; // *MENU*

	void IsResidual(bool val) { fIsResidual = val; }
	bool IsResidual() { return fIsResidual; }

private:
	TCalibrationGraphSet* fParent{nullptr}; ///< pointer to the set this graph belongs to
	bool fIsResidual{false}; ///< flag to indicate that this graph is for residuals

	ClassDefOverride(TCalibrationGraph, 1)
};

class TCalibrationGraphSet : public TObject {
public:
	TCalibrationGraphSet(TGraphErrors* graph = nullptr, const std::string& label = "");
	~TCalibrationGraphSet();

	bool SetResidual(const bool& force = false);
	void Add(TGraphErrors*, const std::string& label);

	void SetLineColor(int index, int color)   { fGraphs[index]->SetLineColor(color);   fResidualGraphs[index]->SetLineColor(color); }
	void SetMarkerColor(int index, int color) { fGraphs[index]->SetMarkerColor(color); fResidualGraphs[index]->SetMarkerColor(color); std::cout<<"fResidualGraph["<<index<<"] marker color "<<fResidualGraphs[index]->GetMarkerColor()<<" from "<<color<<std::endl; }

	int GetN() { return fTotalGraph->GetN(); }
	double* GetX() { return fTotalGraph->GetX(); }
	double* GetY() { return fTotalGraph->GetY(); }

	void Fit(TF1* function, Option_t* opt) { fTotalGraph->Fit(function, opt); }
	TF1* GetCalibration() { return reinterpret_cast<TF1*>(fTotalGraph->GetListOfFunctions()->FindObject("calibration")); }

	void DrawCalibration(Option_t* opt, TLegend* legend = nullptr);
	void DrawResidual(Option_t* opt, TLegend* legend = nullptr);

	Int_t RemovePoint();
	Int_t RemoveResidualPoint();

	void Print();

	TCalibrationGraphSet& operator=(const TCalibrationGraphSet& rhs)
	{
		fGraphs.resize(rhs.fGraphs.size());
		for(size_t i = 0; i < fGraphs.size(); ++i) {
			fGraphs[i] = static_cast<TCalibrationGraph*>(rhs.fGraphs[i]->Clone());
		}
		fResidualGraphs.resize(rhs.fResidualGraphs.size());
		for(size_t i = 0; i < fResidualGraphs.size(); ++i) {
			fResidualGraphs[i] = static_cast<TCalibrationGraph*>(rhs.fResidualGraphs[i]->Clone());
		}
		fLabel = rhs.fLabel;
		fTotalGraph = static_cast<TGraphErrors*>(rhs.fTotalGraph->Clone());
		fTotalResidualGraph = static_cast<TGraphErrors*>(rhs.fTotalResidualGraph->Clone());
		fGraphIndex = rhs.fGraphIndex;
		fPointIndex = rhs.fPointIndex;
		fResidualSet = rhs.fResidualSet;
		return *this;
	}


private:
	std::vector<TCalibrationGraph*> fGraphs; ///< These are the graphs used for plotting the calibration points per source
	std::vector<TCalibrationGraph*> fResidualGraphs; ///< These are the graphs used for plotting the residuals per source
	std::vector<std::string> fLabel; ///< The labels for the different graphs
	TGraphErrors* fTotalGraph{nullptr}; ///< The sum of the other graphs, used for fitting
	TGraphErrors* fTotalResidualGraph{nullptr}; ///< The sum of the residuals, not used right now?
	std::vector<size_t> fGraphIndex; ///< index of the graph this point belongs to
	std::vector<size_t> fPointIndex; ///< index of the point within the graph this point corresponds to
	bool fResidualSet{false}; ///< Flag to indicate if the residual has been set correctly

	ClassDefOverride(TCalibrationGraphSet, 1)
};
#endif
