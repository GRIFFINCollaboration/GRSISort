//g++ GainMatchGRIFFIN.C -std=c++0x -I$GRSISYS/include -L$GRSISYS/libraries -lGRSIFormat -lAnalysisTreeBuilder -lGriffin -lSceptar -lDescant -lPaces -lGRSIDetector -lTigress -lSharc -lCSM -lTriFoil -lTGRSIint -lTGRSIFit -lGRSILoop -lMidasFormat -lGRSIRootIO -lDataParser -lMidasFormat -lXMLParser -lXMLIO -lProof -lGuiHtml `root-config --cflags --libs` -Wl,--no-as-needed -lTreePlayer -lSpectrum -lX11 -lXtst -lXpm -lGROOT -lTGRSIFit -o GainMatchGRIFFIN
#include<algorithm>
#include<vector>
#include<string>
#include<TFile.h>
#include<TH1.h>
#include<TH2.h>
#include<THStack.h>
#include<TSpectrum.h>
#include<TGraph.h>
#include<TGraphErrors.h>
#include<TMultiGraph.h>
#include<TF1.h>
#include<TCanvas.h>
#include<TList.h>
#include<TMinuit.h>
#include<TFragment.h>
#include<TGriffin.h>
#include<TGriffinHit.h>
#include<TChannel.h>
#include<TPeak.h>
#include<TBranch.h>
#include<TGaxis.h>
#include<TStyle.h>
#include<TLegend.h>
#include<TSystem.h>

enum class EType { kMaxBin, kTPeak, kTSpectrum };

void make_calibration_histograms(const char* fragFileName, const char* histFileName, int minchannel, int maxchannel, std::vector<int> channelstoskip, int bins, int xmin, int xmax);
void make_calibration_histograms(const char* fragFileName, const char* histFileName, int minchannel, int maxchannel, int bins, int xmin, int xmax);
TList* MakeGRIFFINChargeHsts(TTree* tree, int minchannel, int maxchannel, int bins, double xmin, double xmax);
TList* MakeGRIFFINChargeHsts(TTree* tree, int minchannel, int maxchannel, std::vector<int> channelstoskip, int bins, double xmin, double xmax);
void create_gainmatch_graphs(const char* histFileName, int minchannel, int maxchannel, double largepeak, std::vector<double> peaks, EType type, double mindistance, bool roughenergy, bool twopeaks, double largepeak2);
void create_gainmatch_graphs(const char* histFileName, int minchannel, int maxchannel, std::vector<int> channelstoskip, double largepeak, std::vector<double> peaks,EType type, double mindistance, bool roughenergy, bool twopeaks, double largepeak2);
TGraph* gainmatch_peaks(TH1* hst, std::vector<double> peakvalues, std::vector<double> peaklocations, double windowsize, EType type);
void create_GRIFFIN_cal(const char* ROOTFileName, const char* outFileName, int minchannel, int maxchannel, const char* paramImgName, const char* graphImgName, int order);
void create_GRIFFIN_cal(const char* ROOTFileName, const char* outFileName, int minchannel, int maxchannel, std::vector<int> channelstoskip, const char* paramImgName, const char* graphImgName, int order);
void recalibrate_spectra(const char* fragFile, const char* newFile, const char* calFile, int minchannel, int maxchannel, std::vector<int> channelstoskip, int xbins, double xmin, double xmax);
void recalibrate_spectra(const char* fragFile, const char* newFile, const char* calFile, int minchannel, int maxchannel, int xbins, double xmin, double xmax);
TList* MakeGRIFFINEnergyHsts(TTree* tree, int minchannel, int maxchannel, int bins, double xmin, double xmax, const char* calfile);
TList* MakeGRIFFINEnergyHsts(TTree* tree, int minchannel, int maxchannel, std::vector<int> channelstoskip, int bins, double xmin, double xmax, const char* calfile);
void check_calibration(const char* testFileName, int minchannel, int maxchannel, std::vector<int> channelstoskip, std::vector<double> peaks, EType type, double width, bool UseMyList, const char* fwhmImgName, const char* fwratioImgName);
void check_calibration(const char* testFileName, int minchannel, int maxchannel, EType type, double width, const char* fwhmImgName,const char* fwratioImgName);
void check_calibration(const char* testFileName, int minchannel, int maxchannel, std::vector<int> channelstoskip, EType type, double width, const char* fwhmImgName,const char* fwratioImgName);
void check_calibration(const char* testFileName, int minchannel, int maxchannel, std::vector<double> peaks, EType type, double width, bool UseMyList, const char* fwhmImgName, const char* fwratioImgName);
double FWXM(TH1* h, double xmin, double xmax, double percent);
double FWHM(TH1* h, double xmin, double xmax);
double FWTM(TH1* h, double xmin, double xmax);
double GetCentroidMaxBin(TH1* hst, double xmin, double xmax);
double GetCentroidTSpectrum(TH1* hst, double xmin, double xmax);
double GetCentroidTPeak(TH1* hst, double xmin, double xmax);
double GetCentroid(EType type, TH1* hst, double xmin, double xmax);
double GetRoughGain(TH1* h, double largepeak, double mindistance, bool twopeaks, double largepeak2);

// the idea in the future is to let the user choose the type of peak finder.
// right now though, we're just using TSpectrum
double GetCentroidMaxBin(TH1* hst, double xmin, double xmax)
{
	hst->GetXaxis()->SetRangeUser(xmin,xmax);
	int bin = hst->GetMaximumBin();
	double centroid = hst->GetBinCenter(bin);
	hst->GetXaxis()->UnZoom();
	return centroid;
}

