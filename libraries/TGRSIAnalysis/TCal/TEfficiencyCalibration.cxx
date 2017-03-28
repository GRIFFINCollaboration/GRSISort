#include "TEfficiencyCalibration.h"

#include <iostream>
#include "Globals.h"
#include "TMath.h"
#include "Math/Minimizer.h"

/// \cond CLASSIMP
ClassImp(TEfficiencyCalibration)
/// \endcond

TEfficiencyCalibration::TEfficiencyCalibration() : TNamed(), fRelativeEffGraph(nullptr), fAbsEffGraph(nullptr), fFitting(false), fRelativeFit(nullptr), fAbsoluteFunc(nullptr) {
	if(!fRelativeEffGraph) fRelativeEffGraph = new TMultiGraph;
	if(!fAbsEffGraph) fAbsEffGraph = new TMultiGraph;
	Clear();
}

TEfficiencyCalibration::TEfficiencyCalibration(const char* name, const char* title) : TNamed(name,title), fRelativeEffGraph(nullptr), fAbsEffGraph(nullptr), fFitting(false), fRelativeFit(nullptr), fAbsoluteFunc(nullptr){ 
	if(!fRelativeEffGraph) fRelativeEffGraph = new TMultiGraph;
	if(!fAbsEffGraph) fAbsEffGraph = new TMultiGraph;
}

TEfficiencyCalibration::~TEfficiencyCalibration(){
	if(fRelativeEffGraph) delete fRelativeEffGraph;
	if(fAbsEffGraph) delete fAbsEffGraph;
	if(fRelativeFit) delete fRelativeFit;
	if(fAbsoluteFunc) delete fAbsoluteFunc;

	fRelativeEffGraph = nullptr;
	fAbsEffGraph = nullptr;
	fRelativeFit = nullptr;
	fAbsoluteFunc = nullptr;
}

TEfficiencyCalibration::TEfficiencyCalibration(const TEfficiencyCalibration& copy) : TNamed(copy), fRelativeEffGraph(nullptr), fAbsEffGraph(nullptr), fFitting(false), fRelativeFit(nullptr), fAbsoluteFunc(nullptr) {
   copy.Copy(*this);
}

void TEfficiencyCalibration::Copy(TObject& copy) const {
   static_cast<TEfficiencyCalibration&>(copy).fGraphMap = fGraphMap;
/*	if(static_cast<TEfficiencyCalibration&>(copy).fRelativeEffGraph){
		delete static_cast<TEfficiencyCalibration&>(copy).fRelativeEffGraph;
		static_cast<TEfficiencyCalibration&>(copy).fRelativeEffGraph = nullptr;	//This will be constructed later
	}*/
}

void TEfficiencyCalibration::Print(Option_t *opt) const {
	std::cout << "Graphs included: " << std::endl;
	for(auto it : fGraphMap){
		std::cout << "Name: " << it.first << " N of points: " << it.second.GetN();
		if(it.second.IsAbsolute())
			std::cout << "  Absolute calibration "; 
		std::cout << std::endl;
	}
	if(fRelativeFit){
		std::cout << "Relative Fit: " << std::endl;
		fRelativeFit->Print();
	}
	if(fAbsoluteFunc){
		std::cout << "Absolute Fit: " << std::endl;
		fAbsoluteFunc->Print();
	}
}

void TEfficiencyCalibration::Clear(Option_t *opt) {
	fGraphMap.clear();
	if(fRelativeFit) fRelativeFit->Clear();
	fFitting = false;
}

void TEfficiencyCalibration::AddEfficiencyGraph(const TEfficiencyGraph & graph, const char* name){
	auto it = fGraphMap.insert(std::make_pair(name,graph));
	if(!it.second){
		std::cout << "There is already a graph with the name " << name << " in this calibration, overwriting." << std::endl;
		it.first->second = graph;
	}
	if(graph.IsAbsolute()){
		fAbsEffGraph->Add(&(it.first->second)); //fill the multigraph with an actual pointer to the graph
	}
	else{
		fRelativeEffGraph->Add(&(it.first->second)); //fill the multigraph with an actual pointer to the graph
	}
	if(fRelativeFit){
		delete fRelativeFit; fRelativeFit = nullptr; //Clear this because it is nonsense now.
	}
}

