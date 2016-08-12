#include "TGRSIRootIO.h"

#include "TFragmentQueue.h"

#include "TGRSIOptions2.h"
#include "TGRSIRunInfo.h"
#include "TGRSIint.h"

/// \cond CLASSIMP
ClassImp(TGRSIRootIO)
/// \endcond

TGRSIRootIO* TGRSIRootIO::fTGRSIRootIO = 0;

TGRSIRootIO* TGRSIRootIO::Get()  {
	if(!fTGRSIRootIO)
		fTGRSIRootIO = new TGRSIRootIO;
	return fTGRSIRootIO;
}

TGRSIRootIO::TGRSIRootIO() {
	fFragmentTree    = 0;
	fBadFragmentTree =0;
	fEpicsTree    = 0;
	fDeadtimeScalerTree   = 0;
	fRateScalerTree   = 0;
	fPPG          = 0;
	fDiagnostics = NULL;

	fOutFile = 0;
}

TGRSIRootIO::~TGRSIRootIO() {  }

void TGRSIRootIO::LoadRootFile(TFile* file) {
	if(file)
		fInFiles.push_back(file);
	return;
}

void TGRSIRootIO::SetUpFragmentTree() {
	if(fOutFile)
		fOutFile->cd();
	fTimesFillCalled = 0;
	fFragmentTree = new TTree("FragmentTree","FragmentTree");
	fBufferFrag = 0;
	fFragmentTree->Bronch("TFragment","TFragment",&fBufferFrag,128000,99);
	//fFragmentTree->BranchRef();
	printf("FragmentTree set up.\n");
}


void TGRSIRootIO::SetUpBadFragmentTree() {
	if(!TGRSIOptions2::Get()->WriteBadFrags())
		return;
	if(fOutFile)
		fOutFile->cd();
	fTimesBadFillCalled = 0;
	fBadFragmentTree = new TTree("BadFragmentTree","BadFragmentTree");
	fBadBufferFrag = 0;
	fBadFragmentTree->Bronch("TFragment","TFragment",&fBadBufferFrag,128000,99);
	printf("BadFragmentTree set up.\n");
}

void TGRSIRootIO::SetUpPPG() {
	if(fOutFile)
		fOutFile->cd();
	fTimesPPGCalled = 0;
	if(TGRSIRunInfo::SubRunNumber() <= 0) {
		fPPG = TPPG::Get();//new TPPG;
		printf("PPG set up.\n");
	} else {
		TFile* prevSubRun = new TFile(Form("fragment%05d_%03d.root",TGRSIRunInfo::RunNumber(),TGRSIRunInfo::SubRunNumber()-1));
		if(prevSubRun->IsOpen()) {
			if(prevSubRun->Get("TPPG") != NULL) {
            printf("Found previous PPG data from run %s\n",prevSubRun->GetName());
				fPPG = (TPPG*) (prevSubRun->Get("TPPG")->Clone());
			} else {
				printf("Error, could not find PPG in file fragment%05d_%03d.root, not adding previous PPG data\n",TGRSIRunInfo::RunNumber(),TGRSIRunInfo::SubRunNumber()-1);
				fPPG = TPPG::Get();//new TPPG;
				printf("PPG set up.\n");
			}
			prevSubRun->Close();
		} else {
			printf("Error, could not find file fragment%05d_%03d.root, not adding previous PPG data\n",TGRSIRunInfo::RunNumber(),TGRSIRunInfo::SubRunNumber()-1);
			fPPG = TPPG::Get();//new TPPG;
			printf("PPG set up.\n");
		}
	}
}


void TGRSIRootIO::SetUpScalerTrees() {
	if(TGRSIOptions2::Get()->IgnoreScaler())
		return;
	if(fOutFile)
		fOutFile->cd();
	fTimesDeadtimeScalerCalled = 0;
	fDeadtimeScalerTree = new TTree("ScalerTree","ScalerTree");
	fDeadtimeScalerData = NULL;
	fDeadtimeScalerTree->Branch("TScalerData","TScalerData",&fDeadtimeScalerData);//,128000,99);
	fDeadtimeScalerTree->BranchRef();
	fTimesRateScalerCalled = 0;
	fRateScalerTree = new TTree("RateScalerTree","RateScalerTree");
	fRateScalerData = NULL;
	fRateScalerTree->Branch("TScalerData","TScalerData",&fRateScalerData);//,128000,99);
	fRateScalerTree->BranchRef();
	printf("Scaler-Trees set up.\n");
}