double GetCentroidTSpectrum(TH1* hst, double xmin, double xmax)
{
	hst->GetXaxis()->SetRangeUser(xmin,xmax);
	TSpectrum* spec = new TSpectrum(1);
	spec->Search(hst);
	double centroid = spec->GetPositionX()[0];
	hst->GetXaxis()->UnZoom();
	return centroid;
}

double GetCentroidTPeak(TH1* hst, double xmin, double xmax)
{
	// for this to be robust, we have to be a bit smarter
	// particularly, we need the fit window to be smarter
	double guesscentroid = GetCentroidMaxBin(hst,xmin,xmax);
	double fwhm = FWHM(hst,xmin,xmax);

	// initial fit
	TPeak* peak;
	hst->GetXaxis()->SetRangeUser(xmin,xmax);
	if(guesscentroid-1.5*fwhm>xmin && guesscentroid+1.5*fwhm<xmax) {
		peak = new TPeak(guesscentroid,guesscentroid-1.5*fwhm,guesscentroid+1.5*fwhm);
	} else {
		peak = new TPeak(guesscentroid,xmin,xmax);
	}
	peak->Fit(hst,"Q");
	double centroid = peak->GetCentroid();
	hst->GetXaxis()->UnZoom();
	return centroid;
}

double GetCentroid(EType type, TH1* hst, double xmin, double xmax)
{
	double centroid = 0.;
	if(type == EType::kMaxBin) centroid = GetCentroidMaxBin(hst,xmin,xmax);
	else if(type == EType::kTSpectrum) centroid = GetCentroidTSpectrum(hst,xmin,xmax);
	else if(type == EType::kTPeak) centroid = GetCentroidTPeak(hst,xmin,xmax);
	return centroid;
}

double GetRoughGain(TH1* h, double largepeak, double mindistance, bool twopeaks=kFALSE, double largepeak2 = 0.0)
{
	TSpectrum* spec2 = new TSpectrum(2);

	// search for big keV peak (or peaks)
	spec2->Search(h);
	double posbigb;
	if(!twopeaks) {
		if(spec2->GetNPeaks()==2) {
			// if we get two peaks, check the height of the peaks to find the actual tallest one
			// using GetMaximum here instead of GetPositionY() because sometimes TSpectrum is weird.
			h->GetXaxis()->SetRangeUser(spec2->GetPositionX()[0]-mindistance,spec2->GetPositionX()[0]+mindistance);
			double mag0 = h->GetMaximum();
			h->GetXaxis()->SetRangeUser(spec2->GetPositionX()[1]-mindistance,spec2->GetPositionX()[1]+mindistance);
			double mag1 = h->GetMaximum();
			h->GetXaxis()->UnZoom();
			posbigb = spec2->GetPositionX()[0];
			if(mag1>mag0) posbigb = spec2->GetPositionX()[1];
		} else {
			posbigb = spec2->GetPositionX()[0];
		}
	} else {
		if(spec2->GetNPeaks()!=2) {
			posbigb = spec2->GetPositionX()[0];
			std::cout <<"Warning: Two peaks were specified, but not found. Setting only peak found to " <<largepeak <<std::endl;
		} else {
			if(std::min(largepeak,largepeak2) == largepeak) {
				posbigb = std::min(spec2->GetPositionX()[0],spec2->GetPositionX()[1]);
			} else {
				posbigb = std::max(spec2->GetPositionX()[0],spec2->GetPositionX()[1]);
			}
		}
	}
	// calculate rough gain
	double gain = posbigb/largepeak;
	return gain;
}

void make_calibration_histograms(const char* fragFileName, const char* histFileName, int minchannel, int maxchannel, std::vector<int> channelstoskip, int bins = 40e3, int xmin = 0, int xmax = 4000)
{
	// create histograms
	TFile* f = new TFile(fragFileName);
	if(f == nullptr) {
		printf("Failed to open file '%s'!\n",fragFileName);
		return;
	}
	if(!f->IsOpen()) {
		printf("Failed to open file '%s'!\n",fragFileName);
		return;
	}
	TTree* FragmentTree = (TTree*) f->Get("FragmentTree");
	TChannel::ReadCalFromTree(FragmentTree);
	TList* hstlist = MakeGRIFFINChargeHsts(FragmentTree,minchannel,maxchannel,channelstoskip,bins,xmin,xmax);
	TFile* outfile = new TFile(histFileName,"recreate");
	hstlist->Write();
	outfile->Close();
	f->Close();
}

void make_calibration_histograms(const char* fragFileName, const char* histFileName, int minchannel, int maxchannel, int bins = 40e3, int xmin = 0, int xmax = 4000)
{
	std::vector<int> dummy;
	make_calibration_histograms(fragFileName,histFileName,minchannel,maxchannel,dummy,bins,xmin,xmax);
}

TList* MakeGRIFFINChargeHsts(TTree* tree, int minchannel, int maxchannel, std::vector<int> channelsToSkip, int bins, double xmin, double xmax)
{
	// initialization stuff
	TList* list = new TList;
	TFragment *currentFrag = 0;
	TChannel::ReadCalFromTree(tree);
	TBranch *branch = tree->GetBranch("TFragment");
	branch->SetAddress(&currentFrag);

	// get the number of entries
	int fEntries = tree->GetEntries();

	// initialize histograms
	TH1F** hst = new TH1F*[maxchannel+1]; // min channel and max channel are inclusive
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		hst[i] = new TH1F(Form("hst%i",i),Form("hst%i",i),bins,xmin,xmax); list->Add(hst[i]);
	}

	std::cout <<"filling histograms." <<std::endl;
	// fill histograms
	for(int i=0;i<fEntries;i++) {
		tree->GetEntry(i);
		int chan = currentFrag->GetChannelNumber();
		int dettype = currentFrag->GetDetectorType();
		bool skipChannel=kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		if(chan>=minchannel && chan<=maxchannel && (dettype==0 || dettype==1)) {
			int charge = currentFrag->GetCharge();
			int kvalue = currentFrag->GetKValue();
			hst[chan]->Fill(double(charge)/kvalue); // this may not be (is probably not) the best way to do this...
			// suggestions welcome. -JKS
		}
	}

	std::cout <<"titling histograms." <<std::endl;
	// title histograms appropriately
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		TChannel* chan = TChannel::GetChannelByNumber(i);
		const char* name = chan->GetName();
		long int address = chan->GetAddress();
		hst[i]->SetTitle(Form("%s-0x%08x", name, static_cast<unsigned int>(address)));
	}

	return list;
}