void TEfficiencyCalibration::AddEfficiencyGraph(const TEfficiencyGraph & graph){
	AddEfficiencyGraph(graph, graph.GetName());
}

void TEfficiencyCalibration::Draw(Option_t* opt){
	fRelativeEffGraph->Draw(opt);
	if(fAbsoluteFunc){
		fAbsEffGraph->Draw("P");
	//	fAbsEffGraph->Draw(Form("%ssame",opt));
		fAbsoluteFunc->Draw("same");
	} else if(fRelativeFit) {
		fRelativeFit->Draw("same");
	}
}

void TEfficiencyCalibration::DrawRelative(Option_t* opt){
	fRelativeEffGraph->Draw(opt);
	if(fRelativeFit) fRelativeFit->Draw("same");
}

void TEfficiencyCalibration::DrawAbsolute(Option_t* opt){
	fAbsEffGraph->Draw(opt);
	if(fAbsoluteFunc) fAbsoluteFunc->Draw("same");
}

void TEfficiencyCalibration::ScaleGuess(){

	//Make an initial graph scaling guess
	//Eventually loop over more graphs for a better guess
	
	//We choose to not scale the first graph in the list
	TList* graph_list = fRelativeEffGraph->GetListOfGraphs();
	
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

}

TFitResultPtr TEfficiencyCalibration::Fit(Option_t *opt){
	//This fits the relative efficiency curve
	UInt_t n_rel_graphs = fRelativeEffGraph->GetListOfGraphs()->GetSize();
	if(fRelativeFit) delete fRelativeFit;
	fRelativeFit = new TF1("fRelativeFit",this,&TEfficiencyCalibration::PhotoPeakEfficiency,0,8000,8+n_rel_graphs,"TEfficiencyCalibration","PhotoPeakEfficiency");	
	
	//Start by naming the parameters of the fit
	int mapIdx = 0;
	for(auto it : fGraphMap){
		fRelativeFit->SetParName(mapIdx,Form("Scale_%d",mapIdx));
		if(mapIdx == 0){
			fRelativeFit->FixParameter(mapIdx++,1.0);
		}
		else{
			fRelativeFit->SetParameter(mapIdx,1.0);
			fRelativeFit->SetParLimits(mapIdx++,0.1,10.);

		}
	}

	void ScaleGuess();

	//Make initial guesses for the fit parameters
	for(size_t i=0; i< 8; ++i){
		fRelativeFit->SetParName(i+n_rel_graphs,Form("a_%lu",i));
		fRelativeFit->SetParameter(i+n_rel_graphs,0.00001);
	}
	fRelativeFit->SetParameter(n_rel_graphs,5.0);
	//We fix the higher order parameters to get to the real minimum more easily
	fRelativeFit->FixParameter(n_rel_graphs+4,0.0);
	fRelativeFit->FixParameter(n_rel_graphs+5,0.0);
	fRelativeFit->FixParameter(n_rel_graphs+6,0.0);
	fRelativeFit->FixParameter(n_rel_graphs+7,0.0);
	fRelativeFit->Print();
	
	//Turn the fitting flag on so that we scale graphs properly.
	fFitting = true;

	//Fix scaling
	for(size_t i=1; i<n_rel_graphs; ++i){
		fRelativeFit->FixParameter(i,fRelativeFit->GetParameter(i));
	}
	
	//Slowly Add parameters back
	fRelativeEffGraph->Fit(fRelativeFit,"R0");
	fRelativeFit->ReleaseParameter(n_rel_graphs+5);
	fRelativeEffGraph->Fit(fRelativeFit,"R0");
	fRelativeFit->ReleaseParameter(n_rel_graphs+6);
	fRelativeEffGraph->Fit(fRelativeFit,"R0");
	fRelativeFit->ReleaseParameter(n_rel_graphs+7);

	//Fit only the very last parameter
	for(size_t i=0; i< 7 + n_rel_graphs; ++i){
		fRelativeFit->FixParameter(i,fRelativeFit->GetParameter(i));
	}
	fRelativeEffGraph->Fit(fRelativeFit,"R0");

	fRelativeFit->SetRange(200,8000);
	//Fit the scaling factor again, fix everything else
	for(size_t i=n_rel_graphs; i<n_rel_graphs+7; ++i){
		fRelativeFit->FixParameter(i,fRelativeFit->GetParameter(i));
	}
	for(size_t i=1; i<n_rel_graphs; ++i){
		fRelativeFit->ReleaseParameter(i);
	}
	fRelativeEffGraph->Fit(fRelativeFit,"R0");
	
	fRelativeFit->SetRange(0,8000);
	//Fix Scale factors and redo fit of other parameters
	for(size_t i=1; i<n_rel_graphs; ++i){
		fRelativeFit->FixParameter(i,fRelativeFit->GetParameter(i));
	}
	for(size_t i=n_rel_graphs; i< 7 + n_rel_graphs; ++i){
		fRelativeFit->ReleaseParameter(i);
	}
	
	fRelativeEffGraph->Fit(fRelativeFit,"R0");
	
	//Fix Scale factors and redo fit of other parameters
	for(size_t i=1; i<n_rel_graphs; ++i){
		fRelativeFit->ReleaseParameter(i);
	}
	for(size_t i=n_rel_graphs; i< 7 + n_rel_graphs; ++i){
		fRelativeFit->ReleaseParameter(i);
	}


	//Do the real fit with all of the parameters
	TFitResultPtr res = fRelativeEffGraph->Fit(fRelativeFit,"SR");

	
	//Turn fitting flag off for drawing
	fFitting = false;

	//Draw The TF1
	fRelativeFit->Draw("same");
	
	//Update the graphs
	for(int i =0; i<fRelativeEffGraph->GetListOfGraphs()->GetSize(); ++i){
		(static_cast<TEfficiencyGraph*>(fRelativeEffGraph->GetListOfGraphs()->At(i)))->Scale(fRelativeFit->GetParameter(i));
	}

	return res;

}