void TGRSIRootIO::SetUpEpicsTree() {
	if(TGRSIOptions2::Get()->IgnoreEpics())
		return;
	if(fOutFile)
		fOutFile->cd();
	fEPICSTimesFillCalled = 0;
	fEpicsTree = new TTree("EpicsTree","EpicsTree");
	fEXBufferFrag = 0;
	fEpicsTree->Bronch("TEpicsFrag","TEpicsFrag",&fEXBufferFrag,128000,99);
	fEpicsTree->BranchRef();
	printf("EPICS-Tree set up.\n");
}

void TGRSIRootIO::SetUpDiagnostics() {
	if(fOutFile)
		fOutFile->cd();
	fDiagnostics = new TDiagnostics;
}

TDiagnostics* TGRSIRootIO::GetDiagnostics(){
	if (!fDiagnostics)
		SetUpDiagnostics();
	return fDiagnostics;
}

void TGRSIRootIO::FillFragmentTree(TFragment* frag) {
	// if(!fFragmentTree)
	//    return;
	*fBufferFrag = *frag;
	int bytes =  fFragmentTree->Fill();
	if(bytes < 1)
		printf("\n fill failed with bytes = %i\n",bytes);
	fTimesFillCalled++;
}


void TGRSIRootIO::FillBadFragmentTree(TFragment* frag) {
	if(!TGRSIOptions2::Get()->WriteBadFrags())
		return;
	*fBadBufferFrag = *frag;
	int bytes =  fBadFragmentTree->Fill();
	if(bytes < 1)
		printf("\n (BadTree) fill failed with bytes = %i\n",bytes);
	fTimesBadFillCalled++;
}

void TGRSIRootIO::FillPPG(TPPGData* data) {
	//Set PPG Stuff here
	fPPG->AddData(data);
	++fTimesPPGCalled;
}

void TGRSIRootIO::FillDeadtimeScalerTree(TScalerData* scalerData) {
	if(TGRSIOptions2::Get()->IgnoreScaler())
		return;
	*fDeadtimeScalerData = *scalerData;
	int bytes =  fDeadtimeScalerTree->Fill();
	if(bytes < 1)
		printf("\n fill failed with bytes = %i\n",bytes);
	fTimesDeadtimeScalerCalled++;
}

void TGRSIRootIO::FillRateScalerTree(TScalerData* scalerData) {
	if(TGRSIOptions2::Get()->IgnoreScaler())
		return;
	*fRateScalerData = *scalerData;
	int bytes =  fRateScalerTree->Fill();
	if(bytes < 1)
		printf("\n fill failed with bytes = %i\n",bytes);
	fTimesRateScalerCalled++;
}

void TGRSIRootIO::FillEpicsTree(TEpicsFrag* EXfrag) {
	if(TGRSIOptions2::Get()->IgnoreEpics())
		return;
	if(!fEpicsTree)
		return;
	*fEXBufferFrag = *EXfrag;
	int bytes =  fEpicsTree->Fill();
	if(bytes < 1)
		printf("\n [EPIX] fill failed with bytes = %i\n",bytes);
	fEPICSTimesFillCalled++;
}

void TGRSIRootIO::FinalizeFragmentTree() {
	if(!fFragmentTree || !fOutFile)
		return;

	fOutFile->cd();
	TChannel* chan = TChannel::GetDefaultChannel();
	if(chan != NULL) {
		chan->SetNameTitle(Form("TChannels[%i]",TChannel::GetNumberOfChannels()),
				Form("%i TChannels.",TChannel::GetNumberOfChannels()));
		// using the write command on any TChannel will now write all
		chan->WriteToRoot(); // the TChannels to a root file.  additionally reading a TChannel
		// from a rootfile will read all the channels saved to it.  TChannels
		// are now saved as a text buffer to the root file.  pcb.
		// update. (3/9/2015) the WriteToRoot function should now
		// corretcly save the TChannels even if the came from the odb(i.e. internal
		// data buffer not set.)  pcb.
	} else {
		printf("Failed to get default channel, not going to write TChannel information!\n");
	}
   TGRSIRunInfo::WriteToRoot();

	fOutFile->cd();
	fFragmentTree->AutoSave(); //Write();
}