TList* MakeGRIFFINChargeHsts(TTree* tree, int minchannel, int maxchannel, int bins, double xmin, double xmax)
{
	TList* list = MakeGRIFFINChargeHsts(tree, minchannel, maxchannel, std::vector<int>(), bins, xmin, xmax);
	return list;
}

void create_gainmatch_graphs(const char* histFileName, int minchannel, int maxchannel, std::vector<int> channelsToSkip, double largepeak, std::vector<double> peaks, EType type = EType::kTSpectrum, double mindistance=20e3, bool roughenergy=kTRUE, bool twopeaks=kFALSE, double largepeak2 = 0.0)
{
	// minchannel and max channel are inclusive values. most loops go from i=min to i<=maxchannel

	// largepeak is the value of largest peak in spectrum (in keV)

	// the values in the vector peaks do not have to be exact values, as the point here is not to run an energy calibration
	// advice: do not use peaks that have another peak near-by

	// mindistance should be in the same units as your original histogram (probably charge)
	// it is the window size for peak searching

	// if you want a rough enery match, your largepeak and peaks values need to be in keV
	// set roughenergy to true if you want a rough energy match as well

	/*---------------------------------------------------------------*/

	TFile* f = new TFile(histFileName,"update");
	if(f == nullptr) {
		printf("Failed to open file '%s'!\n",histFileName);
		return;
	}
	if(!f->IsOpen()) {
		printf("Failed to open file '%s'!\n",histFileName);
		return;
	}

	// fit histograms, create graphs, save slopes and offsets
	TGraph** g = new TGraph*[maxchannel+1];

	// create and save the graphs
	for(int i=minchannel;i<=maxchannel;i++) {
		std::cout <<"\t" <<i <<std::endl;
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		std::cout <<"Creating graph for channel " <<i <<std::endl;
		TH1F* h = (TH1F*) f->Get(Form("hst%i",i)); // grab histogram
		if(h==0) {
			std::cout <<"Error: hst" <<i <<" does not exist. Please check your calib_hst file and your inputs to GainMatchGRIFFIN." <<std::endl;
			continue;
		}

		// calculate rough gain
		double gain = GetRoughGain(h,largepeak,mindistance,twopeaks,largepeak2);
		//		std::cout <<"Rough gain for channel " <<i <<" is " <<gain <<std::endl;

		// calculate rough positions of peaks
		std::vector<double> peakguesses;
		for(auto peak : peaks) {
			peakguesses.push_back(gain*peak);
		}

		// gainmatch
		g[i] = gainmatch_peaks(h,peaks,peakguesses,mindistance,type);

		// check the first histogram and change the peak values if necessary
		if(i==minchannel) {
			if(roughenergy) g[i]->Fit("pol1","q","goff");
			for(int j=0;j<g[i]->GetN();j++) {
				double x,y;
				g[i]->GetPoint(j,x,y);
				if(!roughenergy) {
					peaks[j] = x;
					g[i]->SetPoint(j,x,x);
				} else {
					peaks[j] = g[i]->GetFunction("pol1")->Eval(x);
					g[i]->SetPoint(j,x,peaks[j]);
				}
			}
		}
		// format graph
		if((i)%4==1) g[i]->SetMarkerColor(kBlue);
		if((i)%4==2) g[i]->SetMarkerColor(kGreen);
		if((i)%4==3) g[i]->SetMarkerColor(kRed);
		if((i)%4==4) g[i]->SetMarkerColor(kBlack);
		g[i]->SetMarkerStyle(20+(i)/4);
		const char* hsttitle = h->GetTitle();
		g[i]->SetTitle(hsttitle);
		g[i]->SetName(Form("graph%i",i));
		g[i]->Write();

	}
	f->Close();
}

void create_gainmatch_graphs(const char* histFileName, int minchannel, int maxchannel, double largepeak, std::vector<double> peaks, EType type = EType::kTSpectrum, double mindistance=20e3, bool roughenergy=kTRUE, bool twopeaks=kFALSE, double largepeak2 = 0.0)
{
	std::vector<int> dummy;
	create_gainmatch_graphs(histFileName,minchannel,maxchannel,dummy,largepeak,peaks,type,mindistance,roughenergy,twopeaks,largepeak2);
	return;
}

TGraph* gainmatch_peaks(TH1* hst, std::vector<double> peakvalues, std::vector<double> peaklocations, double windowsize, EType type = EType::kTSpectrum)
{
	size_t n = peakvalues.size();
	if(peaklocations.size()!=n) {
		std::cout <<"Error: input vectors are not the same size." <<std::endl;
		return 0;
	}

	TGraph* graph = new TGraph();

	std::cout <<"Peaks for " <<hst->GetName() <<std::endl;
	for(size_t i=0;i<n;i++)	{
		std::cout<<i<<std::endl;
		double centroid = GetCentroid(type,hst,peaklocations[i]-windowsize,peaklocations[i]+windowsize);

		// set point in plot
		int npoints = graph->GetN();
		graph->SetPoint(npoints,centroid,peakvalues[i]);
	}

	return graph;
}

