#include "TCalibrationList.h"

#include <iostream>

/// \cond CLASSIMP
ClassImp(TCalibrationList)
/// \endcond

TCalibrationList::TCalibrationList():TNamed(){
	Clear();
}

TCalibrationList::~TCalibrationList(){}

TCalibrationList::TCalibrationList(const char *name, const char* title) : TNamed(name,title) { 
	Clear();
}

TCalibrationList::TCalibrationList(const TCalibrationList& copy) : TNamed(copy){
   copy.Copy(*this);
}

void TCalibrationList::Copy(TObject& obj) const {
	TNamed::Copy(obj);
	static_cast<TCalibrationList&>(obj).Clear();
	for(auto it : fCalList){
		static_cast<TCalibrationList&>(obj).AddPoint(it.second);
	}
}

void TCalibrationList::AddPoint(const TCalibrationPoint& point){
	fCalList.insert(std::make_pair(std::floor(point.Centroid()),point));
}

void TCalibrationList::Print(Option_t *opt) const {
	int idx = 0;
	std::cout << GetName() << "   " << GetTitle() << std::endl;
	for(auto it : fCalList){
		std::cout << idx++ << "    " << it.first << std::endl;
		it.second.Print();
	}
}

void TCalibrationList::Clear(Option_t *opt) {
	fCalList.clear();
}

void TCalibrationList::FillGraph(TGraph *graph) const{
	graph->Clear();
	Int_t i=0;
	TGraphErrors* ge = dynamic_cast<TGraphErrors*>(graph);
	
	for(auto it : fCalList){
		graph->SetPoint(i,it.second.Centroid(),it.second.Area());
		if(ge)
			ge->SetPointError(i++,it.second.CentroidErr(),it.second.AreaErr());
	}
}
