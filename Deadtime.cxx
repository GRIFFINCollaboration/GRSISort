//To compile:
//Note: GRSISort looks for .cxx extensions when compiling (for example it looks in the myAnalysis directory)
//Alternatively you may use the following to compile:
//g++ myanalysis.cxx -o myanalysis -std=c++0x -I$GRSISYS/GRSISort/include/ `grsi-config --cflags --all-libs --root`

#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <functional>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>
#include <stdio.h>
#include <math.h>       /* round, floor, ceil, trunc */
#include <time.h>
using namespace std;

#include "TF1.h"
#include "TMath.h"
#include "TH1.h"
#include "TH1F.h"
#include "THStack.h"
#include "TString.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TFile.h"
#include "TFileIter.h"
#include "TKey.h"
#include "TTree.h"
#include "TH2F.h"
#include "TROOT.h"
#include "TPPG.h"
#include "TScaler.h"
#include "TApplication.h"
#include "TLeaf.h"

#ifndef __CINT__ 
#include "TGriffin.h"
#include "TSceptar.h"
#endif

void 
Printaddress(int *channel);
void
MakeSpectra(const char*& filename, int& prog, const char*& fname, int& nsclr, int& ncycle, double *rate, int *channel, int& index, int* trun, double& thresh);
void
CheckFile(const char*& fname);
void 
DoAnalysis(const char*& fname, int& nfile, double *rate, int& nsclr, int& patlen, int& ncycle, int *trun, double& eor, const char*& hname, const char*& iname, const char*& jname, const char*& kname, const char*& lname, const char*& mname, const char*& nname, int& nscaler);

