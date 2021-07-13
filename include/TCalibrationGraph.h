#ifndef TCALIBRATIONGRAPH_H
#define TCALIBRATIONGRAPH_H

#include "TObject.h"
#include "TGraphErrors.h"
#include "TFitResultPtr.h"
#include "TList.h"

class TCalibrationGraph : public TGraphErrors {
public:
	TCalibrationGraph(int size);
	~TCalibrationGraph();

	void Add(TCalibrationGraph* graph);
	void SetPoint(int index, double x, double y) override;
	void SetPointError(int index, double x, double y) override;
	bool SetResidual(const bool& force = false);
	Int_t RemovePoint() override; // *MENU*

	void SetLineColor(int color) { TGraphErrors::SetLineColor(color); fResidual->SetLineColor(color); }
	void SetMarkerColor(int color) { TGraphErrors::SetMarkerColor(color); fResidual->SetMarkerColor(color); }

	void DrawResidual(Option_t* opt = "") { fResidual->Draw(opt); }

	TFitResultPtr FitResidual(TF1* function, Option_t* opt = "") { return fResidual->Fit(function, opt); }

	TList* GetListOfResidualFunctions() { return fResidual->GetListOfFunctions(); }

	TGraphErrors* Residual() { return fResidual; }

	bool ResidualSet() { return fResidualSet; }

	TCalibrationGraph& operator=(const TCalibrationGraph& rhs)
	{
		fResidual = static_cast<TGraphErrors*>(rhs.fResidual->Clone());
		fResidualSet = rhs.fResidualSet;
		return *this;
	}

private:
	TGraphErrors* fResidual{nullptr};
	bool          fResidualSet{false};

	ClassDefOverride(TCalibrationGraph, 1)
};
#endif
