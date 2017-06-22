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
	auto* file = new TFile(argv[1]);
	if(!file->IsOpen()) {
		return 1;
	}
	std::string outputFileName = "FitMg22.root";
	if(argc == 3) outputFileName = argv[2];

	TList list;

	auto* eff = new TF1("eff","TMath::Exp([0]+[1]*TMath::Log(x)+[2]*TMath::Power(TMath::Log(x),2)+[3]*TMath::Power(TMath::Log(x),3)+[4]*TMath::Power(TMath::Log(x),4)+[5]*TMath::Power(TMath::Log(x),5)+[6]*TMath::Power(TMath::Log(x),6)+[7]*TMath::Power(TMath::Log(x),7)+[8]*TMath::Power(TMath::Log(x),8))", 1., 2000.);
	eff->SetParameters(-5.20813, 0.594665, 0.303463, -0.0134724, -0.00879938, -0.000716062, 0.000119174, 0.000030346, -0.00000300406);
	list.Add(eff);

	// fit singles peaks at 74, 583, 1280, 1354, and 1937
	TH1F* gSingles = dynamic_cast<TH1F*>( file->Get("gammaSingles")); list.Add(gSingles);

	auto* p74   = new TPeak(  74.,   60.,   90.); list.Add(p74);
	p74->SetName("p74");
	p74->Fit(gSingles,"+");
	auto* p583  = new TPeak( 583.,  560.,  600.); list.Add(p583);
	p583->SetName("p583");
	p583->Fit(gSingles,"+");
	auto* p1280 = new TPeak(1280., 1260., 1290.); list.Add(p1280);
	p1280->SetName("p1280");
	p1280->Fit(gSingles,"+");
	auto* p1354 = new TPeak(1354., 1330., 1365.); list.Add(p1354);
	p1354->SetName("p1354");
	p1354->Fit(gSingles,"+");
	auto* p1937 = new TPeak(1937., 1900., 1990.); list.Add(p1937);
	p1937->SetName("p1937");
	p1937->Fit(gSingles,"+");

	// fit gated peaks for different time windows
	THnSparseF* gglongmatrixT = dynamic_cast<THnSparseF*>( file->Get("gglongmatrixT"));

	//create time spectra for different photo-peak combinations
	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p74->GetCentroid()-p74->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p74->GetCentroid()+p74->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p74->GetCentroid()-p74->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p74->GetCentroid()+p74->GetFWHM()));
	gglongmatrixT->SetName("timing_74_74");
	list.Add(gglongmatrixT->Projection(2));
	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p74->GetCentroid()-p74->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p74->GetCentroid()+p74->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p583->GetCentroid()-p583->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p583->GetCentroid()+p583->GetFWHM()));
	gglongmatrixT->SetName("timing_74_583");
	list.Add(gglongmatrixT->Projection(2));
	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p74->GetCentroid()-p74->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p74->GetCentroid()+p74->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p1280->GetCentroid()-p1280->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p1280->GetCentroid()+p1280->GetFWHM()));
	gglongmatrixT->SetName("timing_74_1280");
	list.Add(gglongmatrixT->Projection(2));

	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p583->GetCentroid()-p583->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p583->GetCentroid()+p583->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p74->GetCentroid()-p74->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p74->GetCentroid()+p74->GetFWHM()));
	gglongmatrixT->SetName("timing_583_74");
	list.Add(gglongmatrixT->Projection(2));
	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p583->GetCentroid()-p583->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p583->GetCentroid()+p583->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p583->GetCentroid()-p583->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p583->GetCentroid()+p583->GetFWHM()));
	gglongmatrixT->SetName("timing_583_583");
	list.Add(gglongmatrixT->Projection(2));
	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p583->GetCentroid()-p583->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p583->GetCentroid()+p583->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p1280->GetCentroid()-p1280->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p1280->GetCentroid()+p1280->GetFWHM()));
	gglongmatrixT->SetName("timing_583_1280");
	list.Add(gglongmatrixT->Projection(2));

	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p1280->GetCentroid()-p1280->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p1280->GetCentroid()+p1280->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p74->GetCentroid()-p74->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p74->GetCentroid()+p74->GetFWHM()));
	gglongmatrixT->SetName("timing_1280_74");
	list.Add(gglongmatrixT->Projection(2));
	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p1280->GetCentroid()-p1280->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p1280->GetCentroid()+p1280->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p583->GetCentroid()-p583->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p583->GetCentroid()+p583->GetFWHM()));
	gglongmatrixT->SetName("timing_1280_583");
	list.Add(gglongmatrixT->Projection(2));
	gglongmatrixT->GetAxis(0)->SetRange(gglongmatrixT->GetAxis(0)->FindBin(p1280->GetCentroid()-p1280->GetFWHM()), 
													gglongmatrixT->GetAxis(0)->FindBin(p1280->GetCentroid()+p1280->GetFWHM()));
	gglongmatrixT->GetAxis(1)->SetRange(gglongmatrixT->GetAxis(1)->FindBin(p1280->GetCentroid()-p1280->GetFWHM()), 
													gglongmatrixT->GetAxis(1)->FindBin(p1280->GetCentroid()+p1280->GetFWHM()));
	gglongmatrixT->SetName("timing_1280_1280");
	list.Add(gglongmatrixT->Projection(2));

	gglongmatrixT->GetAxis(0)->UnZoom();
	gglongmatrixT->GetAxis(1)->UnZoom();

	double time[N] = {0.,50.,100.,150., 200.,250.,300.,350., 400.,500., 750.,1000., 1250.,1500.,1750., 2000. };
	double time_err[N] = { 0., 0., 0., 0., 0., 0. };
	TH2D* ggmatrix[2*N];
	TH1D* gg_py[2*N];
	GPeak* p1280gate[2*N];
	TH1* gate1280[2*N];
	TPeak* p74_gated[2*N];
	TPeak* p583_gated[2*N];

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

		gglongmatrixT->GetAxis(2)->SetRange(gglongmatrixT->GetAxis(2)->FindBin(-time[i]), 
														gglongmatrixT->GetAxis(2)->FindBin(2000.));
		gglongmatrixT->SetName(Form("ggmatrix_%.0f",-time[i]));
		ggmatrix[i+N] = gglongmatrixT->Projection(0,1); //list.Add(ggmatrix[i]);
		gg_py[i+N] = ggmatrix[i+N]->ProjectionX(); list.Add(gg_py[i+N]);
		// fit gate at 1280, create gated spectrum, and fit peaks at 74 and 583
		p1280gate[i+N] = new GPeak(1280., 1260., 1290.); list.Add(p1280gate[i+N]);
		p1280gate[i+N]->Fit(gg_py[i+N],"Q+");
		p1280gate[i+N]->SetName(Form("p1280gate_%.0f",-time[i]));
		gate1280[i+N] = ProjectionYBGP(ggmatrix[i+N],1276,1284,1289,1349); list.Add(gate1280[i+N]);

		p74_gated[i+N] = new TPeak(74., 65., 85.); list.Add(p74_gated[i+N]);
		p74_gated[i+N]->SetName(Form("p74_gated_%.0f",-time[i]));
		p74_gated[i+N]->SetParLimits(2, 0.5, 3.);//sigma
		p74_gated[i+N]->Fit(gate1280[i+N],"Q+");

		p583_gated[i+N] = new TPeak(583., 560., 600.); list.Add(p583_gated[i+N]);
		p583_gated[i+N]->SetName(Form("p583_gated_%.0f",-time[i]));
		p583_gated[i+N]->InitParams(gate1280[i+N]);
		p583_gated[i+N]->SetParLimits(1, 580., 586.);//centroid
		p583_gated[i+N]->SetParLimits(2, 0.5, 3.);//sigma
		p583_gated[i+N]->SetParLimits(7, 0., 1.);//BG slope
		p583_gated[i+N]->Fit(gate1280[i+N],"+");
	}

	std::cout<<"singles:"<<std::endl
				<<"\t"<<  p74->GetCentroid()<<" +- "<<  p74->GetCentroidErr()<<" keV: "<<  p74->GetArea()<<" +- "<<  p74->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<< p583->GetCentroid()<<" +- "<< p583->GetCentroidErr()<<" keV: "<< p583->GetArea()<<" +- "<< p583->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<<p1280->GetCentroid()<<" +- "<<p1280->GetCentroidErr()<<" keV: "<<p1280->GetArea()<<" +- "<<p1280->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<<p1354->GetCentroid()<<" +- "<<p1354->GetCentroidErr()<<" keV: "<<p1354->GetArea()<<" +- "<<p1354->GetAreaErr()<<" counts"<<std::endl
				<<"\t"<<p1937->GetCentroid()<<" +- "<<p1937->GetCentroidErr()<<" keV: "<<p1937->GetArea()<<" +- "<<p1937->GetAreaErr()<<" counts"<<std::endl
				<<std::endl;

	double p74_gated_area[2*N];
	double p74_gated_area_err[2*N];
	double p74_gated_sigma[2*N];
	double p74_gated_sigma_err[2*N];
	double p74_gated_centr[2*N];
	double p74_gated_centr_err[2*N];
	double p74_gated_r[2*N];
	double p74_gated_r_err[2*N];
	double p583_gated_area[2*N];
	double p583_gated_area_err[2*N];
	double p583_gated_sigma[2*N];
	double p583_gated_sigma_err[2*N];
	double p583_gated_centr[2*N];
	double p583_gated_centr_err[2*N];
	double p583_gated_r[2*N];
	double p583_gated_r_err[2*N];
	double rel_effic[2*N];
	double rel_effic_err[2*N];
	double branch[2*N];
	double branch_err[2*N];
	double branch_hardy[2*N];
	double branch_hardy_err[2*N];
	double branch_releff[2*N];
	double branch_releff_err[2*N];
	double denom;
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

		denom = 1. + p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.);
		
		branch[i] = ((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))) / denom;
		
		branch_err[i] = TMath::Sqrt(TMath::Power(p74->GetAreaErr()/p583->GetArea()/denom,2) + TMath::Power(-(p583->GetAreaErr()/p583->GetArea()*p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]-p583->GetAreaErr()/p583->GetArea()*p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))*denom + ((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))*(p583->GetAreaErr()/p583->GetArea()*p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.))/TMath::Power(denom,2),2) + TMath::Power(p583_gated_area_err[i]/p74_gated_area[i]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p74_gated_area_err[i]/p74_gated_area[i]*p583_gated_area[i]/p74_gated_area[i]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p1280->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)/denom,2) + TMath::Power(p1937->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.)*((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))/TMath::Power(denom,2),2));

		denom = 1. + 0.032e-2;
		
		branch_hardy[i] = ((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - (5.40e-2)) / denom;
	
	   branch_hardy_err[i] = TMath::Sqrt(TMath::Power(p74->GetAreaErr()/p583->GetArea()/denom,2) + TMath::Power(-(p583->GetAreaErr()/p583->GetArea()*p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]-p583->GetAreaErr()/p583->GetArea()*5.4e-2)*denom + ((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - 5.4e-2)*(p583->GetAreaErr()/p583->GetArea()*0.032e-2)/TMath::Power(denom,2),2) + TMath::Power(p583_gated_area_err[i]/p74_gated_area[i]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p74_gated_area_err[i]/p74_gated_area[i]*p583_gated_area[i]/p74_gated_area[i]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(0.07e-2/denom,2) + TMath::Power(0.003e-2*((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - 5.4e-2)/TMath::Power(denom,2),2));

		denom = 1. + p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.);
		
		branch_releff[i] = ((p74->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(74.)) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))) / denom;
		
		branch_releff_err[i] = TMath::Sqrt(TMath::Power(p74->GetAreaErr()/p583->GetArea()/denom,2) + TMath::Power(-(p583->GetAreaErr()/p583->GetArea()*p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]-p583->GetAreaErr()/p583->GetArea()*p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))*denom + ((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))*(p583->GetAreaErr()/p583->GetArea()*p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.))/TMath::Power(denom,2),2) + TMath::Power(p583_gated_area_err[i]/p74_gated_area[i]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p74_gated_area_err[i]/p74_gated_area[i]*p583_gated_area[i]/p74_gated_area[i]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p1280->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)/denom,2) + TMath::Power(p1937->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.)*((p74->GetArea()/p583->GetArea()*p583_gated_area[i]/p74_gated_area[i]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))/TMath::Power(denom,2),2));

		std::cout << "The BR is = " << branch[i] << " +/- " << branch_err[i] << " With Hardy = " << branch_hardy[i] << " +/- " << branch_hardy_err[i] << std::endl;

		std::cout<<"time gate "<<-time[i]<<" - 2000. ns:"<<std::endl
					<<"\t"<< p74_gated[i+N]->GetCentroid()<<" +- "<< p74_gated[i+N]->GetCentroidErr()<<" keV: "<< p74_gated[i+N]->GetArea()<<" +- "<< p74_gated[i+N]->GetAreaErr()<<" counts"<<std::endl
					<<"\t"<<p583_gated[i+N]->GetCentroid()<<" +- "<<p583_gated[i+N]->GetCentroidErr()<<" keV: "<<p583_gated[i+N]->GetArea()<<" +- "<<p583_gated[i+N]->GetAreaErr()<<" counts"<<std::endl
					<<std::endl;

		p74_gated_area[i+N] =      p74_gated[i+N]->GetArea();
		p74_gated_area_err[i+N] =  p74_gated[i+N]->GetAreaErr();
		p74_gated_sigma[i+N] =     p74_gated[i+N]->GetParameter(2);
		p74_gated_sigma_err[i+N] = p74_gated[i+N]->GetParError(2);
		p74_gated_centr[i+N] =     p74_gated[i+N]->GetParameter(1);
		p74_gated_centr_err[i+N] = p74_gated[i+N]->GetParError(1);
		p74_gated_r[i+N] =         p74_gated[i+N]->GetParameter(4);
		p74_gated_r_err[i+N] =     p74_gated[i+N]->GetParError(4);
		p583_gated_area[i+N] =     p583_gated[i+N]->GetArea();
		p583_gated_area_err[i+N] = p583_gated[i+N]->GetAreaErr();
		p583_gated_sigma[i+N] =    p583_gated[i+N]->GetParameter(2);
		p583_gated_sigma_err[i+N] =p583_gated[i+N]->GetParError(2);
		p583_gated_centr[i+N] =    p583_gated[i+N]->GetParameter(1);
		p583_gated_centr_err[i+N] =p583_gated[i+N]->GetParError(1);
		p583_gated_r[i+N] =        p583_gated[i+N]->GetParameter(4);
		p583_gated_r_err[i+N] =    p583_gated[i+N]->GetParError(4);
		rel_effic[i+N] =           p583_gated_area[i+N]/p74_gated_area[i+N];
		rel_effic_err[i+N] = TMath::Sqrt(TMath::Power(p583_gated_area_err[i+N]/p583_gated_area[i+N],2.) + TMath::Power(p74_gated_area_err[i+N]/p74_gated_area[i+N],2.));

		denom = 1. + p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.);
		
		branch[i+N] = ((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))) / denom;
		
		branch_err[i+N] = TMath::Sqrt(TMath::Power(p74->GetAreaErr()/p583->GetArea()/denom,2) + TMath::Power(-(p583->GetAreaErr()/p583->GetArea()*p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]-p583->GetAreaErr()/p583->GetArea()*p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))*denom + ((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))*(p583->GetAreaErr()/p583->GetArea()*p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.))/TMath::Power(denom,2),2) + TMath::Power(p583_gated_area_err[i+N]/p74_gated_area[i+N]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p74_gated_area_err[i+N]/p74_gated_area[i+N]*p583_gated_area[i+N]/p74_gated_area[i+N]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p1280->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)/denom,2) + TMath::Power(p1937->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.)*((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))/TMath::Power(denom,2),2));

		denom = 1. + 0.032e-2;
		
		branch_hardy[i+N] = ((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - (5.40e-2)) / denom;
	
	   branch_hardy_err[i+N] = TMath::Sqrt(TMath::Power(p74->GetAreaErr()/p583->GetArea()/denom,2) + TMath::Power(-(p583->GetAreaErr()/p583->GetArea()*p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]-p583->GetAreaErr()/p583->GetArea()*5.4e-2)*denom + ((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - 5.4e-2)*(p583->GetAreaErr()/p583->GetArea()*0.032e-2)/TMath::Power(denom,2),2) + TMath::Power(p583_gated_area_err[i+N]/p74_gated_area[i+N]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p74_gated_area_err[i+N]/p74_gated_area[i+N]*p583_gated_area[i+N]/p74_gated_area[i+N]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(0.07e-2/denom,2) + TMath::Power(0.003e-2*((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - 5.4e-2)/TMath::Power(denom,2),2));

		denom = 1. + p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.);
		
		branch_releff[i+N] = ((p74->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(74.)) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))) / denom;
		
		branch_releff_err[i+N] = TMath::Sqrt(TMath::Power(p74->GetAreaErr()/p583->GetArea()/denom,2) + TMath::Power(-(p583->GetAreaErr()/p583->GetArea()*p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]-p583->GetAreaErr()/p583->GetArea()*p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.))*denom + ((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))*(p583->GetAreaErr()/p583->GetArea()*p1937->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.))/TMath::Power(denom,2),2) + TMath::Power(p583_gated_area_err[i+N]/p74_gated_area[i+N]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p74_gated_area_err[i+N]/p74_gated_area[i+N]*p583_gated_area[i+N]/p74_gated_area[i+N]*p74->GetArea()/p583->GetArea()/denom,2) + TMath::Power(p1280->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)/denom,2) + TMath::Power(p1937->GetAreaErr()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1937.)*((p74->GetArea()/p583->GetArea()*p583_gated_area[i+N]/p74_gated_area[i+N]) - (p1280->GetArea()/p583->GetArea()*eff->Eval(583.)/eff->Eval(1280.)))/TMath::Power(denom,2),2));

		std::cout << "The BR is = " << branch[i+N] << " +/- " << branch_err[i+N] << " With Hardy = " << branch_hardy[i+N] << " +/- " << branch_hardy_err[i+N] << std::endl;
	}

	auto* p74_time = new TGraphErrors(N, time, p74_gated_area, time_err, p74_gated_area_err); list.Add(p74_time);
	p74_time->SetName("p74_time"); p74_time->SetTitle("74 keV");
	p74_time->SetMarkerColor(2);
	p74_time->SetLineColor(2);
	auto* p583_time = new TGraphErrors(N, time, p583_gated_area, time_err, p583_gated_area_err); list.Add(p583_time);
	p583_time->SetName("p583_time"); p583_time->SetTitle("583 keV");
	p583_time->SetMarkerColor(4);
	p583_time->SetLineColor(4);
	auto* rel_effic_graph = new TGraphErrors(N, time, rel_effic, time_err, rel_effic_err); list.Add(rel_effic_graph);
	rel_effic_graph->SetName("rel_effic_graph"); rel_effic_graph->SetTitle("relative efficiency");

	auto* p74_sigma = new TGraphErrors(N, time, p74_gated_sigma, time_err, p74_gated_sigma_err); list.Add(p74_sigma);
	p74_sigma->SetName("p74_sigma"); p74_sigma->SetTitle("74 keV - #sigma");
	auto* p74_centr = new TGraphErrors(N, time, p74_gated_centr, time_err, p74_gated_centr_err); list.Add(p74_centr);
	p74_centr->SetName("p74_centr"); p74_centr->SetTitle("74 keV - centroid");
	auto* p74_r = new TGraphErrors(N, time, p74_gated_r, time_err, p74_gated_r_err); list.Add(p74_r);
	p74_r->SetName("p74_r"); p74_r->SetTitle("74 keV - R");
	auto* p583_sigma = new TGraphErrors(N, time, p583_gated_sigma, time_err, p583_gated_sigma_err); list.Add(p583_sigma);
	p583_sigma->SetName("p583_sigma"); p583_sigma->SetTitle("583 keV - #sigma");
	auto* p583_centr = new TGraphErrors(N, time, p583_gated_centr, time_err, p583_gated_centr_err); list.Add(p583_centr);
	p583_centr->SetName("p583_centr"); p583_centr->SetTitle("583 keV - centroid");
	auto* p583_r = new TGraphErrors(N, time, p583_gated_r, time_err, p583_gated_r_err); list.Add(p583_r);
	p583_r->SetName("p583_r"); p583_r->SetTitle("583 keV - R");

	auto* branch_time = new TGraphErrors(N, time, branch, time_err, branch_err); list.Add(branch_time);
	branch_time->SetName("branch_time"); branch_time->SetTitle("branching ratio");
	auto* branch_hardy_time = new TGraphErrors(N, time, branch_hardy, time_err, branch_hardy_err); list.Add(branch_hardy_time);
	branch_hardy_time->SetName("branch_hardy_time"); branch_hardy_time->SetTitle("branching ratio using Hardy");
	auto* branch_releff_time = new TGraphErrors(N, time, branch_releff, time_err, branch_releff_err); list.Add(branch_releff_time);
	branch_releff_time->SetName("branch_releff_time"); branch_releff_time->SetTitle("branching ratio using rel. eff.");


	auto* p74_neg_time = new TGraphErrors(N, time, &p74_gated_area[N], time_err, &p74_gated_area_err[N]); list.Add(p74_neg_time);
	p74_neg_time->SetName("p74_neg_time"); p74_neg_time->SetTitle("74 keV");
	p74_neg_time->SetMarkerColor(2);
	p74_neg_time->SetLineColor(2);
	auto* p583_neg_time = new TGraphErrors(N, time, &p583_gated_area[N], time_err, &p583_gated_area_err[N]); list.Add(p583_neg_time);
	p583_neg_time->SetName("p583_neg_time"); p583_neg_time->SetTitle("583 keV");
	p583_neg_time->SetMarkerColor(4);
	p583_neg_time->SetLineColor(4);
	auto* rel_effic_graph_neg = new TGraphErrors(N, time, &rel_effic[N], time_err, &rel_effic_err[N]); list.Add(rel_effic_graph_neg);
	rel_effic_graph_neg->SetName("rel_effic_graph_neg"); rel_effic_graph_neg->SetTitle("relative efficiency");

	auto* branch_neg_time = new TGraphErrors(N, time, &branch[N], time_err, &branch_err[N]); list.Add(branch_neg_time);
	branch_neg_time->SetName("branch_neg_time"); branch_neg_time->SetTitle("branching ratio");
	auto* branch_hardy_neg_time = new TGraphErrors(N, time, &branch_hardy[N], time_err, &branch_hardy_err[N]); list.Add(branch_hardy_neg_time);
	branch_hardy_neg_time->SetName("branch_hardy_neg_time"); branch_hardy_neg_time->SetTitle("branching ratio using Hardy");
	auto* branch_releff_neg_time = new TGraphErrors(N, time, &branch_releff[N], time_err, &branch_releff_err[N]); list.Add(branch_releff_neg_time);
	branch_releff_neg_time->SetName("branch_releff_neg_time"); branch_releff_neg_time->SetTitle("branching ratio using rel. eff.");

	std::cout<<"done, writing everything to "<<outputFileName<<std::endl;

	TFile output(outputFileName.c_str(), "recreate");
	output.cd();
	list.Write();
	output.Close();

	return 0;
}