void TGRSIRootIO::FinalizeBadFragmentTree() {
	if(!fFragmentTree || !fOutFile)
		return;
	fOutFile->cd();
	fBadFragmentTree->AutoSave(); //Write();
}

void TGRSIRootIO::FinalizePPG() {
	if(!fPPG || !fOutFile)
		return;
	fOutFile->cd();
	if(fPPG->PPGSize()) {
		printf("Writing PPG\n");
		fPPG->Write("TPPG",TObject::kSingleKey);
	}
}

void TGRSIRootIO::FinalizeEpicsTree() {
	if(TGRSIOptions2::Get()->IgnoreEpics())
		return;
	if(!fEpicsTree || !fOutFile)
		return;
	fOutFile->cd();
	fEpicsTree->AutoSave(); //Write();
}

void TGRSIRootIO::FinalizeScalerTrees() {
	if(TGRSIOptions2::Get()->IgnoreScaler())
		return;
	if(!fDeadtimeScalerTree || !fRateScalerTree || !fOutFile)
		return;
	fOutFile->cd();
	if(fDeadtimeScalerTree->GetEntries()>0) fDeadtimeScalerTree->Write();
	if(fRateScalerTree->GetEntries()>0) fRateScalerTree->Write();
}

void TGRSIRootIO::FinalizeDiagnostics() {
	if(!fDiagnostics || !fOutFile)
		return;
	fOutFile->cd();
	fDiagnostics->ReadPPG(fPPG); //this function checks itself whether fPPG is NULL or not
	printf("Writing Diagnostics to root file.\n");
	fDiagnostics->Write("TDiagnostics",TObject::kSingleKey);
	if(TGRSIOptions2::Get()->WriteDiagnostics()) {
	  fDiagnostics->WriteToFile(Form("stats%05i_%03i.log", TGRSIRunInfo::RunNumber(), TGRSIRunInfo::SubRunNumber()));
	}
}

bool TGRSIRootIO::SetUpRootOutFile(int runNumber, int subRunNumber) {
	char fileName[64];
	if(subRunNumber>-1)
		sprintf(fileName,"fragment%05i_%03i.root",runNumber,subRunNumber);
	else
		sprintf(fileName,"fragment%05i.root",runNumber);
	printf("Creating root outfile: %s\n",fileName);
	//Add the fileName to the possible root files so that it can be auto sorted.
	//If there are no -s or -a flags these extra names do not matter
	std::string tempName(fileName);
	//TGRSIOptions2::Get()->AddInputRootFile(tempName);
	fOutFile = new TFile(fileName,"recreate");

	if(!fOutFile->IsOpen()) {
		return false;
	}
   TGRSIRunInfo::SetRunNumber(runNumber);
   TGRSIRunInfo::SetSubRunNumber(subRunNumber);
	SetUpFragmentTree();
	SetUpBadFragmentTree();
	SetUpPPG();
	SetUpScalerTrees();
	SetUpEpicsTree();
	SetUpDiagnostics();

	return true;
}