// i imagine a lot of this function could be rewritten with the TGainMatch class
void create_GRIFFIN_cal(const char* ROOTFileName, const char* outFileName, int minchannel, int maxchannel, std::vector<int> channelsToSkip, const char* paramImgName = "GRIFFIN_fitting_params.png", const char* graphImgName="GRIFFIN_calgraph.png", int order = 1)
{
	TFile* f = new TFile(ROOTFileName);
	if(f == nullptr) {
		printf("Failed to open file '%s'!\n",ROOTFileName);
		return;
	}
	if(!f->IsOpen()) {
		printf("Failed to open file '%s'!\n",ROOTFileName);
		return;
	}

	TGraph** g = new TGraph*[maxchannel+1];
	for(int i=minchannel;i<=maxchannel;i++) { // channel number
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		g[i] = (TGraph*) f->Get(Form("graph%i",i));
		if(g[i] == nullptr) {
			std::cout <<"Error: graph" <<i <<" does not exist. Please check your calib_hst file and your inputs to GainMatchGRIFFIN." <<std::endl;
			std::cout <<"Function create_GRIFFIN_cal will continue, but will skip this channel." <<std::endl;
			channelsToSkip.push_back(i);
			continue;
		}
	}

	// declare all sorts of variables
	std::vector<double> slopes(maxchannel+1);
	std::vector<double> offsets(maxchannel+1);
	std::vector<double> chi2(maxchannel+1);
	for(int i=0;i<=maxchannel;i++) chi2[i] = 0; // initialize array
	TGraphErrors* slopeg = new TGraphErrors();
	TGraphErrors* offsetg = new TGraphErrors();
	TGraphErrors* chi2g = new TGraphErrors();
	TMultiGraph* allgraphs = new TMultiGraph();
	std::vector<std::vector<double> > parameters(maxchannel+1, std::vector<double>(order+1));

	// fit graphs 
	for(int i=minchannel;i<=maxchannel;i++) {
		// check for skipped channel
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;

		// fit graphs
		g[i]->Fit(Form("pol%i",order),"q");

		// grab linear fitting parameters and chi^2
		for(int j=0;j<=order;j++) parameters[i][j] = g[i]->GetFunction(Form("pol%i",order))->GetParameter(j);
		slopes[i] = g[i]->GetFunction(Form("pol%i",order))->GetParameter(1); // grab slope
		offsets[i] = g[i]->GetFunction(Form("pol%i",order))->GetParameter(0); // grab offset
		int ndf = g[i]->GetFunction(Form("pol%i",order))->GetNDF();
		if(ndf!=0) chi2[i] = g[i]->GetFunction(Form("pol%i",order))->GetChisquare()/ndf; // grab offset
		else {
			chi2[i] = g[i]->GetFunction(Form("pol%i",order))->GetChisquare(); // grab offset
			if(chi2[i]==0) chi2[i] = 1e-24;
		}

		// fill TGraphErrors
		int point = slopeg->GetN();
		slopeg->SetPoint(point,i,slopes[i]);
		offsetg->SetPoint(point,i,offsets[i]);
		chi2g->SetPoint(point,i,chi2[i]);
		double error = g[i]->GetFunction(Form("pol%i",order))->GetParError(1);
		slopeg->SetPointError(point,0,error);
		error = g[i]->GetFunction(Form("pol%i",order))->GetParError(0);
		offsetg->SetPointError(point,0,error);
		g[i]->GetFunction(Form("pol%i",order))->Delete();

		// format graph and add to multigraph
		allgraphs->Add(g[i]); // add to multigraph
	}

	// draw graphs
	TCanvas* c1 = new TCanvas("c1","c1",800,600);
	allgraphs->Draw("ap");
	allgraphs->SetTitle("GRIFFIN calibration curves;ADC channel;Energy (keV)");
	allgraphs->GetXaxis()->CenterTitle(kTRUE);
	allgraphs->GetYaxis()->CenterTitle(kTRUE);
	c1->Print(graphImgName);

	// print slopes, offsets, chi2
	std::cout <<"Channel\tSlope\t\tOffset\t\tChi^2" <<std::endl;
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		if(chi2[i] == 0) continue;
		std::cout <<i <<"\t" <<slopes[i] <<"\t" <<offsets[i] <<"\t" <<chi2[i] <<std::endl;
	}

	// format histograms
	slopeg->SetTitle("Energy calibration slopes;Channel Number;Slope (keV/ADC channel)");
	offsetg->SetTitle("Energy calibration offsets;Channel Number;Offset (keV)");
	chi2g->SetTitle("Energy calibration #chi^{2}/NDF values;Channel Number;#chi^{2}/NDF value");
	slopeg->SetMarkerStyle(20);
	offsetg->SetMarkerStyle(20);
	chi2g->SetMarkerStyle(20);
	slopeg->GetXaxis()->SetNdivisions(505);
	slopeg->GetXaxis()->CenterTitle(kTRUE);
	slopeg->GetYaxis()->CenterTitle(kTRUE);
	offsetg->GetXaxis()->SetNdivisions(505);
	offsetg->GetXaxis()->CenterTitle(kTRUE);
	offsetg->GetYaxis()->CenterTitle(kTRUE);
	chi2g->GetXaxis()->SetNdivisions(505);
	chi2g->GetXaxis()->CenterTitle(kTRUE);
	chi2g->GetYaxis()->CenterTitle(kTRUE);

	// display slopes and offsets and chi2 values
	TGaxis::SetMaxDigits(3);
	TCanvas *c3 = new TCanvas("c3","c3",1200,400);
	c3->Divide(3,1);
	c3->cd(1);
	slopeg->Draw("pe1a");
	c3->cd(2);
	offsetg->Draw("pe1a");
	c3->cd(3);
	chi2g->Draw("pa");
	c3->Print(paramImgName);

	// create output file
	std::ofstream outFile;
	outFile.open(outFileName);
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		if(chi2[i] == 0) continue;

		// get name and address
		std::string nameaddress = f->Get(Form("graph%i",i))->GetTitle();
		std::string name = nameaddress.substr(0,10);
		std::string address = nameaddress.substr(11,10);

		// start printing out calibration file
		outFile <<name.c_str() <<"\t{" <<std::endl;
		outFile <<"Name:      " <<name.c_str() <<std::endl;
		outFile <<"Number:    " <<i <<std::endl;
		outFile <<"Address:   " <<address.c_str() <<std::endl;
		outFile <<"Digitizer: " <<std::endl;
		outFile <<"EngCoeff:  ";
		for(int j=0;j<=order;j++) outFile <<parameters[i][j] <<"\t";
		outFile <<std::endl;
		outFile <<std::endl;
		outFile <<"}" <<std::endl;
		outFile <<"//====================================//" <<std::endl;

	}
	outFile.close();	
}

