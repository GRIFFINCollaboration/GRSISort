#include "TCalibrationGraph.h"

#include <iostream>
#include <algorithm>

#include "TMath.h"
#include "TPad.h"
#include "TF1.h"

ClassImp(TCalibrationGraph)
ClassImp(TCalibrationGraphSet)

Int_t TCalibrationGraph::RemovePoint()
{
	std::cout<<__PRETTY_FUNCTION__<<std::endl;
	if(fIsResidual) return fParent->RemoveResidualPoint();
	else            return fParent->RemovePoint();
}

TCalibrationGraphSet::TCalibrationGraphSet(TGraphErrors* graph, const std::string& label)
{
	fTotalGraph = new TGraphErrors;
	fTotalResidualGraph = new TGraphErrors;
	if(graph != nullptr) {
		Add(graph, label);
		Print();
	}
}

TCalibrationGraphSet::~TCalibrationGraphSet()
{
}

void TCalibrationGraphSet::Add(TGraphErrors* graph, const std::string& label)
{
	std::cout<<__PRETTY_FUNCTION__<<std::endl;
	Print();
	graph->GetListOfFunctions()->Clear(); // we don't want to include any fits

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
	std::vector<std::tuple<double, double, double, double, size_t, size_t> > data(fTotalGraph->GetN()+graph->GetN());
	std::cout<<"Filling vector of size "<<data.size()<<" with "<<fTotalGraph->GetN()<<" and "<<graph->GetN()<<" entries"<<std::endl;
	for(int i = 0; i < fTotalGraph->GetN(); ++i) {
		data[i] = std::make_tuple(x[i], y[i], ex[i], ey[i], fGraphIndex[i], fPointIndex[i]);
	}
	for(int i = 0; i < graph->GetN(); ++i) {
		data[fTotalGraph->GetN()+i] = std::make_tuple(rhsX[i], rhsY[i], rhsEX[i], rhsEY[i], fGraphs.size(), i);
	}

	std::sort(data.begin(), data.end());

	std::cout<<"sorted vector, setting graph sizes"<<std::endl;

	fTotalGraph->Set(data.size());
	fTotalResidualGraph->Set(data.size());
	fGraphIndex.resize(data.size());
	fPointIndex.resize(data.size());

	std::cout<<"Filling fTotalGraph, fGraphIndex, and fPointIndex with "<<data.size()<<" points"<<std::endl;
	for(size_t i = 0; i < data.size(); ++i) {
		fTotalGraph->SetPoint(i, std::get<0>(data[i]), std::get<1>(data[i]));
		fTotalGraph->SetPointError(i, std::get<2>(data[i]), std::get<3>(data[i]));
		fGraphIndex[i] = std::get<4>(data[i]);
		fPointIndex[i] = std::get<5>(data[i]);
	}
	// doesn't really make sense to calculate the residual here, as we don't have a fit of all the data yet
	fResidualSet = false;

	std::cout<<"Adding new calibration graph and label to vectors"<<std::endl;
	// add graph and label to our vectors
	TCalibrationGraph* calibrationGraph = new TCalibrationGraph(this, graph);
	fGraphs.push_back(calibrationGraph);
	fResidualGraphs.push_back(new TCalibrationGraph(this, 0, true));
	fLabel.push_back(label);
	std::cout<<"done"<<std::endl;
	Print();
}