void TGRSIRootIO::CloseRootOutFile()   {


	if(!fOutFile)
		return;
	fOutFile->cd();
	printf(DMAGENTA "\n Fill tree called " DYELLOW "%i " DMAGENTA "times.\n" RESET_COLOR, fTimesFillCalled);
	if(TGRSIOptions2::Get()->WriteBadFrags())
		printf(DRED "   Fill bad tree called " DYELLOW "%i " DRED "times.\n" RESET_COLOR, fTimesBadFillCalled);

	FinalizeFragmentTree();
	FinalizePPG();
	FinalizeScalerTrees();
	FinalizeEpicsTree();
	FinalizeDiagnostics();

	if(TGRSIRunInfo::GetNumberOfSystems()>0) {
		printf(DMAGENTA " Writing RunInfo with " DYELLOW "%i " DMAGENTA " systems to file." RESET_COLOR "\n",TGRSIRunInfo::GetNumberOfSystems());
		//get run start and stop (in seconds) from the fragment tree
		TGRSIRunInfo::Get()->SetRunStart(fFragmentTree->GetMinimum("MidasTimeStamp"));
		TGRSIRunInfo::Get()->SetRunStop( fFragmentTree->GetMaximum("MidasTimeStamp"));
		TGRSIRunInfo::Get()->SetRunLength(fFragmentTree->GetMaximum("MidasTimeStamp") - fFragmentTree->GetMinimum("MidasTimeStamp"));
		printf("set run start to %.0f, and stop to %.0f (run length %.0f)\n",TGRSIRunInfo::Get()->RunStart(),TGRSIRunInfo::Get()->RunStop(),TGRSIRunInfo::Get()->RunLength());
		TGRSIRunInfo::Get()->Write();
	}

	fOutFile->Close();

	delete fOutFile;
	fOutFile = 0;
}

/*
void TGRSIRootIO::MakeUserHistsFromFragmentTree() {
	int runNumber =0; //This is where hists run number problem is happening
	int subRunNumber=0; //Might have to make these vectors?

	TChain* chain = new TChain("FragmentTree");

	for(size_t x=0;x<TGRSIOptions2::Get()->GetInputRoot().size();x++) {
		TFile f(TGRSIOptions2::Get()->GetInputRoot().at(x).c_str(),"read");
		chain->Add(TGRSIOptions2::Get()->GetInputRoot().at(x).c_str());
		f.Close();
	}

	const char* firstFileName = TGRSIOptions2::Get()->GetInputRoot().at(0).c_str();
	runNumber    = GetRunNumber(firstFileName);
	subRunNumber = GetSubRunNumber(firstFileName);

   TProofLite *proof = (TProofLite*)TProofLite::Open("");
   proof->ClearCache();
  // proof->Exec("gSystem->Load(\"libTGRSIFormat.so\")");
   const char* pPath = getenv("GRSISYS");
   gSystem->AddDynamicPath(pPath);
   proof->Exec(Form("gSystem->Load(\"%s/libraries/libTGRSIFormat.so\")",pPath));
  // printf("%s\n",pPath);
  // printf("%d\n",gSystem->Load(Form("%s/libraries/libTGRSIFormat.so",pPath)));
   proof->SetProgressDialog(TGRSIOptions2::Get()->ProgressDialog());
   //Going to get run number from file name. This will allow us to chain->chop off the subrun numbers

	chain->SetProof();
	TFragmentSelector* fragSelc = new TFragmentSelector(runNumber,subRunNumber);
	chain->Process(fragSelc);

	chain->Delete();
}
*/

int TGRSIRootIO::GetRunNumber(std::string fileName) {
	if(fileName.length()==0) {
		return 0;
	}
	std::size_t found = fileName.rfind(".root");
	if(found == std::string::npos) {
		return 0;
	}
	std::size_t found2 = fileName.rfind('-');
	//printf("found 2 = %i\n",found2);

	if(found2 == std::string::npos)
		found2 = fileName.rfind('_');
	std::string temp;
	if(found2 == std::string::npos || fileName.compare(found2+4,5,".root") !=0 ) {
		temp = fileName.substr(found-5,5);
	}
	else {
		temp = fileName.substr(found-9,5);
	}
	return atoi(temp.c_str());
}


int TGRSIRootIO::GetSubRunNumber(std::string fileName)	{
	if(fileName.length()==0)
		return -1;

	std::size_t found = fileName.rfind("-");
	if(found != std::string::npos) {
		std::string temp = fileName.substr(found+1,3);
		return atoi(temp.c_str());
	}
	found = fileName.rfind("_");
	if(found != std::string::npos) {
		std::string temp = fileName.substr(found+1,3);
		return atoi(temp.c_str());
	}
	return -1;
}
