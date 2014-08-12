//rootcint TFragmentSelectorDict.cxx -c TFragmentSelector.h
//g++ main.cxx TFragmentSelector.cxx TFragmentSelectorDict.cxx -I ../GRSISort/include -L ../libraries `root-config --cflags --glibs` -lProof -lGRSIFormat -o runme.exe

#include <cstdio>
#include <vector>

#include <TChain.h>
#include <TProofLite.h>

#include "TFragmentSelector.h"


int main(int argc, char **argv) {
	if(argc==1) {
		printf("Add fragment<>.root files to the command line.\n");
		return 1;
	}

	TChain *chain = new TChain("FragmentTree");
	for(int x=1;x<argc;x++)
		chain->Add(argv[x]);


	TProofLite *proof = (TProofLite*)TProofLite::Open("");


	proof->ClearCache();	

	//proof->AddIncludePath("/home/grifuser/packages/GRSISort/include");

	proof->Exec("gSystem->Load(\"$(GRSISYS)/libraries/libGRSIFormat.so\")");

	

	chain->SetProof();
	TFragmentSelector *fragSelc = new TFragmentSelector;

//	proof->SetBit(TProof::kUsingSessionGui);

	chain->Process("TFragmentSelector.cxx+");   //fragSelc);
	//gProof->Process("TFragmentSelector.cxx+");   //fragSelc);
	//gProof->Process("TFragmentSelector.cxx+",chain->GetEntries());

	
	return 0;

}