bool TCalibrationGraphSet::SetResidual(const bool& force)
{
	std::cout<<__PRETTY_FUNCTION__<<std::endl;
	TF1* calibration = GetFitFunction();
	if(calibration != nullptr && (!fResidualSet || force)) {
		double* x  = fTotalGraph->GetX();
		double* y  = fTotalGraph->GetY();
		double* ex = fTotalGraph->GetEX();
		double* ey = fTotalGraph->GetEY();
		fTotalResidualGraph->Set(fTotalGraph->GetN());
		for(int i = 0; i < fTotalGraph->GetN(); ++i) {
			fTotalResidualGraph->SetPoint(i, y[i] - calibration->Eval(x[i]), y[i]);
			fTotalResidualGraph->SetPointError(i, TMath::Sqrt(TMath::Power(ey[i],2) + TMath::Power(ex[i]*calibration->Derivative(x[i]),2)), ey[i]);
		}
		for(size_t g = 0; g < fGraphs.size(); ++g) {
			x  = fGraphs[g]->GetX();
			y  = fGraphs[g]->GetY();
			ex = fGraphs[g]->GetEX();
			ey = fGraphs[g]->GetEY();
			fResidualGraphs[g]->Set(fGraphs[g]->GetN());
			for(int i = 0; i < fGraphs[g]->GetN(); ++i) {
				fResidualGraphs[g]->SetPoint(i, y[i] - calibration->Eval(x[i]), y[i]);
				fResidualGraphs[g]->SetPointError(i, TMath::Sqrt(TMath::Power(ey[i],2) + TMath::Power(ex[i]*calibration->Derivative(x[i]),2)), ey[i]);
			}
		}
		fResidualSet = true;
		auto mother = gPad->GetMother();
		int pad = 0;
		while(mother->GetPad(pad) != nullptr) {
			mother->GetPad(pad)->Modified();
			std::cout<<"Modified pad "<<pad<<" = "<<mother->GetPad(pad)->GetName()<<std::endl;
			pad++;
		}
	} else {
		std::cout<<__PRETTY_FUNCTION__<<": didn't find calibration ("<<calibration<<"), or the residual was already set ("<<(fResidualSet?"true":"false")<<") and we don't force it ("<<(force?"true":"false")<<")"<<std::endl; 
		if(calibration == nullptr) fResidualSet = false;
	}
	Print();
	return fResidualSet;
}

void TCalibrationGraphSet::DrawCalibration(Option_t* opt, TLegend* legend)
{
	TString options = opt;
	options.Append("a");
	fTotalGraph->Draw(options.Data());

	for(size_t i = 0; i < fGraphs.size(); ++i) {
		std::cout<<__PRETTY_FUNCTION__<<" drawing "<<i<<". graph with option \""<<opt<<"\""<<std::endl;
		fGraphs[i]->Draw(opt);
		if(legend != nullptr) {
			legend->AddEntry(fGraphs[i], fLabel[i].c_str());
		}
	}
}

void TCalibrationGraphSet::DrawResidual(Option_t* opt, TLegend* legend)
{
	TString options = opt;
	options.Append("a");
	fTotalResidualGraph->Draw(options.Data());

	for(size_t i = 0; i < fResidualGraphs.size(); ++i) {
		std::cout<<__PRETTY_FUNCTION__<<" drawing "<<i<<". residual graph with option \""<<opt<<"\", marker color "<<fResidualGraphs[i]->GetMarkerColor()<<std::endl;
		fResidualGraphs[i]->Draw(opt);
		if(legend != nullptr) {
			legend->AddEntry(fResidualGraphs[i], fLabel[i].c_str());
		}
	}
}

