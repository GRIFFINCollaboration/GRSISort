#include "TCalGraph.h"

#include <iostream>
#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TCalGraph)
/// \endcond

TCalGraph::TCalGraph() : TGraphErrors() {
	Clear();
}

TCalGraph::~TCalGraph(){}

TCalGraph::TCalGraph(const TCalGraph& copy) : TGraphErrors(copy){
 //  copy.Copy(*this);
}

void TCalGraph::Print(Option_t *opt) const {
	TGraphErrors::Print(opt);
	for(auto it : fCompareMap){
		std::cout << DBLUE<< " Data " << RESET_COLOR << std::endl;
		it.second.first.Print();
		std::cout << DGREEN << " Source " << RESET_COLOR<< std::endl;
		it.second.second.Print();
		std::cout << std::endl;
	}
}

void TCalGraph::Clear(Option_t *opt) {
	TGraphErrors::Clear(opt);
	fCompareMap.clear();
}

void TCalGraph::AddPoint(const TCalPoint& cal_point){
	Int_t idx = GetN();
	SetPoint(idx,cal_point.Centroid(),cal_point.Area());
	SetPointError(idx,cal_point.CentroidErr(), cal_point.AreaErr());
}

Int_t TCalGraph::AddLists(const TCalList& cal_list, const TSourceList& src_list){
	//This is where we build the compared list
	const auto &cal_map = cal_list.Map();
	const auto &src_map = src_list.Map();

	std::vector<UInt_t> missing_cal_values;

	//Look in cal_list for src_list entries
	for(const auto &cl : cal_map){
		const auto &it = src_map.find(cl.first);
		if(it != src_map.cend()){ //we found the matching data point
			fCompareMap.insert(std::make_pair(cl.first,std::make_pair(cl.second,it->second)));
		}		
		missing_cal_values.push_back(cl.first);
	}
	return fCompareMap.size();
}

