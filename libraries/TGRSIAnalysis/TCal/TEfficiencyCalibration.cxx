#include "TEfficiencyCalibration.h"

#include <iostream>
#include "Globals.h"
#include "Math/Minimizer.h"

/// \cond CLASSIMP
ClassImp(TEfficiencyCalibration)
/// \endcond

TEfficiencyCalibration::TEfficiencyCalibration() : TNamed(), fMultiGraph(nullptr), fFitting(false) {
	if(!fMultiGraph) fMultiGraph = new TMultiGraph;
	Clear();
}

TEfficiencyCalibration::TEfficiencyCalibration(const char* name, const char* title) : TNamed(name,title), fMultiGraph(nullptr), fFitting(false){ 
	if(!fMultiGraph) fMultiGraph = new TMultiGraph;
}

TEfficiencyCalibration::~TEfficiencyCalibration(){
	if(fMultiGraph) delete fMultiGraph;

	fMultiGraph = nullptr;
}

TEfficiencyCalibration::TEfficiencyCalibration(const TEfficiencyCalibration& copy) : TNamed(copy), fMultiGraph(nullptr), fFitting(false){
   copy.Copy(*this);
}

void TEfficiencyCalibration::Copy(TObject& copy) const {
   static_cast<TEfficiencyCalibration&>(copy).fGraphMap = fGraphMap;
	if(static_cast<TEfficiencyCalibration&>(copy).fMultiGraph){
		delete static_cast<TEfficiencyCalibration&>(copy).fMultiGraph;
		static_cast<TEfficiencyCalibration&>(copy).fMultiGraph = nullptr;	//This will be constructed later
	}
}

void TEfficiencyCalibration::Print(Option_t *opt) const {
	std::cout << "Graphs included: " << std::endl;
	for(auto it : fGraphMap){
		std::cout << "Name: " << it.first << " N of points: " << it.second.GetN() << std::endl;
	}
}

void TEfficiencyCalibration::Clear(Option_t *opt) {
	fGraphMap.clear();
	
	fFitting = false;
}

void TEfficiencyCalibration::AddEfficiencyGraph(const TEfficiencyGraph & graph, const char* name){
	auto it = fGraphMap.insert(std::make_pair(name,graph));
	if(!it.second){
		std::cout << "There is already a graph with the name " << name << " in this calibration, overwriting." << std::endl;
		it.first->second = graph;
	}
	fMultiGraph->Add(&(it.first->second)); //fill the multigraph with an actual pointer to the graph
}

void TEfficiencyCalibration::AddEfficiencyGraph(const TEfficiencyGraph & graph){
	AddEfficiencyGraph(graph, graph.GetName());
}

void TEfficiencyCalibration::Draw(Option_t* opt){
	fMultiGraph->Draw(opt);
}

void TEfficiencyCalibration::ScaleGuess(){

	//Make an initial graph scaling guess
	//Eventually loop over more graphs for a better guess
	
	//We choose to not scale the first graph in the list
	TList* graph_list = fMultiGraph->GetListOfGraphs();
	
	//We want to loop through the list and find the best scale factor
	for(int graph_idx=1; graph_idx<graph_list->GetSize(); ++graph_idx){ //Start at 1 because we don't want to change 0
		Double_t closest_dist = 99999.;
		Int_t closest_loop_idx = 0;
		Int_t closest_fixed_idx =0;
		TEfficiencyGraph *fixed_graph = static_cast<TEfficiencyGraph*>(graph_list->At(0));
		TEfficiencyGraph *loop_graph = static_cast<TEfficiencyGraph*>(graph_list->At(graph_idx));
		Double_t *fixed_x = fixed_graph->GetX();
		for(int i=0; i< fixed_graph->GetN(); ++i){
			if(loop_graph->FindDistToClosestPointX(fixed_x[i]) < closest_dist){
				closest_dist = loop_graph->FindDistToClosestPointX(fixed_x[i]) ;
				closest_fixed_idx = i;
				closest_loop_idx = loop_graph->FindClosestPointX(fixed_x[i]);
			}
		}
		//We have now found the two closest points, scale them
		std::cout << "Scaling " <<graph_idx << " graph by " << fixed_graph->GetY()[closest_fixed_idx]/loop_graph->GetY()[closest_loop_idx]<< std::endl;
		loop_graph->Scale((fixed_graph->GetY()[closest_fixed_idx])/(loop_graph->GetY()[closest_loop_idx]));
	}

	
/*	Int_t counter = 0;
	for(auto &it : fGraphMap){
		if(0 == counter){ counter++; continue;}
		//Find the two closest points from each graph.
		Int_t closest_begin_idx = 0;
		Int_t closest_it_idx = 0;
		Double_t closest_dist = 99999.;
		Double_t* x_begin = fGraphMap.begin()->second.GetX();
		for(int i = 0; i< fGraphMap.begin()->second.GetN(); ++i){
			for(int j =0; j < it.second.GetN(); ++j){
				if(fGraphMap.begin()->second.FindDistToClosestPointX(x_begin[i]) < closest_dist){
					closest_dist = fGraphMap.begin()->second.FindDistToClosestPointX(x_begin[i]) ;
					closest_begin_idx = i;
					closest_it_idx = j;
				}
			}
		}
		//Get the y values of these points and compare them
		std::cout << "Scaling " <<counter << "graph by " << fGraphMap.begin()->second.GetY()[closest_begin_idx]/it.second.GetY()[closest_it_idx]<< std::endl;
		it.second.Scale((fGraphMap.begin()->second.GetY()[closest_begin_idx])/(it.second.GetY()[closest_it_idx]));
		counter++;
	}*/


}
/*
void TEfficiencyCalibration::FitOverlap(TF1* fit){
	Int_t closest_graph = 0;
	Double_t dist_to_closest = 99999.;
	TList * gList = fMultiGraph->GetListOfGraphs();

	for(Int_t i = 0; i < gList->GetSize(); ++i){
		

			if((static_cast<TEfficiencyGraph*>(gList->At(i)))->FindDistToClosestPointX(x[0]) < dist_to_closest){
				dist_to_closest = (static_cast<TEfficiencyGraph*>(gList->At(i)))->FindDistToClosestPointX(x[0]); 
				closest_graph = i;
			}
		}
	
}
*/

