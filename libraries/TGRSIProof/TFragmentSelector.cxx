// The class definition in TFragmentSelector.h has been generated automatically
// by the ROOT utility TTree::MakeSelector(). This class is derived
// from the ROOT class TSelector. For more information on the TSelector
// framework see $ROOTSYS/README/README.SELECTOR or the ROOT User Manual.

// The following methods are defined in this file:
//    Begin():        called every time a loop on the tree starts,
//                    a convenient place to create your histograms.
//    SlaveBegin():   called after Begin(), when on PROOF called only on the
//                    slave servers.
//    Process():      called for each event, in this function you decide what
//                    to read and fill your histograms.
//    SlaveTerminate: called at the end of the loop on the tree, when on PROOF
//                    called only on the slave servers.
//    Terminate():    called at the end of the loop on the tree,
//                    a convenient place to draw/fit your histograms.
//
// To use this file, try the following session on your Tree T:
//
// Root > T->Process("TFragmentSelector.C")
// Root > T->Process("TFragmentSelector.C","some options")
// Root > T->Process("TFragmentSelector.C+")
//

#include "TFragmentSelector.h"
#include <TSystem.h>
#include "TGRSIRunInfo.h"
#include <TH2.h>
#include <TStyle.h>
/// \cond CLASSIMP
ClassImp(TFragmentSelector)
/// \endcond

TH2F* hp_charge;
TH2F* hp_energy;

void TFragmentSelector::Begin(TTree * /*tree*/)
{
   // The Begin() function is called at the start of the query.
   // When running with PROOF Begin() is only called on the client.
   // The tree argument is deprecated (on PROOF 0 is passed).
   std::cout << "Begin" << std::endl;
   std::cout << fChain << std::endl;
   TString option = GetOption();

}

void TFragmentSelector::SlaveBegin(TTree * /*tree*/)
{
   // The SlaveBegin() function is called after the Begin() function.
   // When running with PROOF SlaveBegin() is called on each slave server.
   // The tree argument is deprecated (on PROOF 0 is passed).
   TChannel::ReadCalFromTree(fChain);
   std::cout << "Starting a slave" << std::endl;
   TString option = GetOption();
   hp_charge = new TH2F("hp_charge","Channel vs Charge",128,0,128,24000,0,12000);
   hp_energy = new TH2F("hp_energy","Channel vs Energy",128,0,128,16000,0,8000);
   fOutput->AddAll(gDirectory->GetList());
}

Bool_t TFragmentSelector::Process(Long64_t entry)
{
   // The Process() function is called for each entry in the tree (or possibly
   // keyed object in the case of PROOF) to be processed. The entry argument
   // specifies which entry in the currently loaded tree is to be processed.
   // It can be passed to either TFragmentSelector::GetEntry() or TBranch::GetEntry()
   // to read either all or the required parts of the data. When processing
   // keyed objects with PROOF, the object is already loaded and is available
   // via the fObject pointer.
   //
   // This function should contain the "body" of the analysis. It can contain
   // simple or elaborate selection criteria, run algorithms on the data
   // of the event and typically fill histograms.
   //
   // The processing can be stopped by calling Abort().
   //
   // Use fStatus to set the return value of TTree::Process().
   //
   // The return value is currently not used
   GetEntry(entry);
   if(!(entry%100000)) std::cout << "On entry: "<< entry << "\r";
   hp_charge->Fill(fFragment->GetChannelNumber(),fFragment->GetCharge());
   hp_energy->Fill(fFragment->GetChannelNumber(),fFragment->GetEnergy());

   return kTRUE;
}

void TFragmentSelector::SlaveTerminate()
{
   // The SlaveTerminate() function is called after all entries or objects
   // have been processed. When running with PROOF SlaveTerminate() is called
   // on each slave server.

}

void TFragmentSelector::Terminate()
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.
   Int_t runnumber = TGRSIRunInfo::Get()->RunNumber();
   Int_t subrunnumber = TGRSIRunInfo::Get()->SubRunNumber();
   TFile output_file(Form("TFragmentSelector%05d_%03d.root",runnumber,subrunnumber),"RECREATE");
   fOutput->Write();
   output_file.Close();

}

void TFragmentSelector::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).
   // Set branch addresses and branch pointers
   std::cout << "Init " << std::endl;
   if (!tree) return;
   
   fChain = tree;
   //fChain->SetMakeClass(1);
   
   fChain->SetBranchAddress("TFragment", &fFragment);
}


Bool_t TFragmentSelector::Notify()
{

   std::cout << "Notified " << std::endl;
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

