#include "TCalibrationGraph.h"

#include <iostream>
#include <algorithm>

#include "TMath.h"
#include "TPad.h"
#include "TF1.h"

ClassImp(TCalibrationGraph)

TCalibrationGraph::TCalibrationGraph(int size)
{
	fResidual = new TGraphErrors(size);
}

TCalibrationGraph::~TCalibrationGraph()
{
	delete fResidual;
}

void TCalibrationGraph::Add(TCalibrationGraph* graph)
{
	// get points and error bars from our calibration
	double* x  = GetX();
	double* y  = GetY();
	double* ex = GetEX();
	double* ey = GetEY();

	// get points and error bars from graph's calibration
	double* rhsX  = graph->GetX();
	double* rhsY  = graph->GetY();
	double* rhsEX = graph->GetEX();
	double* rhsEY = graph->GetEY();

	std::vector<std::tuple<double, double, double, double> > data(GetN()+graph->GetN());
	for(int i = 0; i < GetN(); ++i) {
		data[i] = std::make_tuple(x[i], y[i], ex[i], ey[i]);
	}
	for(int i = 0; i < graph->GetN(); ++i) {
		data[GetN()+i] = std::make_tuple(rhsX[i], rhsY[i], rhsEX[i], rhsEY[i]);
	}

	std::sort(data.begin(), data.end());

	for(size_t i = 0; i < data.size(); ++i) {
		SetPoint(i, std::get<0>(data[i]), std::get<1>(data[i]));
		SetPointError(i, std::get<2>(data[i]), std::get<3>(data[i]));
	}
	// doesn't really make sense to calculate the residual here, as we don't have a fit of all the data yet
	fResidualSet = false;
}

void TCalibrationGraph::SetPoint(int index, double x, double y)
{
	TGraph::SetPoint(index, x, y);
	TF1* calibration = static_cast<TF1*>(GetListOfFunctions()->FindObject("calibration"));
	if(calibration == nullptr) {
		fResidualSet = false;
	} else {
		fResidual->SetPoint(index, y - calibration->Eval(x), y);
	}
	// we leave fResidualSet in the state it was
}

void TCalibrationGraph::SetPointError(int index, double ex, double ey)
{
	TGraphErrors::SetPointError(index, ex, ey);
	TF1* calibration = static_cast<TF1*>(GetListOfFunctions()->FindObject("calibration"));
	if(calibration == nullptr) {
		fResidualSet = false;
	} else {
		fResidual->SetPointError(index, TMath::Sqrt(TMath::Power(ey,2) + TMath::Power(ex*calibration->Derivative(GetPointX(index)),2)), ey);
	}
	// we leave fResidualSet in the state it was
}

bool TCalibrationGraph::SetResidual(const bool& force)
{
	TF1* calibration = static_cast<TF1*>(GetListOfFunctions()->FindObject("calibration"));
	if(calibration != nullptr && (!fResidualSet || force)) {
		double* x  = GetX();
		double* y  = GetY();
		double* ex = GetEX();
		double* ey = GetEY();
		fResidual->Set(GetN());
		for(int i = 0; i < GetN(); ++i) {
			fResidual->SetPoint(i, y[i] - calibration->Eval(x[i]), y[i]);
			fResidual->SetPointError(i, TMath::Sqrt(TMath::Power(ey[i],2) + TMath::Power(ex[i]*calibration->Derivative(x[i]),2)), ey[i]);
		}
		fResidualSet = true;
	} else {
		std::cout<<__PRETTY_FUNCTION__<<": didn't find calibration ("<<calibration<<"), or the residual was already set ("<<(fResidualSet?"true":"false")<<") and we don't force it ("<<(force?"true":"false")<<")"<<std::endl; 
		fResidualSet = false;
	}
	return fResidualSet;
}

Int_t TCalibrationGraph::RemovePoint()
{
	/// This function is primarily a copy of TGraph::RemovePoint
   Int_t px = gPad->GetEventX();
   Int_t py = gPad->GetEventY();

   //localize point to be deleted
   Int_t ipoint = -2;
   Int_t i;
   // start with a small window (in case the mouse is very close to one point)
   for (i = 0; i < fNpoints; i++) {
      Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(fX[i]));
      Int_t dpy = py - gPad->YtoAbsPixel(gPad->YtoPad(fY[i]));
      if (dpx * dpx + dpy * dpy < 100) {
         ipoint = i;
         break;
      }
   }
	if(fResidual->RemovePoint(ipoint) < 0) {
		// we failed to remove the point in the residual, so we assume it's out of whack
		fResidualSet = false;
	}
   return TGraph::RemovePoint(ipoint);
}

