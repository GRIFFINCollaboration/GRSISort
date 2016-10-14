#include <iostream>
#include <string>

#include "TFile.h"
#include "TList.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TH2D.h"
#include "THnSparse.h"

#include "TPeak.h"
#include "GPeak.h"

#define N 16

#include "GateBG.C"

int main(int argc, char** argv) {
	if(argc != 2 && argc != 3) {
		std::cout<<"Usage: "<<argv[0]<<" <input file name> <optional output file name>"<<std::endl;
		return 1;
	}
	TFile* file = new TFile(argv[1]);
	if(!file->IsOpen()) {
		return 1;
	}
	std::string outputFileName = "FitMg22.root";
	if(argc == 3) outputFileName = argv[2];

	TList list;

	// fit singles peaks at 74, 583, 1280, 1354, and 1937
	TH1F* gSingles = (TH1F*) file->Get("gammaSingles"); list.Add(gSingles);

	TPeak* p74   = new TPeak(  74.,   60.,   90.); list.Add(p74);
	p74->SetName("p74");
	p74->Fit(gSingles,"+");
	TPeak* p583  = new TPeak( 583.,  560.,  600.); list.Add(p583);
	p583->SetName("p583");
	p583->Fit(gSingles,"+");
	TPeak* p1280 = new TPeak(1280., 1260., 1290.); list.Add(p1280);
	p1280->SetName("p1280");
	p1280->Fit(gSingles,"+");
	TPeak* p1354 = new TPeak(1354., 1330., 1365.); list.Add(p1354);
	p1354->SetName("p1354");
	p1354->Fit(gSingles,"+");
	TPeak* p1937 = new TPeak(1937., 1900., 1990.); list.Add(p1937);
	p1937->SetName("p1937");
	//p1937->Fit(gSingles,"+");

	// fit gated peaks for different time windows
	THnSparseF* gglongmatrixT = (THnSparseF*) file->Get("gglongmatrixT");

	double time[N] = {0.,50.,100.,150., 200.,250.,300.,350., 400.,500., 750.,1000., 1250.,1500.,1750., 2000. };
	double time_err[N] = { 0., 0., 0., 0., 0., 0. };
	TH2D* ggmatrix[N];
	TH1D* gg_py[N];
	GPeak* p1280gate[N];
	TH1* gate1280[N];
	TPeak* p74_gated[N];
	TPeak* p583_gated[N];

	// loop over coincidence times
	for(int i = 0; i < N; ++i) {
	//for(int i = 0; i < 1; ++i) {
		std::cout<<std::endl
			      <<"Starting on "<<i<<". time window "<<time[i]<<std::endl
					<<std::endl;

		gglongmatrixT->GetAxis(2)->SetRange(gglongmatrixT->GetAxis(2)->FindBin(-200.), 
														gglongmatrixT->GetAxis(2)->FindBin( time[i]));
		gglongmatrixT->SetName(Form("ggmatrix_%.0f",time[i]));
		ggmatrix[i] = gglongmatrixT->Projection(0,1); //list.Add(ggmatrix[i]);
		gg_py[i] = ggmatrix[i]->ProjectionX(); list.Add(gg_py[i]);
		// fit gate at 1280, create gated spectrum, and fit peaks at 74 and 583
		p1280gate[i] = new GPeak(1280., 1260., 1290.); list.Add(p1280gate[i]);
		p1280gate[i]->Fit(gg_py[i],"Q+");
		p1280gate[i]->SetName(Form("p1280gate_%.0f",time[i]));
		gate1280[i] = ProjectionYBGP(ggmatrix[i],1276,1284,1289,1349); list.Add(gate1280[i]);

		p74_gated[i] = new TPeak(74., 65., 85.); list.Add(p74_gated[i]);
		p74_gated[i]->SetName(Form("p74_gated_%.0f",time[i]));
		p74_gated[i]->SetParLimits(2, 0.5, 3.);//sigma
		p74_gated[i]->Fit(gate1280[i],"Q+");

		p583_gated[i] = new TPeak(583., 560., 600.); list.Add(p583_gated[i]);
		p583_gated[i]->SetName(Form("p583_gated_%.0f",time[i]));
		p583_gated[i]->InitParams(gate1280[i]);
		p583_gated[i]->SetParLimits(1, 580., 586.);//centroid
		p583_gated[i]->SetParLimits(2, 0.5, 3.);//sigma
		p583_gated[i]->SetParLimits(7, 0., 1.);//BG slope
		p583_gated[i]->Fit(gate1280[i],"+");
	}

	std::cout<<"singles:"<<std::endl
				<<"\t"<<  p74->GetCentroid()<<" +- "<<  p74->GetCentroidErr()<<" keV: "<<  p74->GetArea()<<" +- "<<  p74->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<< p583->GetCentroid()<<" +- "<< p583->GetCentroidErr()<<" keV: "<< p583->GetArea()<<" +- "<< p583->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<<p1280->GetCentroid()<<" +- "<<p1280->GetCentroidErr()<<" keV: "<<p1280->GetArea()<<" +- "<<p1280->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<<p1354->GetCentroid()<<" +- "<<p1354->GetCentroidErr()<<" keV: "<<p1354->GetArea()<<" +- "<<p1354->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<<p1937->GetCentroid()<<" +- "<<p1937->GetCentroidErr()<<" keV: "<<p1937->GetArea()<<" +- "<<p1937->GetAreaErr()<<" counts"<<std::endl
				<<std::endl;

	double p74_gated_area[N];
	double p74_gated_area_err[N];
	double p74_gated_sigma[N];
	double p74_gated_sigma_err[N];
	double p74_gated_centr[N];
	double p74_gated_centr_err[N];
	double p74_gated_r[N];
	double p74_gated_r_err[N];
	double p583_gated_area[N];
	double p583_gated_area_err[N];
	double p583_gated_sigma[N];
	double p583_gated_sigma_err[N];
	double p583_gated_centr[N];
	double p583_gated_centr_err[N];
	double p583_gated_r[N];
	double p583_gated_r_err[N];
	double rel_effic[N];
	double rel_effic_err[N];
	for(int i = 0; i < N; ++i) {
		std::cout<<"time gate -200. - "<<time[i]<<" ns:"<<std::endl
					<<"\t"<< p74_gated[i]->GetCentroid()<<" +- "<< p74_gated[i]->GetCentroidErr()<<" keV: "<< p74_gated[i]->GetArea()<<" +- "<< p74_gated[i]->GetAreaErr()<<" counts"<<std::endl
					<<"\t"<<p583_gated[i]->GetCentroid()<<" +- "<<p583_gated[i]->GetCentroidErr()<<" keV: "<<p583_gated[i]->GetArea()<<" +- "<<p583_gated[i]->GetAreaErr()<<" counts"<<std::endl
					<<std::endl;

		p74_gated_area[i] =      p74_gated[i]->GetArea();
		p74_gated_area_err[i] =  p74_gated[i]->GetAreaErr();
		p74_gated_sigma[i] =     p74_gated[i]->GetParameter(2);
		p74_gated_sigma_err[i] = p74_gated[i]->GetParError(2);
		p74_gated_centr[i] =     p74_gated[i]->GetParameter(1);
		p74_gated_centr_err[i] = p74_gated[i]->GetParError(1);
		p74_gated_r[i] =         p74_gated[i]->GetParameter(4);
		p74_gated_r_err[i] =     p74_gated[i]->GetParError(4);
		p583_gated_area[i] =     p583_gated[i]->GetArea();
		p583_gated_area_err[i] = p583_gated[i]->GetAreaErr();
		p583_gated_sigma[i] =    p583_gated[i]->GetParameter(2);
		p583_gated_sigma_err[i] =p583_gated[i]->GetParError(2);
		p583_gated_centr[i] =    p583_gated[i]->GetParameter(1);
		p583_gated_centr_err[i] =p583_gated[i]->GetParError(1);
		p583_gated_r[i] =        p583_gated[i]->GetParameter(4);
		p583_gated_r_err[i] =    p583_gated[i]->GetParError(4);
		rel_effic[i] = p583_gated_area[i]/p74_gated_area[i];
		rel_effic_err[i] = TMath::Sqrt(TMath::Power(p583_gated_area_err[i]/p583_gated_area[i],2.) + TMath::Power(p74_gated_area_err[i]/p74_gated_area[i],2.));
	}

	TGraphErrors* p74_time = new TGraphErrors(N, time, p74_gated_area, time_err, p74_gated_area_err); list.Add(p74_time);
	p74_time->SetName("p74_time");
	p74->SetMarkerColor(2);
	p74->SetLineColor(2);
	TGraphErrors* p583_time = new TGraphErrors(N, time, p583_gated_area, time_err, p583_gated_area_err); list.Add(p583_time);
	p583_time->SetName("p583_time");
	p583->SetMarkerColor(4);
	p583->SetLineColor(4);
	TGraphErrors* rel_effic_graph = new TGraphErrors(N, time, rel_effic, time_err, rel_effic_err); list.Add(rel_effic_graph);
	rel_effic_graph->SetName("rel_effic_graph");

	TGraphErrors* p74_sigma = new TGraphErrors(N, time, p74_gated_sigma, time_err, p74_gated_sigma_err); list.Add(p74_sigma);
	p74_sigma->SetName("p74_sigma");
	TGraphErrors* p74_centr = new TGraphErrors(N, time, p74_gated_centr, time_err, p74_gated_centr_err); list.Add(p74_centr);
	p74_centr->SetName("p74_centr");
	TGraphErrors* p74_r = new TGraphErrors(N, time, p74_gated_r, time_err, p74_gated_r_err); list.Add(p74_r);
	p74_r->SetName("p74_r");
	TGraphErrors* p583_sigma = new TGraphErrors(N, time, p583_gated_sigma, time_err, p583_gated_sigma_err); list.Add(p583_sigma);
	p583_sigma->SetName("p583_sigma");
	TGraphErrors* p583_centr = new TGraphErrors(N, time, p583_gated_centr, time_err, p583_gated_centr_err); list.Add(p583_centr);
	p583_centr->SetName("p583_centr");
	TGraphErrors* p583_r = new TGraphErrors(N, time, p583_gated_r, time_err, p583_gated_r_err); list.Add(p583_r);
	p583_r->SetName("p583_r");

	std::cout<<"done, writing everything to "<<outputFileName<<std::endl;

	TFile output(outputFileName.c_str(), "recreate");
	output.cd();
	list.Write();
	output.Close();

	return 0;
}