Double_t TEfficiencyCalibration::PhotoPeakEfficiency( Double_t *x, Double_t *par){

	Int_t closest_graph = 0;
	Double_t dist_to_closest = 99999.;
	if(fFitting){
		TList * gList = fRelativeEffGraph->GetListOfGraphs();
		for(Int_t i = 0; i < gList->GetSize(); ++i){
			if((static_cast<TEfficiencyGraph*>(gList->At(i)))->FindDistToClosestPointX(x[0]) < dist_to_closest){
				dist_to_closest = (static_cast<TEfficiencyGraph*>(gList->At(i)))->FindDistToClosestPointX(x[0]); 
				closest_graph = i;
			}
		}
	}

	double sum = 0.0;
	for(int i = 0; i < 9; ++i){
		sum += par[i+fRelativeEffGraph->GetListOfGraphs()->GetSize()]*TMath::Power(TMath::Log(x[0]),i);
	}
	if(fFitting){
		return TMath::Exp(sum)/par[closest_graph];
	}
	else{
		return TMath::Exp(sum);
	}
}

Double_t TEfficiencyCalibration::AbsoluteEfficiency( Double_t *x, Double_t *par){

	double sum = 0.0;
	for(int i = 0; i < 9; ++i){
		sum += par[i+1]*TMath::Power(TMath::Log(x[0]),i);
	}
	return par[0]*TMath::Exp(sum);
}