void create_GRIFFIN_cal(const char* ROOTFileName, const char* outFileName, int minchannel, int maxchannel, const char* paramImgName = "GRIFFIN_fitting_params.png", const char* graphImgName="GRIFFIN_calgraph.png", int order = 1)
{
	std::vector<int> dummy;
	create_GRIFFIN_cal(ROOTFileName,outFileName,minchannel,maxchannel,dummy,paramImgName,graphImgName,order);
}

void recalibrate_spectra(const char* fragFile, const char* newFile, const char* calFile, int minchannel, int maxchannel, std::vector<int> channelsToSkip, int xbins = 40e3, double xmin = 0, double xmax = 4000)
{
	TFile* fdata = new TFile(fragFile);
	if(fdata == nullptr) {
		printf("Failed to open file '%s'!\n",fragFile);
		return;
	}
	if(!fdata->IsOpen()) {
		printf("Failed to open file '%s'!\n",fragFile);
		return;
	}

	TTree* FragmentTree = (TTree*) fdata->Get("FragmentTree");
	TChannel::ReadCalFile(calFile);
	TList *list = MakeGRIFFINEnergyHsts(FragmentTree,minchannel,maxchannel,channelsToSkip,xbins,xmin,xmax,"GRIFFINcal.cal");
	TFile* f = new TFile(newFile,"recreate");
	list->Write();
	f->Close();
	fdata->Close();
}

void recalibrate_spectra(const char* fragFile, const char* newFile, const char* calFile, int minchannel, int maxchannel, int xbins = 40e3, double xmin = 0, double xmax = 4000)
{
	std::vector<int> dummy;
	recalibrate_spectra(fragFile,newFile,calFile,minchannel,maxchannel,dummy,xbins,xmin,xmax);	
}

TList* MakeGRIFFINEnergyHsts(TTree* tree, int minchannel, int maxchannel, std::vector<int> channelsToSkip, int bins, double xmin, double xmax, const char* calfile)
{
	// initialization stuff
	TList* list = new TList;
	TFragment *currentFrag = 0;
	TChannel::ReadCalFile(calfile);
	TBranch *branch = tree->GetBranch("TFragment");
	branch->SetAddress(&currentFrag);

	// get the number of entries
	int fEntries = tree->GetEntries();

	// initialize histograms
	TH1F** hst = new TH1F*[maxchannel+1]; // min channel and max channel are inclusive
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel=kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		hst[i] = new TH1F(Form("hst%i",i),Form("hst%i",i),bins,xmin,xmax); list->Add(hst[i]);
	}

	std::cout <<"filling histograms." <<std::endl;
	// fill histograms
	for(int i=0;i<fEntries;i++) {
		tree->GetEntry(i);
		int chan = currentFrag->GetChannelNumber();
		bool skipChannel=kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		int dettype = currentFrag->GetDetectorType();
		if(chan>=minchannel && chan<=maxchannel && (dettype==0 || dettype==1)) {
			double energy = currentFrag->GetEnergy();
			hst[chan]->Fill(energy);
		}
	}

	std::cout <<"titling histograms." <<std::endl;
	// title histograms appropriately
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) skipChannel = kTRUE;
		}
		if(skipChannel) continue;
		if(hst[i]->Integral()==0) continue;
		TChannel* chan = TChannel::GetChannelByNumber(i);
		const char* name = chan->GetName();
		long int address = chan->GetAddress();
		hst[i]->SetTitle(Form("%s-0x%08x",name,static_cast<unsigned int>(address)));
	}
	return list;
}

TList* MakeGRIFFINEnergyHsts(TTree* tree, int minchannel, int maxchannel, int bins, double xmin, double xmax, const char* calfile)
{
	std::vector<int> dummy;
	TList* list = MakeGRIFFINEnergyHsts(tree,minchannel,maxchannel,dummy,bins,xmin,xmax,calfile);
	return list;
}