int main(int argc, char* argv[]) {
    	TApplication theApp("Analysis", &argc, argv);
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~USER INPUTS
	std::ifstream filelist("/home/mbowry/Documents/ROOT/Backup/newfilelist_hs.txt");	//input file(s) DATA
	std::ifstream ODBlist("/home/mbowry/Documents/ROOT/Backup/ODBlist_hs.txt");		//input file(s) ODB (runinfo)
	const char* fname = "viewscaler.root";							//output file (scaler spectra)
	const char* hname = "random_counts.txt"; 						//output file (frequency histograms, source)
	const char* iname = "combined_counts.txt"; 						//output file (frequency histograms, source+pulser)
	const char* jname = "random_seed.txt"; 							//output file (check random number generation)
	const char* kname = "RESULTS.txt";							//output file (deadtime results)
	const char* lname = "asymmetric_error.txt";						//output file (error combination histogram)
	const char* mname = "random_deadtime.txt";						//output file (random deadtime histogram)
	const char* nname = "accepted_rand.txt";						//output file (accepted random rate histogram)
	int nsclr = 9;										//total number of pulser inputs
	int addr[9]={0x0000,0x000d,0x0101,0x0107,0x020b,0x020c,0x0302,0x030c,0x030d};		//addresses with pulser inputs
	double freq[4]={2.e3,5.e3,1.e4,2.e4};							//precision pulser rates (2,5,10,20 kHz)
	int patlen=10;										//pattern length in seconds
	int ncycle=1;										//read out period of scalers (seconds);
	int scaleri = 0;									//scaler# START 
	int scalerf = 3;									//scaler# END (0->3 = all scalers)
	double thresh=0.3;									//threshold for rejection of spurious scaler events
	double eor=0.3;										//threshold for end-of-run cut off
	int specoff = 1;									//temporary flag: if =1, only analysis performed
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~DEFINITIONS
	int tdiff[4]={0,0,0,0};									//run time extracted from ODB
	int *td=&tdiff[0];
	int *p=&addr[0];
	double *q=&freq[0];
	int counter=0;
	int nds=0; 
	int nscaler=0;
	int len = 70;							
  	char line[len];
	char odb[len];	
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
	if (not (filelist.is_open())) {	
    		std::cerr << "Failed to open filelist. Check path. " << std::endl;
    		exit(EXIT_FAILURE);
  	} else if (not (ODBlist.is_open())) {
		std::cerr << "Failed to open ODBlist. Check path. " << std::endl;
    		exit(EXIT_FAILURE);
	}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*     
	//in the ODB file there are the lines:
		//Start time binary = DWORD : 1445453916
		//Stop time binary = DWORD : 1445454042
		//the difference (stop-start) equals the run time in seconds. Useful for histogram binning purposes.
	size_t posa; size_t posb; int sub=28;
	const char* starttime = "Start time binary";
	const char* stoptime = "Stop time binary";
	int tstart; int tend; int runtime; int nppg;
	string odbline;
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*	
	int line_count = std::count(								//read in number of lines(files)
        std::istreambuf_iterator<char>(filelist),
        std::istreambuf_iterator<char>(), '\n');
   	printf(DMAGENTA "-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/" RESET_COLOR "\n");
   	printf(DGREEN "Started Deadtime v1 / 04-Mar-2016 / mbowry@triumf.ca" RESET_COLOR "\n");
   	printf(DBLUE "Calculates deadtime on a channel-by-channel basis using precision scaler data" RESET_COLOR "\n");
   	printf(DBLUE "Sub-runs must be merged (e.g. using gadd) before running this program" RESET_COLOR "\n");
   	printf(DBLUE "A list of fragments (run files) and their corresponding ODB files must be provided" RESET_COLOR "\n");
	printf(DGREEN "Number of files loaded: %i" RESET_COLOR "\n",line_count);
	printf(DBLUE "Spectra are saved in %s" RESET_COLOR "\n",fname);
	printf(DBLUE "Deadtime results are recorded in %s" RESET_COLOR "\n",kname);
   	printf(DMAGENTA "-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/" RESET_COLOR "\n");
	printf(DBLUE "Working, be patient .. " RESET_COLOR "\n");
	printf("\n");
	filelist.clear();									
	filelist.seekg(0, ios::beg);
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
	Printaddress(p);									//Credit to to E.Kwan for this part
	p=&addr[0];
	while(counter<line_count){									
		filelist.getline(line, len, '\n');
		const char* filename = line;
		//-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/ODB STUFF
		ODBlist.getline(odb, len, '\n');	//<retreive ODB file name ('odb' assigned here)	
		std::ifstream InFile(odb);		//<set the I/O stream to the current file
		while(InFile.good()){			
      		getline(InFile,odbline);
      		posa=odbline.find(starttime);
		posb=odbline.find(stoptime);
      			if(posa!=string::npos && odbline.size()>sub){
				odbline = odbline.substr(sub);			
				tstart = std::stoi (odbline,nullptr,10);
        		} else if(posb!=string::npos && odbline.size()>(sub-1)){
				odbline = odbline.substr((sub-1));			
				tend = std::stoi (odbline,nullptr,10);
        		}
  		}
		runtime=tend-tstart;
		nppg=floor(runtime/patlen);
		tdiff[counter]=runtime;
		std::cout << "\n";
		if(runtime<=600){
			printf(DBLUE "Read ODB %s : run time = %i seconds / %i transitions" RESET_COLOR "\n",odb,runtime,nppg);
		} else if(runtime>600){
			printf(DBLUE "Read ODB %s : run time = %i minutes / %i transitions" RESET_COLOR "\n",odb,(runtime/60),nppg);
		}
		//-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-			
		for(int index=scaleri;index<(scalerf+1);index++){
			if(specoff==1){
				printf(DMAGENTA "Creation of histograms suppressed .. performing analysis step only." RESET_COLOR "\n");
			} else {
				MakeSpectra(filename, counter, fname, nsclr, ncycle, q, p, index, td, thresh);
			}
			p=&addr[0];
			nds+=1;
		}
		counter++;
		*q++;
		*td++;
		tstart=tend=0;
	}
	q=&freq[0];
	td=&tdiff[0];
	nscaler=nds/counter;
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
	CheckFile(fname);									
	DoAnalysis(fname, line_count, q, nsclr, patlen, ncycle, td, eor, hname, iname, jname, kname, lname, mname, nname, nscaler);
	printf(DBLUE "Spectra are saved in %s" RESET_COLOR "\n",fname);	
	printf(DBLUE "Deadtime results are %s" RESET_COLOR "\n",kname);
	printf(DBLUE "Done. Control-c to exit." RESET_COLOR "\n");
	theApp.Run(kTRUE);	

  	return EXIT_SUCCESS;
}
void Printaddress(int *channel){
	   printf(DBLUE "Addresses with both source and pulser inputs:" RESET_COLOR "\n");
    	for(int i=0;i<10;++i)     {
       	printf(DBLUE "%04x " RESET_COLOR ,*channel);
       	*channel++;
     	}
	printf("\n");
}
void
MakeSpectra(const char*& filename, int& prog, const char*& fname, int& nsclr, int& ncycle, double *rate, int *channel, int& index, int* trun, double& thresh){

  int nsc = nsclr;

  //define spectra
  TH1D *grif[nsc];
  //define file pointer
  TFile *vs;

  //make spectra
    TFile *rf = new TFile(filename,"read");
    TTree *maple = (TTree*)rf->Get("ScalerTree");							//Scaler data

    int nofBins = *trun/ncycle;
    double xaxis = 0; double yaxis = 0; double prev = 0; double xpast=0;
    int j = 0; int k = 0; double clk = 20e-9;	//2 clock tics (20ns)
    
    while (j<nsc) {
    grif[j] = new TH1D(Form("grif%d_0x%04x_%d",prog,*channel,index),Form("Address 0x%04x, scaler %i vs time in cycle; time [s]; counts/%d s",*channel,index,ncycle),nofBins,0.,*trun);
	j++;	
	*channel++;
	}

   TScalerData* scaler = 0;
   TScaler(maple).Clear();
   maple->SetBranchAddress("TScalerData", &scaler); 	
   Long64_t nentries = maple->GetEntries();

    //*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*
    //Build histograms directly from trees
    //*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*
    	if (prog==0 && index==0) {	
		vs = new TFile(fname,"recreate");
	} else {
		vs = new TFile(fname,"update");
	}
    //*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*
	for(int i=0;i<nsc;i++){
		*channel--;
	}
		while (k<nsc){
			for (Long64_t j=0;j<nentries;j++) {
			maple->GetEntry(j);
				if(scaler->GetAddress()==*channel){
				xaxis = (scaler->GetTimeStamp()/1e8);		
					//we check both the value of the scaler and the timestamp (ts difference should be = readout time)
					if(prev != 0 && prev < scaler->GetScaler(index) && (xaxis-xpast)<=(double(ncycle)+clk)) {
						yaxis = (scaler->GetScaler(index)-prev);
						grif[k]->Fill(xaxis,yaxis);
					}
				prev = scaler->GetScaler(index);
				xpast=xaxis;
				}
			}
		k++;
		*channel++;
		prev=xpast=0;
		}
	
    printf(DGREEN "Created histograms for scaler %i : file = %s" RESET_COLOR "\n",index,filename);
    //write hists
    for (int i=0; i<nsc; i++) {
    	grif[i]->Write();
    }	
    vs->Close();
    //*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*
  return;
}
void
CheckFile(const char*& fname){
  TFile f(fname);
  if (f.IsZombie()) {
  printf("\n");
  printf(DRED "Error opening ROOT file %s" RESET_COLOR "\n",fname);
  exit(-1);
  } else {
  printf("\n");
  printf(DBLUE "ROOT file %s opens with no problems.." RESET_COLOR "\n",fname);
  }
return;
}
void
DoAnalysis(const char*& fname, int& nfile, double *rate, int& nsclr, int& patlen, int& ncycle, int *trun, double& eor, const char*& hname, const char*& iname, const char*& jname, const char*& kname, const char*& lname, const char*& mname, const char*& nname, int& nscaler){
  
  TFile *vs = new TFile(fname,"read");
  ofstream ofile;
  ofile.open("diagnostic.txt");
  FILE *random = fopen(hname,"w");
  FILE *combine = fopen(iname,"w");
  FILE *rng = fopen(jname,"w");
  FILE *deadtime = fopen(kname,"w");
  FILE *asymerror = fopen(lname,"w");
  FILE *randdt = fopen(mname,"w");
  FILE *randw = fopen(nname,"w");
  ofile.precision(4);

  int nsc = nsclr;
  int cnt = 0;
  int ppgstat[2]={0,0};
  //generate random seed from system time for use in error analysis
  time_t timer;
  timer = time(NULL);
  srand (timer);	

  TFile f(fname);
  TIter next(f.GetListOfKeys());
  TKey *key;
  TH1D *spec[nfile*(nsc*nscaler)];
 
  //*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~USER EDITABLE
  //limits for deadtime matrices [us]
  //NOTE: The order of the limits (rp1,rp2 etc.) MUST be identical to the file order
  double rp1[8]={-15,15,-15,15,0,20,85,115}; 					//2kHz, scaler0-3
  double rp2[8]={-15,15,-15,15,0,20,85,115}; 					//5kHz, scaler0-3
  double rp3[8]={-15,15,-15,15,0,20,170,230}; 					//10kHz, scaler0-3
  double rp4[8]={-15,15,-15,15,0,20,200,300}; 					//20kHz, scaler0-3
  double *lowrtau=&rp1[0];		//internal pointers: points to correction coefficients in each array
  double *upprtau=&rp1[1];		//
  int cflag=0;				//counter
  //*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

  std::cout<< "\n";
  //output headers
  fprintf(deadtime,"#np=pulser,nr=source,nrp=source+pulser,tau=deadtime,erb=FWHM est. err,erf=full range err,erp=standard err propagation");
  fprintf(deadtime,"\n");
  fprintf(deadtime,"%s\t\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s","#spec","chan","np","nr","+/-","nrp","+","-","tau","+erb","-erb","+erf","-erf","+/-erp");
  fprintf(deadtime,"\n");
  fprintf(deadtime,"%s\t\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s","#","-","kHz","kHz","kHz","kHz","kHz","kHz","us","us","us","us","us","us"); 
  fprintf(deadtime,"\n");
  fprintf(randdt, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s","#rc","rr","rc-rr","rtau","lim1","lim2","flag","err(rc-rr)");
  fprintf(randdt,"\n");

	while ((key=(TKey*)next())) {
	int numpat = floor(*trun/patlen);	//minimum number of expected transitions based on run time
	const char* sname = key->GetName();
	spec[cnt] = (TH1D*)vs->Get(sname);
	
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~*~*~*variables
	double wcomb = 0; double ncomb = 0; double rcomb = 0; double sdcomb = 0;
	double wrand = 0; double nrand = 0; double rrand = 0; double sdrand = 0; double sum = 0, sumc = 0;
	double maxl = 0; double minl = 1e6; double x = 0; double w = 0; double diff = 0; double rcmin = 0;
        int minb = 0; double lbd; double ubd; double rmax=0; int flag=0;
	double z = 0; double y = 0; double v = 0; double dz = 0; double dv = 0; double d2z = 0;
	int fbin = 10; double max=0; double dlim=0; int nt=0; int ord=0; 
	int sref; int pref; int ppg[numpat][2]; int chop=2;	//'chop'= ignore first/last two bins of each pattern
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*find PPG transition boundaries
	//run through and find boundaries (pulser on/off etc.)
	//'ord' defines increase (1) or decrease (0) in rate	
	int xbins = *trun;
	double trans[xbins][3];
	double freq[fbin][2];
	int bnd[numpat][2];
	//initialise the bnd matrix (prevents the program from hanging up later!)
		for(int i=0; i<numpat; i++){
			bnd[i][0]=0; 
			bnd[i][1]=0;
		}
		//find change in counts between bins
		//we always look at the relative change transitioning 'up' (OFF->ON), even if we are transitioning 'down'
		//we don't want to misidentify gaps in spectrum as transitions! (& vice versa)
		for(int i=0; i<=xbins; i++){
			x = spec[cnt]->GetBinContent(i);
			if(w>0 && x>0 && x>w){			
				diff=((x-w)/w);
				ord = 1;	
			} else if(w>0 && x>0 && x<w) {
				diff=((w-x)/x);
				ord = 0;
			} else if(x>0 && w==0){
			//this statement deals with gaps in spectrum
			//we look at earlier bins and calculate difference compared to the current bin (up to t=0)					
				for (int j =(i-1); j>=0; j--){
					z = spec[cnt]->GetBinContent(j);  
					if(z>0 && x>z){
						diff=((x-z)/z);
						ord = 1;
						break;
					} else if(z>0 && x<z){
						diff=((z-x)/x);
						ord = 0;
						break;
					//what if there are no earlier bins >0? (rare, unless at very start of run)	
					}
				}    
			} else if(w>0 && x==0){		
			//we look at later bins and calculate difference compared to the current bin (up to t=trun)
			//what if there are no later bins with counts >0? (rare, unless at very end of run)						
				for (int j =(i+1); j<=*trun; j++){
					z = spec[cnt]->GetBinContent(j); 
					if(z>0){
						diff=0;
						break;
					} 
				}
				if(z==0){
					for (int j=*trun; j>=(*trun-(int(0.7*patlen))); j--){
						y = spec[cnt]->GetBinContent(j);
						if(y>0 && v>0){
							dz =(abs(y-v)/v);
							if(dz>0 && dv>0){
								d2z=abs(dz-dv);
							}
						}
						if(d2z>eor){
							flag+=1;
							diff=0;
							break;
						}
						v = spec[cnt]->GetBinContent(j);
						dv = dz;
					}
					if(flag==0){
						diff=(0.9*rmax);
					}
					flag=0;
				}
			} else if(x==w){
				diff=0;
			} else if(x==0 && w==0){
				diff=0;
			}
			trans[i][0]=i; trans[i][1]=diff; trans[i][2]=ord;
			w=spec[cnt]->GetBinContent(i);			
			if(abs(diff)>rmax){
				rmax=diff;
			}
		}
		rmax = rmax + (0.1*rmax);
		//set up the frequency histogram
		for(int i=0; i<=xbins; i++){
			for(int j=0; j<fbin; j++){
				if(i==1){		
				freq[j][0]=(0.+(double(j)*(rmax/double(fbin))));
				freq[j][1]=0;
			}
		if(abs(trans[i][1])>(0.+(double(j)*(rmax/double(fbin)))) && abs(trans[i][1])<=(rmax/double(fbin))+(double(j)*(rmax/double(fbin)))){
			freq[j][1]=freq[j][1]+1;
		}
			}
		}
		//set transition limit using histogram
		for(int j=0; j<fbin; j++){		
			if(freq[j][1]>max){
			max=freq[j][1];
			dlim = (2*((rmax/double(fbin))+(double(j)*(rmax/double(fbin)))));
			}
		}
		//std::cout<<"PPG transitions assumed above "<<(dlim*100.)<<" % change in rate.."<<endl;
		for(int i=0; i<=xbins; i++){
			if(abs(trans[i][1])>dlim){
				nt+=1;
				if(nt>numpat){
			printf(DRED "Warning: Found more PPG transition(s) than expected in spectrum %s (%i/%i)" RESET_COLOR "\n",sname,nt,numpat);
					//std::cout<<"(see bin number "<<i<<" in spectrum "<<sname<<")"<<endl;
					ppgstat[1]+=1;
					break;
				} else {
					bnd[nt-1][0]=i; bnd[nt-1][1]=trans[i][2];
				}
			}
		}	
		if(nt==numpat){
			printf(DGREEN "Found correct number of PPG transitions in spectrum %s (%i/%i)" RESET_COLOR "\n",sname,nt,numpat);
			ppgstat[0]+=1;
		} else if (nt<numpat){
			printf(DRED "Warning: Found too few PPG transitions in spectrum %s (%i/%i)" RESET_COLOR "\n",sname,nt,numpat);
			ppgstat[1]+=1;
		}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//DECIDE THE PPG ORDER (PULSER OFF/PULSER ON)
	ppg[0][0]=0; 		
	for(int i=0; i<numpat; i++){
		ppg[i+1][0]=bnd[i][0]; 	
		ppg[i][1]=(bnd[i][0])-1;	
	}
	if(bnd[0][1]==0){		//pulser must be first (first transition is pulser OFF)
		sref=1; pref=0;
	} else if (bnd[0][1]==1){	//source must be first (first transition is pulser ON)
		sref=0; pref=1;
	}
	//diagnostic only
	if(cnt%nsc==0){
		for(int i=0; i<numpat; i++){	
			ofile<<ppg[i][0]<<"\t"<<ppg[i][1]<<endl;
		}
		ofile<<"/"<<endl;
	}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//SOURCE ONLY
	for (int i=sref; i<numpat; i+=2) {
		for (int j=(ppg[i][0]+chop); j<=(ppg[i][1]-chop); j++) {
			x = spec[cnt]->GetBinContent(j);
			if(x!=0){
				wrand = wrand + x;
				nrand+=1;
			}	
		}
	}
	rrand = (wrand/nrand);	//mean
	//diagnostic spectrum (dspec) parameters
	int lim1 = rrand-(0.5*dlim*rrand); int lim2 = rrand+(0.5*dlim*rrand);
	int dsbin = (lim2-lim1)/20; int dspec[dsbin][2];
	for (int i=0; i<dsbin; i++){
		dspec[i][0]=lim1+(i*((lim2-lim1)/dsbin));
		dspec[i][1]=0;
	}
	//standard deviation / increment dspec
	for (int i=sref; i<numpat; i+=2) {
		for (int j=(ppg[i][0]+chop); j<=(ppg[i][1]-chop); j++) {
			x = spec[cnt]->GetBinContent(j);
				if(cnt%nsc==0){		//dspec for channel 0 only
					for(int k=0; k<dsbin; k++){
						if(x>=dspec[k][0] && x<dspec[k+1][0]){
						dspec[k][1]+=1;
						}
					}
				}
				if(x!=0){
				sum = sum + pow((x-rrand),2);
				}	
		}
	}
	sdrand = sqrt(sum/(nrand-1));		//standard deviation
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~FREQUENCY HISTOGRAM
	if(cnt%nsc==0){	
		for (int i=0; i<dsbin; i++){		//dspec
			fprintf(random, "%i \t %i", dspec[i][0], dspec[i][1]);
			fprintf(random, "\n");
		}
		fprintf(random, "/");
		fprintf(random, "\n");	
	}
	//std::cout<<"Mean = "<<rrand<<", standard deviation = "<<sdrand<<endl;
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//SOURCE+PULSER
	for(int i=pref; i<numpat; i+=2) {
		for (int j=(ppg[i][0]+chop); j<=(ppg[i][1]-chop); j++) {
			x = spec[cnt]->GetBinContent(j);
			if(x!=0){			
				wcomb = wcomb + x;
				ncomb+=1;
			} else {
			continue;
			}		
	   	}
	}
	rcomb = (wcomb/ncomb);	//mean
	//now calculate max. likelihood
	for(int i=pref; i<numpat; i+=2) {
		for (int j=(ppg[i][0]+chop); j<=(ppg[i][1]-chop); j++) {
			x = spec[cnt]->GetBinContent(j);
			if(x!=0){
				maxl = (0.5*(pow((x-rcomb),2)))/x;
					if(maxl<minl){
					minl=maxl;
					minb=j;
					}
			} else {
			continue;
			}		
	   	}
	}
	rcmin = spec[cnt]->GetBinContent(minb);
	lbd=rcmin; ubd=rcmin;
	//find min/max values of parabola with respect to minimum
	for(int i=pref; i<numpat; i+=2) {
		for (int j=(ppg[i][0]+chop); j<=(ppg[i][1]-chop); j++) {
			x = spec[cnt]->GetBinContent(j);
			if(x!=0){
				maxl = (0.5*(pow((x-rcmin),2)))/x;
					//ofile<<j<<"\t"<<x<<"\t"<<maxl<<endl;
					if(maxl<=(minl+0.5) && x<rcmin && x<lbd){	//fixed
						lbd = x;
					}
					if(maxl<=(minl+0.5) && x>rcmin && x>ubd){	//fixed
						ubd = x;
					}
			} else {
			continue;
			}		
	   	}
	} 
	//diagnostic spectrum (dspec) parameters (re-define for source+pulser)
	lim1 = lbd-(2.0*abs(rcmin-lbd)); lim2 = ubd+(2.0*abs(rcmin-ubd));
	//std::cout<<lim1<<"\t"<<lim2<<"\t"<<rcmin<<endl;
	dsbin = (lim2-lim1)/20; dspec[dsbin][2];
	for (int i=0; i<dsbin; i++){
		dspec[i][0]=lim1+(i*((lim2-lim1)/dsbin));
		dspec[i][1]=0;
	}
	//standard deviation / increment dspec
	for(int i=pref; i<numpat; i+=2) {
		for (int j=(ppg[i][0]+chop); j<=(ppg[i][1]-chop); j++) {
			x = spec[cnt]->GetBinContent(j);
				if(cnt%nsc==0){		//dspec for channel 0 only
					for(int k=0; k<dsbin; k++){
						if(x>=dspec[k][0] && x<dspec[k+1][0]){
						dspec[k][1]+=1;
						}
					}
				}
				if(x!=0){
				sumc = sumc + pow((x-rcomb),2);
				}		
	   	}
	}
	sdcomb = sqrt(sumc/(ncomb-1));		//standard deviation (not strictly applicable to source+pulser data)
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~FREQUENCY HISTOGRAM
	if(cnt%nsc==0){	
		for (int i=0; i<dsbin; i++){		//dspec
			fprintf(combine, "%i \t %i", dspec[i][0], dspec[i][1]);
			fprintf(combine, "\n");
		}
		fprintf(combine, "/");
		fprintf(combine, "\n");	
	}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//To find the error in (rcmin-rrand), we generate a likelihood curve using an iterative procedure
	//Ref. "Asymmetric Statistical Errors", Roger Barlow, http://arxiv.org/pdf/physics/0406120.pdf
	//WARNING: u must be given a reasonable range. This is assumed to be ~1.5*max(a1,a2)
		
	double a1=abs(rcmin-ubd); double a2=abs(rcmin-lbd);
	double maxa = std::max(a1,a2);
	double uhi = (1.5*maxa); double ulo = -uhi; double du = (uhi-ulo)/1e3; double u=ulo;
	int itr = 0; int umin; double w1; double x1; double x2; int nrow=int((uhi-ulo)/du);
	double w2 = pow((pow(sdrand,2)),2)/((2.*pow(sdrand,2)));	//const.
	minl=-1e6; double array[nrow][2]; double sigm; double sigp; double lnl;

		while(itr<nrow){			
			if(itr==0){
				x1 = 0.;
				x2 = 0.;
			} else {
				x1 = (u*w1)/(w1+w2);
				x2 = (u*w2)/(w1+w2);
			}
		w1 = pow(((a1*a2)+((a1-a2)*x1)),2)/((2.*(a1*a2))+((a1-a2)*x1));
		lnl = -0.5*((pow(x1,2)/((a1*a2)+((a1-a2)*x1)))+(pow(x2,2)/(pow(sdrand,2))));
			if(lnl<=0.){			//reject positive likelihoods (these tend occur at large +/- values of u) 
				if(itr>0 && lnl>minl){	//mean value
				minl=lnl;	
				umin=itr;
				}
				array[itr][0]=u;
				array[itr][1]=lnl;
				u+=du;
				itr++;
			} else {
				u+=du;
			}
		}
		//determine upper/lower limits
		sigm=array[umin][0]; sigp=array[umin][0];
		for(int i=1;i<nrow;i++){	
			if(array[i][1]>=(minl-0.5) && array[i][0]<array[umin][0] && array[i][0]<sigm){
				sigm = array[i][0];
			}
			if(array[i][1]>=(minl-0.5) && array[i][0]>array[umin][0] && array[i][0]>sigp){
				sigp = array[i][0];
			}
		}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*PARABOLA HISTOGRAM
	if(cnt%nsc==0){
  		fprintf(asymerror,"%s\t %s\t %s","#chan","Err[Hz]","Ln(L)");			
		fprintf(asymerror, "\n");		
		for (int i=1; i<nrow; i++){
			fprintf(asymerror, "%i \t %.4E \t %.4E",cnt%nsc,array[i][0],array[i][1]);
			fprintf(asymerror, "\n");
		}
		fprintf(asymerror, "/");
		fprintf(asymerror, "\n");	
	}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	double tau = ((1.0/rrand)*(1.0-sqrt((rcmin-rrand)/(*rate))))*1.0e6;	//deadtime (us)
	double rtau = 0;
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//FINAL ERROR ANALYSIS*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	//Idea: now that the error boundaries are known for rrand, rcmin and (rcmin-rrand), generate a random number
	//which chooses a value within these boundaries - the resulting tau is plotted and the range gives the error in tau  

	int iter=1e4, bin=10; double tempa, tempb; double var1, var2, var3; double l1, l2, m1=0, m2=1e6, n1=0, n2=1e6;
	int wbin = 40; int wrow=0, wsize=int(pow(wbin,2)); double randcheck[bin][2], randtau[iter][2], wspec[wsize][3];
	int flagc=0; int binsize=3;

	//Check the "randomness" of the random number generator **RCHECK**
	for(int i=0;i<bin;i++){
			randcheck[i][0]=0+(double(i)*(1/double(bin)));
			randcheck[i][1]=0;
	}
	//initialise matrix to get final uncertainty
	for(int i=0;i<wbin;i++){
		for(int j=0;j<wbin;j++){
			wspec[wrow][0]=((rcmin-rrand)+sigm)+(double(i)*((sigp-sigm)/double(wbin)));	//x, (rcmin-rrand)
			wspec[wrow][1]=(*lowrtau)+(double(j)*((*upprtau-*lowrtau)/double(wbin)));	//y, (rtau);	
			wspec[wrow][2]=0;								//z, frequency
			wrow+=1;									
		}
	}
	//generate all possible guesses for rcmin and rrand to satisfy test value of (rcmin-rrand) in the limits l1,l2
		int i = 0;
		while(i<(nrow-binsize)){
			if(array[i][0]>=sigm && array[i][0]<=sigp){
				l1=((rcmin-rrand)+array[i][0]);			
				l2=((rcmin-rrand)+array[i+binsize][0]);

				for(int j=0; j<iter; j++){
					tempa= rand() / double(RAND_MAX);
					var1= ((tempa*(a1+a2))+(rcmin-a2));
					tempb= rand() / double(RAND_MAX);
					var2= ((tempb*(2.e0*sdrand))+(rrand-sdrand));
					var3= (var1-var2);
					//~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~'good' events	
					if(var3>=l1 && var3<=l2){
						flagc++;
						//calculate deadtime using var3(rcmin-rrand), var2(rrand)
						rtau=((1.0/var2)*(1.0-sqrt(var3/(*rate))))*1.0e6;
						//build wspec matrix
						for(int k=0; k<wsize; k++){
							if(var3>=wspec[k][0] && var3<wspec[k+wbin][0] && rtau>=wspec[k][1] && rtau<wspec[k+1][1]){
								wspec[k][2]+=1;	//this seems to work now
							}
						}
					//~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
					} else {
						continue;
					}
				}
				i+=binsize;
				flagc=0;				
			} else {
				i+=1;
				continue;
			}
		}
	//diagnostic
	//printf(DMAGENTA "(comb-rand)= %f, sigp= %f, sigm= %f, rand= %f, sdrand= %f" RESET_COLOR "\n",(rcmin-rrand),sigp,sigm,rrand,sdrand);
	//~*~*~*~*~*~*~*~*~*~*~*~~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
	//Determine final error bounds using wspec matrix
	double gmax=0, hmax=0, tmax=0; double erbp=0, erbm=0, erfp=0, erfm=0;
	double frmin=1e6, frmax=0, srmin=1e6, srmax=0;

	for(int i=0; i<wsize; i++){
		if(wspec[i][2]>gmax){
			gmax=wspec[i][2];
			tmax=wspec[i][1];
			hmax=(gmax/2);
		}		
	}
	//calculate max/min range (select above half the height of 'peak')
	for(int i=0; i<wsize; i++){
		if(wspec[i][1]>srmax && wspec[i][2]>hmax){
			srmax=wspec[i][1];
		}
		if(wspec[i][1]<srmin && wspec[i][2]>hmax){
			srmin=wspec[i][1];
		}
	}
	//calculate max/min range (only select above zero)
	for(int i=0; i<wsize; i++){
		if(wspec[i][1]>frmax && wspec[i][2]>0){
			frmax=wspec[i][1];
		}
		if(wspec[i][1]<frmin && wspec[i][2]>0){
			frmin=wspec[i][1];
		}
	}
	//final errors: b=best estimate, f=full range
	erbm=abs(tau-srmin);	
	erbp=abs(tau-srmax);
	erfm=abs(tau-frmin);
	erfp=abs(tau-frmax);
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*STANDARD ERROR PROPAGATION
	double p1=sqrt(pow(sdcomb,2)+pow(sdrand,2));
	double p1r=p1/(rcmin-rrand);
	double p2=(rcmin-rrand)/(*rate);
	//Here we assume that sqrt(rel.err.) ~ 0.5 * rel.err. This is approximately true where the +/- uncertainties are similar.
	double p3=(0.5*p1r*p2);
	double eprop=tau*(sqrt(pow((p3/(1-sqrt(p2))),2)+pow((sdrand/rrand),2)));
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*RANDOM TAU HISTOGRAM
	//if(cnt%nsc==0){
		fprintf(randdt, "%i \t %.2f \t %.2f \t %.2f \t %.2f",cnt,srmin,srmax,frmin,frmax);
		//fprintf(randdt, "#//end channel 0");
		fprintf(randdt, "\n");
	//}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~WIDTH HISTOGRAM
	if(cnt%nsc==0){
		for (int i=0; i<wsize; i++){
				fprintf(randw, "%.2f \t %.2f \t %.2f", wspec[i][0], wspec[i][1], wspec[i][2]);
				fprintf(randw, "\n");
		}
		fprintf(randw, "//end channel 0");
		fprintf(randw, "\n");
	}
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~RCHECK HISTOGRAM
	//print random number generator results to file for a single spectrum
	/*if(cnt%nsc==0){
		for (int i=0; i<bin; i++){
			fprintf(rng, "%f \t %f", randcheck[i][0], randcheck[i][1]);
			fprintf(rng, "\n");
		}
		fprintf(rng, "//end channel 0");
		fprintf(rng, "\n");
	}*/
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~PRINT FINAL RESULTS TO FILE
	fprintf(deadtime, "%s\t%i\t%.1f\t%.1f\t%.2f\t%.1f\t%.2f\t%.2f\t %.1f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f",sname,(cnt%nsc),((*rate)/1e3),(rrand/1e3),(sdrand/1e3),(rcmin/1e3),(abs(rcmin-ubd)/1e3),(abs(rcmin-lbd)/1e3),tau,erbp,erbm,erfp,erfm,eprop,"\n");
	fprintf(deadtime, "\n");	
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
		cnt ++;
		//change pulser rate and runtime accordingly for each file
		if(cnt%(nsc*nscaler)==0){	
			*rate++;
			*trun++;
		}
		//change limits accordingly for each scaler in each file
		if(cnt%(nsc)==0) {
			for(int i=0;i<2;i++){
				*lowrtau++;	
				*upprtau++;
			}
		}
		if(cnt%(nsc)==0 && cnt%(nsc*nscaler)==0){	
			cflag++;
			if(cflag==1){
				lowrtau=&rp2[0]; upprtau=&rp2[1];
			} else if(cflag==2){
				lowrtau=&rp3[0]; upprtau=&rp3[1];
			} else if(cflag==3){
				lowrtau=&rp4[0]; upprtau=&rp4[1];
			}
		}
	}	//END SPECTRUM ANALYSIS LOOP
	//*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~**~*~*~*~*~*~**~*~*~*~*~*~*
	ofile.close();
	fclose(random);
	fclose(combine);
	fclose(rng);
	fclose(deadtime);
	fclose(asymerror);
	fclose(randdt);
	fclose(randw);
	int good = ppgstat[0];
	printf("\n");
		//info/warnings
		if(good<cnt){
			printf(DRED "Correct # of PPG transitions obtained for %i out of %i spectra" RESET_COLOR "\n",good,cnt);
		} else {
			printf(DBLUE "Correct # of PPG transitions obtained for %i out of %i spectra" RESET_COLOR "\n",good,cnt);
		}
  return;
}
