//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Aug 11 14:56:50 2014 by ROOT version 5.34/14
// from TTree FragmentTree/FragmentTree
// found on file: fragment00008_000.root
//////////////////////////////////////////////////////////

#ifndef TFragmentSelector_h
#define TFragmentSelector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

#include <TSystem.h>

//class TChannel;

// Header file for the classes stored in the TTree if any.
#include "TFragment.h"
#include "TChannel.h"



// Fixed size dimensions of array or collections stored in the TTree if any.

class TFragmentSelector : public TSelector {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

   // Declaration of leaf types
 //TFragment       *TFragment;

   TFragment *fragment;      //The Current fragment being sent to the fragment selector
   
/*
   Long_t          MidasTimeStamp;
   UInt_t          MidasId;
   ULong_t         TriggerId;
   UInt_t          FragmentId;
   Int_t           TriggerBitPattern;
   Int_t           ChannelNumber;
   UInt_t           ChannelAddress;
   std::vector<int32_t> Cfd;
   std::vector<int32_t> Led;
   std::vector<int32_t> Charge;
   Int_t           TimeStampLow;
   Int_t           TimeStampHigh;
   Int_t           TimeToTrig;
   UInt_t          PPG;
   UShort_t        DeadTime;
   UShort_t        NumberOfFilters;
   UShort_t        NumberOfPileups;
   UShort_t        DataType;
   UShort_t        DetectorType;
   UInt_t          ChannelId;
   std::vector<unsigned short> KValue;
   std::vector<int16_t> wavebuffer;

   // List of branches
   TBranch        *b_TFragment_MidasTimeStamp;   //!
   TBranch        *b_TFragment_MidasId;   //!
   TBranch        *b_TFragment_TriggerId;   //!
   TBranch        *b_TFragment_FragmentId;   //!
   TBranch        *b_TFragment_TriggerBitPattern;   //!
   TBranch        *b_TFragment_ChannelNumber;   //!
   TBranch        *b_TFragment_ChannelAddress;   //!
   TBranch        *b_TFragment_Cfd;   //!
   TBranch        *b_TFragment_Led;   //!
   TBranch        *b_TFragment_Charge;   //!
   TBranch        *b_TFragment_TimeStampLow;   //!
   TBranch        *b_TFragment_TimeStampHigh;   //!
   TBranch        *b_TFragment_TimeToTrig;   //!
   TBranch        *b_TFragment_PPG;   //!
   TBranch        *b_TFragment_DeadTime;   //!
   TBranch        *b_TFragment_NumberOfFilters;   //!
   TBranch        *b_TFragment_NumberOfPileups;   //!
   TBranch        *b_TFragment_DataType;   //!
   TBranch        *b_TFragment_DetectorType;   //!
   TBranch        *b_TFragment_ChannelId;   //!
   TBranch        *b_TFragment_KValue;   //!
   TBranch        *b_TFragment_wavebuffer;   //!
*/


   TFragmentSelector(int runnumber=0,int subrunnumber=0,TTree * /*tree*/ =0) : fChain(0),fragment(0) {
    frunnumber = runnumber; fsubrunnumber = subrunnumber;
   }
   virtual ~TFragmentSelector() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   private:
     int frunnumber;     //The current run number
     int fsubrunnumber;  //The current subrun number

   ClassDef(TFragmentSelector,0); //Filters TFragments and creates User Defined histograms
};

#endif

#ifdef TFragmentSelector_cxx
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
   if (!tree) return;
   fChain = tree;

   fChain->SetBranchAddress("TFragment",&fragment);


  if(TChannel::GetNumberOfChannels()==0) {
     TList *chanlist = fChain->GetUserInfo();
     TIter iter(chanlist);
		 while(TObject *obj = iter.Next()) {
     //while(TChannel *chan = (TChannel*)iter.Next()) {
				if(!obj->InheritsFrom("TChannel"))
					continue;
				TChannel *newchan = new TChannel((TChannel*)obj);//->GetAddress());
				TChannel::UpdateChannel(newchan);
        //TChannel::CopyChannel(newchan,chan);
     }
   }



/*
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("MidasTimeStamp", &MidasTimeStamp, &b_TFragment_MidasTimeStamp);
   fChain->SetBranchAddress("MidasId", &MidasId, &b_TFragment_MidasId);
   fChain->SetBranchAddress("TriggerId", &TriggerId, &b_TFragment_TriggerId);
   fChain->SetBranchAddress("FragmentId", &FragmentId, &b_TFragment_FragmentId);
   fChain->SetBranchAddress("TriggerBitPattern", &TriggerBitPattern, &b_TFragment_TriggerBitPattern);
   fChain->SetBranchAddress("ChannelNumber", &ChannelNumber, &b_TFragment_ChannelNumber);
   fChain->SetBranchAddress("ChannelAddress", &ChannelAddress, &b_TFragment_ChannelAddress);
   fChain->SetBranchAddress("Cfd", &Cfd, &b_TFragment_Cfd);
   fChain->SetBranchAddress("Led", &Led, &b_TFragment_Led);
   fChain->SetBranchAddress("Charge", &Charge, &b_TFragment_Charge);
   fChain->SetBranchAddress("TimeStampLow", &TimeStampLow, &b_TFragment_TimeStampLow);
   fChain->SetBranchAddress("TimeStampHigh", &TimeStampHigh, &b_TFragment_TimeStampHigh);
   fChain->SetBranchAddress("TimeToTrig", &TimeToTrig, &b_TFragment_TimeToTrig);
   fChain->SetBranchAddress("PPG", &PPG, &b_TFragment_PPG);
   fChain->SetBranchAddress("DeadTime", &DeadTime, &b_TFragment_DeadTime);
   fChain->SetBranchAddress("NumberOfFilters", &NumberOfFilters, &b_TFragment_NumberOfFilters);
   fChain->SetBranchAddress("NumberOfPileups", &NumberOfPileups, &b_TFragment_NumberOfPileups);
   fChain->SetBranchAddress("DataType", &DataType, &b_TFragment_DataType);
   fChain->SetBranchAddress("DetectorType", &DetectorType, &b_TFragment_DetectorType);
   fChain->SetBranchAddress("ChannelId", &ChannelId, &b_TFragment_ChannelId);
   fChain->SetBranchAddress("KValue", &KValue, &b_TFragment_KValue);
   fChain->SetBranchAddress("wavebuffer", &wavebuffer, &b_TFragment_wavebuffer);
*/
}


Bool_t TFragmentSelector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef TFragmentSelector_cxx