void check_calibration(const char* testFileName, int minchannel, int maxchannel, std::vector<int> channelsToSkip, std::vector<double> peaks, EType type = EType::kTSpectrum, double width = 10, bool UseMyList = kFALSE, const char* fwhmImgName="GRIFFIN_FWHM_diagnostic.png", const char* fwratioImgName="GRIFFIN_FWratio_diagnostic.png")
{
	TFile* f = new TFile(testFileName,"update");
	if(f == nullptr) {
		printf("Failed to open file '%s'!\n",testFileName);
		return;
	}
	if(!f->IsOpen()) {
		printf("Failed to open file '%s'!\n",testFileName);
		return;
	}

	TH1F** hsts = new TH1F*[maxchannel+1];
	TH1F* temphst = (TH1F*) gFile->Get(Form("hst%i",minchannel));
	TH1F* hstall = (TH1F*) temphst->Clone("hstall");

	// declare 2D histograms for use
	// energy along the x-axis, fwhm along the y-axis
	double elow = temphst->GetBinLowEdge(1);
	int bins = temphst->GetXaxis()->GetNbins();
	double ehigh = temphst->GetBinLowEdge(bins+1);

	TH2F* fwhm = new TH2F("fwhm","fwhm",400,elow,ehigh,200,0,width);
	TH2F* fwhm_all = new TH2F("fwhm_all","fwhm_all",400,elow,ehigh,200,0,width);

	TH2F* fwratio = new TH2F("fwratio","fwratio",400,elow,ehigh,200,0,5);
	TH2F* fwratio_all = new TH2F("fwratio_all","fwratio_all",400,elow,ehigh,200,0,5);

	if(!UseMyList) { // generate vector of peaks
		peaks.clear();
		TSpectrum* spec = new TSpectrum();
		int n = spec->Search(temphst);
		for(int i=0;i<n;i++) {
			peaks.push_back(spec->GetPositionX()[i]);
			std::cout <<peaks[i] <<std::endl;
		}
	}

	gStyle->SetOptStat(0);
	THStack* stack = new THStack("stack","All histograms");

	// create hstall
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) {
				skipChannel = kTRUE;
				break;
			}
		}
		if(skipChannel) continue;
		hsts[i] = (TH1F*) gFile->Get(Form("hst%i",i));
		if(hsts[i]==0) {
			std::cout <<"Error: hst" <<i <<" does not exist. Please check your newcal and calib_hst files and your inputs to GainMatchGRIFFIN." <<std::endl;
			std::cout <<"Function check_calibration will continue, but will skip this channel." <<std::endl;
			channelsToSkip.push_back(i);
			continue;
		}
		if(i!=minchannel) hstall->Add(hsts[i]);
	}

	// get peak values from summed spectrum for residual calculation
	std::vector<double> summedpeaks;
	for(auto peak : peaks) {
		double centroid = GetCentroid(type,hstall,peak-width,peak+width);
		summedpeaks.push_back(centroid);
	}	

	// formatting, FWHM, residuals, etc.
	TGraph** residuals = new TGraph*[maxchannel+1];
	for(int i=minchannel;i<=maxchannel;i++) {
		bool skipChannel = kFALSE;
		for(auto skip : channelsToSkip) {
			if(i == skip) {
				skipChannel = kTRUE;
				break;
			}
		}
		if(skipChannel) continue;
		hsts[i]->SetLineColor(i+1);
		stack->Add(hsts[i]);
		residuals[i] = new TGraph(peaks.size());
		for(size_t j=0;j<peaks.size();j++) {
			// calculate FWHM and FWTM
			double myfwhm = FWHM(hsts[i],peaks[j]-width,peaks[j]+width);
			if(myfwhm!=0.0) fwhm->Fill(peaks[j],myfwhm);
			double myfwtm = FWTM(hsts[i],peaks[j]-width,peaks[j]+width);
			if(myfwtm!=0.0 && myfwhm!=0.0) fwratio->Fill(peaks[j],myfwtm/myfwhm);

			// calculate residuals
			double centroid = GetCentroid(type,hsts[i],peaks[j]-width,peaks[j]+width);
			//			hsts[i]->GetXaxis()->SetRangeUser(peaks[j]-width,peaks[j]+width);
			//			int bin = hsts[i]->GetMaximumBin();
			//			residuals[i]->SetPoint(j,summedpeaks[j],hsts[i]->GetBinCenter(bin)-summedpeaks[j]);
			//			hsts[i]->GetXaxis()->UnZoom();
			residuals[i]->SetPoint(j,summedpeaks[j],centroid-summedpeaks[j]);
		}
		residuals[i]->SetName(Form("residual%i",i));
		residuals[i]->SetTitle(hsts[i]->GetTitle());
		residuals[i]->SetMarkerStyle(4);
		residuals[i]->Write();
	}
	//stack->Write();
	hstall->Write();

	// fill 2D histograms for summed spectrum
	for(auto peak : peaks) {
		double myfwhm = FWHM(hstall,peak-width,peak+width);
		if(myfwhm!=0.0) fwhm_all->Fill(peak,myfwhm);
		double myfwtm = FWTM(hstall,peak-width,peak+width);
		if(myfwtm!=0.0 && myfwhm!=0.0) fwratio_all->Fill(peak,myfwtm/myfwhm);
	}

	// save histograms to file
	fwhm->Write();
	fwhm_all->Write();
	fwratio->Write();
	fwratio_all->Write();

	// draw fwhm results
	TCanvas* c_fwhm = new TCanvas("c_fwhm","FWHM comparison, crystals and summed spectrum", 600,400);
	fwhm->SetTitle("FWHM values of individual crystals and summed spectrum;Energy (keV);FWHM (keV)");
	fwhm->SetMarkerStyle(4);
	fwhm->Draw("candle");
	fwhm_all->SetMarkerStyle(30);
	fwhm_all->Draw("same");
	TLegend* legend = new TLegend(0.6,0.6,0.88,0.85);
	legend->SetFillColor(kWhite);
	legend->AddEntry(fwhm,"Single crystal mean","p");
	legend->AddEntry(fwhm_all,"Summed","p");
	legend->Draw();
	c_fwhm->SaveAs(fwhmImgName);

	// draw fwhm results
	TCanvas* c_fwratio = new TCanvas("c_fwratio","FWTM:FWHM, crystals and summed spectrum", 600,400);
	fwratio->SetTitle("FWTM:FWHM values of individual crystals and summed spectrum;Energy (keV);FWTM/FWHM");
	fwratio->Draw("candle");
	fwratio_all->SetMarkerStyle(30);
	fwratio_all->Draw("same");
	legend->Draw();
	c_fwratio->SaveAs(fwratioImgName);

	f->Close();
}