TFitResultPtr TEfficiencyCalibration::Fit(Option_t *opt){
	TF1* fit = new TF1("fit",this,&TEfficiencyCalibration::PhotoPeakEfficiency,0,8000,8+fGraphMap.size(),"TEfficiencyCalibration","PhotoPeakEfficiency");	
	
	//Start by naming the parameters of the fit
	int mapIdx = 0;
	for(auto it : fGraphMap){
		fit->SetParName(mapIdx,Form("Scale_%d",mapIdx));
		if(mapIdx == 0){
			fit->FixParameter(mapIdx++,1.0);
		}
		else{
			fit->SetParameter(mapIdx,1.0);
			fit->SetParLimits(mapIdx++,0.1,10.);

		}
	}

	void ScaleGuess();
	//void FitOverLap(fit);

	//Make initial guesses for the fit parameters
	for(size_t i=0; i< 8; ++i){
		fit->SetParName(i+fGraphMap.size(),Form("a_%lu",i));
		fit->SetParameter(i+fGraphMap.size(),0.00001);
	}
	fit->SetParameter(fGraphMap.size(),5.0);
	//We fix the higher order parameters to get to the real minimum more easily
	fit->FixParameter(fGraphMap.size()+4,0.0);
	fit->FixParameter(fGraphMap.size()+5,0.0);
	fit->FixParameter(fGraphMap.size()+6,0.0);
	fit->FixParameter(fGraphMap.size()+7,0.0);
	fit->Print();
	
	//Turn the fitting flag on so that we scale graphs properly.
	fFitting = true;

	//Fix scaling
	for(size_t i=1; i<fGraphMap.size(); ++i){
		fit->FixParameter(i,fit->GetParameter(i));
	}
	
	//Slowly Add parameters back
	fMultiGraph->Fit(fit,"R0");
	fit->ReleaseParameter(fGraphMap.size()+5);
	fMultiGraph->Fit(fit,"R0");
	fit->ReleaseParameter(fGraphMap.size()+6);
	fMultiGraph->Fit(fit,"R0");
	fit->ReleaseParameter(fGraphMap.size()+7);

	//Fit only the very last parameter
	for(size_t i=0; i< 7 + fGraphMap.size(); ++i){
		fit->FixParameter(i,fit->GetParameter(i));
	}
	fMultiGraph->Fit(fit,"R0");

	fit->SetRange(200,8000);
	//Fit the scaling factor again, fix everything else
	for(size_t i=fGraphMap.size(); i<fGraphMap.size()+7; ++i){
		fit->FixParameter(i,fit->GetParameter(i));
	}
	for(size_t i=1; i<fGraphMap.size(); ++i){
		fit->ReleaseParameter(i);
	}
	fMultiGraph->Fit(fit,"R0");
	
	fit->SetRange(0,8000);
	//Fix Scale factors and redo fit of other parameters
	for(size_t i=1; i<fGraphMap.size(); ++i){
		fit->FixParameter(i,fit->GetParameter(i));
	}
	for(size_t i=fGraphMap.size(); i< 7 + fGraphMap.size(); ++i){
		fit->ReleaseParameter(i);
	}
	
	fMultiGraph->Fit(fit,"R0");
	
	//Fix Scale factors and redo fit of other parameters
	for(size_t i=1; i<fGraphMap.size(); ++i){
		fit->ReleaseParameter(i);
	}
	for(size_t i=fGraphMap.size(); i< 7 + fGraphMap.size(); ++i){
		fit->ReleaseParameter(i);
	}


	//Do the real fit with all of the parameters
	TFitResultPtr res = fMultiGraph->Fit(fit,"SR");

	
	//Turn fitting flag off for drawing
	fFitting = false;

	//Draw The TF1
	fit->Draw("same");
	
	//Update the graphs
	for(int i =0; i<fMultiGraph->GetListOfGraphs()->GetSize(); ++i){
		(static_cast<TEfficiencyGraph*>(fMultiGraph->GetListOfGraphs()->At(i)))->Scale(fit->GetParameter(i));
	}

	return res;

}

Double_t TEfficiencyCalibration::PhotoPeakEfficiency( Double_t *x, Double_t *par){

//	std::cout << "Fitting x = " << x[0] << std::endl;

	Int_t closest_graph = 0;
	Double_t dist_to_closest = 99999.;
	if(fFitting){
		TList * gList = fMultiGraph->GetListOfGraphs();
		for(Int_t i = 0; i < gList->GetSize(); ++i){
			if((static_cast<TEfficiencyGraph*>(gList->At(i)))->FindDistToClosestPointX(x[0]) < dist_to_closest){
				dist_to_closest = (static_cast<TEfficiencyGraph*>(gList->At(i)))->FindDistToClosestPointX(x[0]); 
				closest_graph = i;
			}
		}
	}

	double sum = 0.0;
	for(int i = 0; i < 9; ++i){
		sum += par[i+fGraphMap.size()]*TMath::Power(TMath::Log(x[0]),i);
	}
	if(fFitting){
		return TMath::Exp(sum)/par[closest_graph];
	}
	else{
		return TMath::Exp(sum);
	}
}