Int_t TCalibrationGraphSet::RemovePoint()
{
	/// This function is primarily a copy of TGraph::RemovePoint with some added bits to remove a point that has been selected in the calibration graph from it and the corresponding point from the residual graph and the total graphs
	Int_t px = gPad->GetEventX();
	Int_t py = gPad->GetEventY();

	//localize point to be deleted
	Int_t ipoint = -2;
	Int_t i;
	// start with a small window (in case the mouse is very close to one point)
	double* x = fTotalGraph->GetX();
	double* y = fTotalGraph->GetY();
	for (i = 0; i < fTotalGraph->GetN(); i++) {
		Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(x[i]));
		Int_t dpy = py - gPad->YtoAbsPixel(gPad->YtoPad(y[i]));
		if (dpx * dpx + dpy * dpy < 100) {
			ipoint = i;
			break;
		}
	}
	if(ipoint < 0) {
		std::cout<<"Failed to find point close to "<<px<<", "<<py<<std::endl;
		return ipoint;
	}
	std::cout<<__PRETTY_FUNCTION__<<std::endl;
	Print();
	std::cout<<ipoint<<": "<<fTotalGraph->RemovePoint(ipoint)<<std::endl;
	if(fTotalResidualGraph->RemovePoint(ipoint) < 0) {
		// we failed to remove the point in the residual, so we assume it's out of whack
		fResidualSet = false;
		std::cout<<ipoint<<" didn't removed residual point"<<std::endl;
	} else {
		std::cout<<ipoint<<" removed residual point"<<std::endl;
	}
	// need to find which of the graphs we have to remove this point from -> use fGraphIndex[ipoint]
	// and also which point this is of the graph -> use fPointIndex[ipoint]
	std::cout<<ipoint<<" - "<<fGraphIndex[ipoint]<<", "<<fPointIndex[ipoint]<<": "<<fGraphs[fGraphIndex[ipoint]]->RemovePoint(fPointIndex[ipoint])<<std::endl;
	std::cout<<ipoint<<" - "<<fGraphIndex[ipoint]<<", "<<fPointIndex[ipoint]<<": "<<fResidualGraphs[fGraphIndex[ipoint]]->RemovePoint(fPointIndex[ipoint])<<std::endl;
	// and now also remove the points from the indices - also need to update all points after this point!
	auto oldGraph = fGraphIndex[ipoint];
	auto oldPoint = fPointIndex[ipoint];
	fGraphIndex.erase(fGraphIndex.begin()+ipoint);
	fPointIndex.erase(fPointIndex.begin()+ipoint);
	for(size_t p = 0; p < fGraphIndex.size(); ++p) {
		if(fGraphIndex[p] == oldGraph && fPointIndex[p] > oldPoint) {
			--fPointIndex[p];
		}
	}
	auto mother = gPad->GetMother();
	int pad = 0;
	while(mother->GetPad(pad) != nullptr) {
		mother->GetPad(pad)->Modified();
		std::cout<<"Modified pad "<<pad<<" = "<<mother->GetPad(pad)->GetName()<<std::endl;
		pad++;
	}
	Print();
	return ipoint;
}

Int_t TCalibrationGraphSet::RemoveResidualPoint()
{
	/// This function is primarily a copy of TGraph::RemovePoint with some added bits to remove a point that has been selected in the residual graph from it and the corresponding point from the calibration graph and the total graphs
	Int_t px = gPad->GetEventX();
	Int_t py = gPad->GetEventY();

	//localize point to be deleted
	Int_t ipoint = -2;
	Int_t i;
	// start with a small window (in case the mouse is very close to one point)
	double* x = fTotalResidualGraph->GetX();
	double* y = fTotalResidualGraph->GetY();
	for (i = 0; i < fTotalResidualGraph->GetN(); i++) {
		Int_t dpx = px - gPad->XtoAbsPixel(gPad->XtoPad(x[i]));
		Int_t dpy = py - gPad->YtoAbsPixel(gPad->YtoPad(y[i]));
		if (dpx * dpx + dpy * dpy < 100) {
			ipoint = i;
			break;
		}
	}
	if(ipoint < 0) {
		std::cout<<"Failed to find point close to "<<px<<", "<<py<<std::endl;
		return ipoint;
	}
	std::cout<<__PRETTY_FUNCTION__<<std::endl;
	Print();
	std::cout<<ipoint<<": "<<fTotalGraph->RemovePoint(ipoint)<<std::endl;
	// no need to check if we can remove the residual point in this case
	std::cout<<ipoint<<": "<<fTotalResidualGraph->RemovePoint(ipoint)<<std::endl;
	// need to find which of the graphs we have to remove this point from -> use fGraphIndex[ipoint]
	// and also which point this is of the graph -> use fPointIndex[ipoint]
	std::cout<<ipoint<<" - "<<fGraphIndex[ipoint]<<", "<<fPointIndex[ipoint]<<": "<<fGraphs[fGraphIndex[ipoint]]->RemovePoint(fPointIndex[ipoint])<<std::endl;
	std::cout<<ipoint<<" - "<<fGraphIndex[ipoint]<<", "<<fPointIndex[ipoint]<<": "<<fResidualGraphs[fGraphIndex[ipoint]]->RemovePoint(fPointIndex[ipoint])<<std::endl;
	// and now also remove the points from the indices - also need to update all points after this point!
	auto oldGraph = fGraphIndex[ipoint];
	auto oldPoint = fPointIndex[ipoint];
	fGraphIndex.erase(fGraphIndex.begin()+ipoint);
	fPointIndex.erase(fPointIndex.begin()+ipoint);
	for(size_t p = 0; p < fGraphIndex.size(); ++p) {
		if(fGraphIndex[p] == oldGraph && fPointIndex[p] > oldPoint) {
			--fPointIndex[p];
		}
	}
	auto mother = gPad->GetMother();
	std::cout<<"Got mother pad "<<mother->GetName()<<" from pad "<<gPad->GetName()<<std::endl;
	int pad = 0;
	while(mother->GetPad(pad) != nullptr) {
		mother->GetPad(pad)->Modified();
		std::cout<<"Modified pad "<<pad<<" = "<<mother->GetPad(pad)->GetName()<<std::endl;
		pad++;
	}
	Print();
	return ipoint;
}