void check_calibration(const char* testFileName, int minchannel, int maxchannel, EType type = EType::kTSpectrum, double width = 10, const char* fwhmImgName="GRIFFIN_FWHM_diagnostic.png",const char* fwratioImgName="GRIFFIN_FWratio_diagnostic.png")
{
	std::vector<double> peaks;
	std::vector<int> dummy;
	check_calibration(testFileName,minchannel,maxchannel,dummy,peaks,type,width,kFALSE,fwhmImgName,fwratioImgName);
}

void check_calibration(const char* testFileName, int minchannel, int maxchannel, std::vector<int> channelsToSkip, EType type = EType::kTSpectrum, double width = 10, const char* fwhmImgName="GRIFFIN_FWHM_diagnostic.png",const char* fwratioImgName="GRIFFIN_FWratio_diagnostic.png")
{
	std::vector<double> peaks;
	check_calibration(testFileName,minchannel,maxchannel,channelsToSkip,peaks,type,width,kFALSE,fwhmImgName,fwratioImgName);
}

void check_calibration(const char* testFileName, int minchannel, int maxchannel, std::vector<double> peaks, EType type = EType::kTSpectrum, double width = 10, bool UseMyList = kFALSE, const char* fwhmImgName="GRIFFIN_FWHM_diagnostic.png", const char* fwratioImgName="GRIFFIN_FWratio_diagnostic.png")
{
	std::vector<int> dummy;
	check_calibration(testFileName,minchannel,maxchannel,dummy,peaks,type,width,UseMyList,fwhmImgName,fwratioImgName);
}

// this function calculates the full-width at X maximum, where x is given as a percent.
double FWXM(TH1* h, double xmin, double xmax, double percent = 0.5)
{
	h->GetXaxis()->SetRangeUser(xmin,xmax);
	double max = h->GetMaximum();
	int maxbin = h->GetMaximumBin();
	double bincontent = max;
	int binbelow = maxbin;
	double low,high;

	// rough estimation of background
	int lowbin = h->FindBin(xmin);
	int highbin = h->FindBin(xmax);
	double bg = (h->GetBinContent(highbin)+h->GetBinContent(lowbin))/2.;

	// find lower limit for FWHM
	while(bincontent>(max-bg)*percent+bg) {
		--binbelow;
		if(binbelow<lowbin) {
			std::cout <<"Couldn't find good lower limit, returning 0." <<std::endl;
			return 0.0;
		}
		bincontent = h->GetBinContent(binbelow);
	}
	double x1,y1,x2,y2,y3,m;
	y3 = (max-bg)*percent+bg;
	x1 = h->GetBinCenter(binbelow);
	x2 = h->GetBinCenter(binbelow+1);
	y1 = h->GetBinContent(binbelow);
	y2 = h->GetBinContent(binbelow+1);
	m = (y2-y1)/(x2-x1);
	low = (y3-y1)/m+x1;

	binbelow = maxbin;
	bincontent = max;
	// find upper limit for FWHM
	while(bincontent>(max-bg)*percent+bg) {
		++binbelow;
		if(binbelow>highbin) {
			std::cout <<"Couldn't find good upper limit, returning 0." <<std::endl;
			return 0.0;
		}
		bincontent = h->GetBinContent(binbelow);
	}
	y3 = max/2.;
	x1 = h->GetBinCenter(binbelow);
	x2 = h->GetBinCenter(binbelow-1);
	y1 = h->GetBinContent(binbelow);
	y2 = h->GetBinContent(binbelow-1);
	m = (y2-y1)/(x2-x1);
	high = (y3-y1)/m+x1;

	h->GetXaxis()->UnZoom();

	return (high-low);
}

double FWHM(TH1* h, double xmin, double xmax)
{
	return FWXM(h,xmin,xmax,0.5);
}

double FWTM(TH1* h, double xmin, double xmax)
{
	return FWXM(h,xmin,xmax,0.1);
}

#ifndef __CINT__ 

