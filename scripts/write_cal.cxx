//Written by: B. Olaizola 2017-08-01

//Experiment: PAELLA

//Root version: 6.08/02

//GRSIsort version: 3.1.3.2


//This script takes a bunch of root files and writes a given calfile to them.

#include <iostream>
#include <fstream>
using namespace std;
#include <iomanip>
#include <string>
#include <vector>

#include "TFile.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH2F.h"
#include "THnSparse.h"

#include "TGriffin.h"
#include "TSceptar.h"
#include "TLaBr.h"
#include "TZeroDegree.h"
#include "TTAC.h"
#include "TString.h"

//const char* path =  "/data1/bruno/griffin/50Ca_Nov2016"; //Currently not being used
//const char* calfile = "/data1/bruno/griffin/timing_test/Co60-test.cal"; //If you don't want to automatically apply a calibration file, comment this
const char* calfile = "/data2/griffin/PAELLA/paella_Cd111_2us.cal";

int main(int argc, char** argv) {
   	printf("Reading calibration file: %s\n",calfile);
   if(argc == 1) {
      std::cout<<"Usage: "<<argv[0]<<" <analysis tree file(s)>"<<std::endl;
      return 1;
   }

   	  TChannel::ReadCalFile(calfile);

   for(int i = 1; i < argc; ++i) {
	   TFile * f = new TFile(argv[i]);
      std::cout<<"Writting to: '"<<argv[i]<<std::endl;
	  TChannel::WriteToRoot();
   }

   return 0;
}