void TCalibrationGraphSet::Scale()
{
	std::cout<<__PRETTY_FUNCTION__<<std::endl;
	double minRef = fGraphs[0]->GetPointX(0);
	double maxRef = fGraphs[0]->GetPointX(fGraphs[0]->GetN()-1);
	for(size_t g = 1; g < fGraphs.size(); ++g) {
		double* x = fGraphs[g]->GetX();
		double* y = fGraphs[g]->GetY();
		double* ey = fGraphs[g]->GetEY();
		if(maxRef < x[0] || x[fGraphs[g]->GetN()-1] < minRef) {
			// no overlap between the two graphs, for now we just skip this one, but we could try and compare it to all the other ones?
			std::cout<<"No overlap between 0. graph ("<<minRef<<" - "<<maxRef<<") and "<<g<<". graph ("<<x[0]<<" - "<<x[fGraphs[g]->GetN()-1]<<")"<<std::endl;
			continue;
		}
		// we have an overlap, so we calculate the scaling factor for each point and take the average (maybe should add some weight from the errors bars)
		int count = 0;
		double sum = 0.;
		for(int p = 0; p < fGraphs[g]->GetN(); ++p) {
			if(minRef < x[p] && x[p] < maxRef) {
				sum += fGraphs[0]->Eval(x[p])/y[p];
				++count;
				std::cout<<g<<", "<<p<<": "<<count<<" - "<<sum<<", "<<fGraphs[0]->Eval(x[p])/y[p]<<std::endl;
			}
		}
		sum /= count;
		std::cout<<g<<": scaling with "<<sum<<std::endl;
		for(int p = 0; p < fGraphs[g]->GetN(); ++p) {
			y[p] *= sum;
			ey[p] *= sum;
		}
	}
	Print();
}

void TCalibrationGraphSet::Print()
{
	std::cout<<"TCalibrationGraphSet: "<<fGraphs.size()<<" calibration graphs, "<<fResidualGraphs.size()<<" residual graphs, "<<fLabel.size()<<" labels, "<<fTotalGraph->GetN()<<" calibration points, and "<<fTotalResidualGraph->GetN()<<" residual points"<<std::endl;
	for(auto g : fGraphs) {
		for(int p = 0; p < g->GetN(); ++p) {
			std::cout<<p<<" - "<<g->GetPointX(p)<<", "<<g->GetPointY(p)<<"; ";
		}
		std::cout<<std::endl;
	}
	std::cout<<fGraphIndex.size()<<" graph indices: ";
	for(auto i : fGraphIndex) std::cout<<i<<" ";
	std::cout<<std::endl;
	std::cout<<fPointIndex.size()<<" point indices: ";
	for(auto i : fPointIndex) std::cout<<i<<" ";
	std::cout<<std::endl;
}