int main(int argc, char **argv) {
	// usage: <fragment tree file> <cal file name (optional)>
	if(argc < 2) {
		printf("try again (usage: %s <fragment tree files>).\n",argv[0]);
		return 0;
	}
	//-------------- VARIABLE DECLARATION SECTION --------------//
	// minimum and maximum channel for gain-matching
	// these are inclusive
	int minchannel = 1;
	int maxchannel = 65;
	std::vector<int> channelsToSkip;
	channelsToSkip.push_back(49);
	channelsToSkip.push_back(50);
	channelsToSkip.push_back(51);
	channelsToSkip.push_back(52);
	channelsToSkip.push_back(53);

	// charge spectrum definitions
	int xbins_charge = 10e3;
	int xmin_charge = 0;
	int xmax_charge = 4000;

	// fitting specifications
	bool roughenergy = kTRUE;
	double largepeak = 586;
	std::vector<double> fittingpeaks;
	fittingpeaks.push_back(2013);
	fittingpeaks.push_back(586);
	fittingpeaks.push_back(2577);
	EType type = EType::kTPeak; // kMaxBin, kTSpectrum, and kTPeak in increasing order of accuracy and decreasing order of stability/reliability
	double mindistance = 40;

	// if there are TWO peaks of large and almost equal magnitude (i.e. 60Co)
	// set twopeaks to kTRUE and assign largepeak2 the value of the second peak
	bool twopeaks = kFALSE;
	double largepeak2 = 1332;

	// calibration file name
	const char* calFile = "GRIFFINcal.cal";

	// energy spectrum definitions
	int xbins_energy = 40e3;
	int xmin_energy = 0;
	int xmax_energy = 4000;

	// evaluate parameters
	// peakstocheck does not have to have the same peaks as fitting peaks
	std::vector<double> peakstocheck;
	peakstocheck.push_back(2013);
	peakstocheck.push_back(586);
	peakstocheck.push_back(566);
	peakstocheck.push_back(2577);
	double windowsize = 10; // the +/- quantity for checking to see if peaks exist

	// turn the different sections of the code on and off
	bool do_make_calibration_histograms = kFALSE;
	bool do_create_gainmatch_graphs = kTRUE;
	bool do_create_GRIFFIN_cal = kTRUE;
	bool do_recalibrate_spectra = kTRUE;
	bool do_check_calibration = kTRUE;

	//-------------- END OF VARIABLE DECLARATION SECTION --------------//

	// filename strings
	std::string fileName, calibfileName, newfileName;
	char *original_name, *calib_hst_name, *test_file_name;

	// here, we allow a loop over multiple fragment files
	for(int i=1;i<argc;i++) {
		fileName = argv[i];
		calibfileName = argv[i];
		if(calibfileName.find_last_of("/") != std::string::npos) {
			calibfileName.insert(calibfileName.find_last_of("/")+1,"calib_hsts_");
		} else {
			calibfileName.insert(0,"calib_hsts_");
		}

		TFile* file = new TFile(argv[i]);
		if(file == nullptr) {
			printf("Failed to open file '%s'!\n",argv[1]);
			return 1;
		}
		if(!file->IsOpen()) {
			printf("Failed to open file '%s'!\n",argv[1]);
			return 1;
		}
		file->Close();

		// input fragment file name. create calib_hst filename from that.
		original_name = argv[i];
		calib_hst_name = new char[calibfileName.length()+1];
		strcpy(calib_hst_name,calibfileName.c_str());

		// this function creates histograms that will be used for calibration
		if(do_make_calibration_histograms) {
			make_calibration_histograms(original_name,calib_hst_name,minchannel,maxchannel,channelsToSkip,xbins_charge,xmin_charge,xmax_charge);
		}
	}

	if(argc>2) {
		// this hard-coded limit is VERY BAD! it should be changed at a later date.
		char buffer[500];
		sprintf(buffer,"hadd calib_hsts_summed.root ");
		for(int i=1;i<argc;i++) {
			calibfileName = argv[i];
			if(calibfileName.find_last_of("/") != std::string::npos) {
				calibfileName.insert(calibfileName.find_last_of("/")+1,"calib_hsts_");
			} else {
				calibfileName.insert(0,"calib_hsts_");
			}
			sprintf(buffer,"%s %s",buffer,calibfileName.c_str());
		}
		calibfileName = "calib_hsts_summed.root";
		calib_hst_name = new char[calibfileName.length()+1];
		strcpy(calib_hst_name,calibfileName.c_str());
		// if we don't want to make the histograms again, we probably also don't want to re-sum them and delete the graphs
		if(do_make_calibration_histograms) {
			gSystem->Exec(Form("rm %s",calibfileName.c_str()));
			gSystem->Exec(buffer);
		}
	}

	// this function creates graphs of uncalibrated peak value vs. calibrated peak value that will be used for fitting
	if(do_create_gainmatch_graphs) {
		create_gainmatch_graphs(calib_hst_name,minchannel,maxchannel,channelsToSkip,largepeak,fittingpeaks,type,mindistance,roughenergy,twopeaks,largepeak2);
	}
	// this function fits those graphs with lines and puts those values into a calibration file
	if(do_create_GRIFFIN_cal) {
		create_GRIFFIN_cal(calib_hst_name,calFile,minchannel,maxchannel,channelsToSkip);
	}

	for(int i=1;i<argc;i++) {
		fileName = argv[i];
		newfileName = argv[i];
		if(newfileName.find_last_of("/") != std::string::npos) {
			newfileName.insert(newfileName.find_last_of("/")+1,"newcal_");
		} else {
			newfileName.insert(0,"newcal_");
		}

		TFile* file = new TFile(argv[i]);
		if(file == nullptr) {
			printf("Failed to open file '%s'!\n",argv[1]);
			return 1;
		}
		if(!file->IsOpen()) {
			printf("Failed to open file '%s'!\n",argv[1]);
			return 1;
		}
		file->Close();

		// input fragment file name. create calib_hst filename from that.
		original_name = argv[i];
		test_file_name = new char[newfileName.length()+1];
		strcpy(test_file_name,newfileName.c_str());

		// this function takes the original data set, applys the calibration, and makes new histograms
		if(do_recalibrate_spectra) {
			recalibrate_spectra(original_name,test_file_name,calFile,minchannel,maxchannel,channelsToSkip,xbins_energy,xmin_energy,xmax_energy);
		}
	}

	if(argc>2) {
		// this hard-coded limit is VERY BAD! it should be changed at a later date.
		char buffer[500];
		sprintf(buffer,"hadd newcal_summed.root ");
		for(int i=1;i<argc;i++) {
			newfileName = argv[i];
			if(newfileName.find_last_of("/") != std::string::npos) {
				newfileName.insert(newfileName.find_last_of("/")+1,"newcal_");
			} else {
				newfileName.insert(0,"newcal_");
			}
			sprintf(buffer,"%s %s",buffer,newfileName.c_str());
		}
		newfileName = "newcal_summed.root";
		test_file_name = new char[newfileName.length()+1];
		strcpy(test_file_name,newfileName.c_str());
		// if we don't want to make recalibrate, we probably also don't want to re-sum them.
		if(do_recalibrate_spectra) {
			gSystem->Exec(Form("rm %s",newfileName.c_str()));
			gSystem->Exec(buffer);
		}
	}

	// this function outputs diagnostic images for evaluating the quality of the gain-matching
	if(do_check_calibration) {
		check_calibration(test_file_name,minchannel,maxchannel,channelsToSkip,peakstocheck,type,windowsize,kTRUE);
	}

	return 0;
}

#endif