bool TEfficiencyCalibration::ScaleToAbsolute() {
	if(fAbsEffGraph->GetListOfGraphs()->GetSize() && fRelativeFit){
		if(!fAbsoluteFunc)
			fAbsoluteFunc = new TF1("fAbsoluteFunc",this,&TEfficiencyCalibration::AbsoluteEfficiency,0,8000,9,"TEfficiencyCalibration","AbsoluteEfficiency");	
		fAbsoluteFunc->SetParName(0, "Scale");
		for(int i = 0; i < 8; ++i){
			fAbsoluteFunc->SetParName(i+1,Form("a%d",i));
			fAbsoluteFunc->SetParameter(i+1,fRelativeFit->GetParameter(i+fRelativeEffGraph->GetListOfGraphs()->GetSize()));
			fAbsoluteFunc->SetParError(i+1,fRelativeFit->GetParError(i+fRelativeEffGraph->GetListOfGraphs()->GetSize()));
		}

		//we need to find the average amount that we must scale the relative efficiency fit by in order to have it line up with the absolute efficiency data points.
		TIter next(fAbsEffGraph->GetListOfGraphs());
		Double_t w_avg_numer = 0.0;
		Double_t w_avg_denom = 0.0;
		Double_t semi_w_uncert = 0.0;
		while (TEfficiencyGraph *abs_graph = static_cast<TEfficiencyGraph*>(next())){
			Double_t *y_val = abs_graph->GetY();
			Double_t *ey_val = abs_graph->GetEY();
			Double_t *x_val = abs_graph->GetX();
			for(int i = 0; i < abs_graph->GetN(); ++i){			
				Double_t scale 	= y_val[i]/fRelativeFit->Eval(x_val[i]);	
				w_avg_numer += scale/TMath::Power(ey_val[i],2.0);
				w_avg_denom += 1./TMath::Power(ey_val[i],2.0);
			}
			semi_w_uncert += 1./TMath::Power(ey_val[0]/y_val[0],2.0);
		}
		
		Double_t w_avg = w_avg_numer/w_avg_denom; //This is how much we should scale everything by
		//Set the parameter and uncertainty based on the scale factor
		fAbsoluteFunc->FixParameter(0,w_avg);
		//For the error we assume that each absolute efficiency source error is entirely systematic in Activity.
		fAbsoluteFunc->SetParError(0,TMath::Sqrt(1./semi_w_uncert)*w_avg);

		//Scale all of the data points now in the relative graph
		for(int i =0; i<fRelativeEffGraph->GetListOfGraphs()->GetSize(); ++i){
			(static_cast<TEfficiencyGraph*>(fRelativeEffGraph->GetListOfGraphs()->At(i)))->Scale(w_avg);
		}

		return true;
	}

	return false;
}

Double_t TEfficiencyCalibration::GetEfficiency(const Double_t& eng){
	if(fAbsoluteFunc)
		return fAbsoluteFunc->Eval(eng);

	return -1000.0;
}

Double_t TEfficiencyCalibration::GetEfficiencyErr(const Double_t& eng){
	if(fAbsoluteFunc){
		//partial derivative * error all squared for each parameter
		//Function looks like Const*exp^(a0 + a1*lnE + a2*(lnE)^2 +....)
		//so exp term shows up in every derivative
		Double_t exp_term = 0.0;
		for(int i=0; i< 8; ++i){
			exp_term += fAbsoluteFunc->GetParameter(i+1)*TMath::Power(TMath::Log(eng),i);
		}
		exp_term = TMath::Exp(exp_term);
		//Now do the derivatives which have a pattern, and say the error in E is negligible
		Double_t sum = TMath::Power(exp_term*fAbsoluteFunc->GetParError(0),2.0);
		for(int i =0; i<8; ++i){
			sum += TMath::Power(fAbsoluteFunc->GetParameter(0)*exp_term*TMath::Power(TMath::Log(eng),i)*fAbsoluteFunc->GetParError(i+1),2.0);
		}
		return TMath::Sqrt(sum);
	}	

	return -1000.0;
}

