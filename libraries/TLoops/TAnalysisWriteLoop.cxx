#include "TAnalysisWriteLoop.h"

#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"
#include "TROOT.h"

#include "GValue.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"
#include "TGRSIOptions.h"
#include "TTreeFillMutex.h"
#include "TAnalysisOptions.h"
#include "TSortingDiagnostics.h"
#include "TDescant.h"

TAnalysisWriteLoop* TAnalysisWriteLoop::Get(std::string name, std::string output_filename)
{
   if(name.length() == 0) {
      name = "write_loop";
   }

   TAnalysisWriteLoop* loop = static_cast<TAnalysisWriteLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if(output_filename.length() == 0) {
         output_filename = "temp.root";
      }
      loop = new TAnalysisWriteLoop(name, output_filename);
   }

   return loop;
}

TAnalysisWriteLoop::TAnalysisWriteLoop(std::string name, std::string output_filename)
   : StoppableThread(name), fOutputFile(nullptr), fEventTree(nullptr), fOutOfOrderTree(nullptr),
     fOutOfOrderFrag(nullptr), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{

   if(output_filename != "/dev/null") {
      // TPreserveGDirectory preserve;
		TGRSIOptions* options = TGRSIOptions::Get(); // get the pointer before we switch to the new file
      fOutputFile = new TFile(output_filename.c_str(), "RECREATE");
		if(fOutputFile == nullptr || !fOutputFile->IsOpen()) {
			throw std::runtime_error(Form("Failed to open \"%s\"\n", output_filename.c_str()));
		}
      fEventTree  = new TTree("AnalysisTree", "AnalysisTree");
      if(options->SeparateOutOfOrder()) {
         fOutOfOrderTree = new TTree("OutOfOrderTree", "OutOfOrderTree");
         fOutOfOrderFrag = new TFragment;
         fOutOfOrderTree->Branch("Fragment", &fOutOfOrderFrag);
      }
   }
}

TAnalysisWriteLoop::~TAnalysisWriteLoop()
{
   for(auto& elem : fDetMap) {
      delete elem.second;
   }

   Write();
}

void TAnalysisWriteLoop::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<TUnpackedEvent> event;
      fInputQueue->Pop(event);
   }
}

std::string TAnalysisWriteLoop::EndStatus()
{
   std::stringstream ss;
   ss<<Name()<<":\t"<<std::setw(8)<<fItemsPopped<<"/"<<fInputSize + fItemsPopped<<", "
     <<fEventTree->GetEntries()<<" good events";
   if(fOutOfOrderTree != nullptr) {
      ss<<", "<<fOutOfOrderTree->GetEntries()<<" separate fragments out-of-order"<<std::endl;
   } else {
      ss<<std::endl;
   }
   return ss.str();
}

bool TAnalysisWriteLoop::Iteration()
{
   std::shared_ptr<TUnpackedEvent> event;
   fInputSize = fInputQueue->Pop(event);
   if(fInputSize < 0) {
      fInputSize = 0;
   }
   ++fItemsPopped;

   if(fOutOfOrderTree != nullptr && fOutOfOrderQueue->Size() > 0) {
      std::shared_ptr<const TFragment> frag;
      fOutOfOrderQueue->Pop(frag, 0);
      if(frag != nullptr) {
         *fOutOfOrderFrag = *frag;
         fOutOfOrderFrag->ClearTransients();
         std::lock_guard<std::mutex> lock(ttree_fill_mutex);
         fOutOfOrderTree->Fill();
      }
   }

   if(event) {
      WriteEvent(*event);
      return true;
   }
   if(fInputQueue->IsFinished()) {
      return false;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

void TAnalysisWriteLoop::Write()
{

   if(fOutputFile != nullptr) {
		// get all singletons before switching to the output file
		gROOT->cd();
		TGRSIRunInfo* runInfo = TGRSIRunInfo::Get();
		TGRSIOptions* options = TGRSIOptions::Get();
		TPPG* ppg = TPPG::Get();
		TSortingDiagnostics* sortingDiagnostics = TSortingDiagnostics::Get();
		GValue* gValues = GValue::Get();

		fOutputFile->cd();

		fEventTree->Write(fEventTree->GetName(), TObject::kOverwrite);

		if(fOutOfOrderTree != nullptr) {
			fOutOfOrderTree->Write(fOutOfOrderTree->GetName(), TObject::kOverwrite);
		}

		if(GValue::Size() != 0) {
			gValues->Write();
		}
		if(TChannel::GetNumberOfChannels() != 0) {
			TChannel::WriteToRoot();
		}
		runInfo->WriteToRoot(fOutputFile);
		options->AnalysisOptions()->WriteToFile(fOutputFile);
		ppg->Write();

		if(options->WriteDiagnostics()) {
			sortingDiagnostics->Write();
		}

		fOutputFile->Close();
		fOutputFile->Delete();
		gROOT->cd();
	}
}

void TAnalysisWriteLoop::AddBranch(TClass* cls)
{
	if(fDetMap.count(cls) == 0u) {
		// This uses the ROOT dictionaries, so we need to lock the threads.
		TThread::Lock();

		// Make a default detector of that type.
		TDetector* det_p  = reinterpret_cast<TDetector*>(cls->New());
		fDefaultDets[cls] = det_p;

		// Make the TDetector**
		auto** det_pp = new TDetector*;
		*det_pp       = det_p;
		fDetMap[cls]  = det_pp;

		// Make a new branch.
		TBranch* new_branch = fEventTree->Branch(cls->GetName(), cls->GetName(), det_pp);

		// Fill the new branch up to the point where the tree is filled.
		// Explanation:
		//   When TTree::Fill is called, it calls TBranch::Fill for each
		// branch, then increments the number of entries.  We may be
		// adding branches after other branches have already been filled.
		// If the S800 branch has been filled 100 times before the Gretina
		// branch is created, then the next call to TTree::Fill will fill
		// entry 101 of S800, but entry 1 of Gretina, rather than entry
		// 101 of both.
		//   Therefore, we need to fill the new branch as many times as
		// TTree::Fill has been called before.
		std::lock_guard<std::mutex> lock(ttree_fill_mutex);
		for(int i = 0; i < fEventTree->GetEntries(); i++) {
			new_branch->Fill();
		}

		std::cout<<"\r"<<std::string(30, ' ')<<"\rAdded \""<<cls->GetName()<<R"(" branch)"<<std::endl;

		// Unlock after we are done.
		TThread::UnLock();
	}
}

void TAnalysisWriteLoop::WriteEvent(TUnpackedEvent& event)
{
	if(fEventTree != nullptr) {
		// Clear pointers from previous writes.
		// Note that we cannot just set this equal to nullptr,
		//   because ROOT would then construct a new object.
		// This contradicts the ROOT documentation for TBranchElement::SetAddress,
		//   which suggests that a new object would be constructed only when setting the address,
		//   not when filling the TTree.
		for(auto& elem : fDetMap) {
			//*elem.second = fDefaultDets[elem.first];
			(*elem.second)->Clear();
		}

		// Load current events
		for(const auto& det : event.GetDetectors()) {
			TClass* cls = det->IsA();
			try {
				**fDetMap.at(cls) = *(det.get());
			} catch(std::out_of_range& e) {
				AddBranch(cls);
				**fDetMap.at(cls) = *(det.get());
			}
			(*fDetMap.at(cls))->ClearTransients();
			// if(cls == TDescant::Class()) {
			//	for(int i = 0; i < static_cast<TDescant*>(det)->GetMultiplicity(); ++i) {
			//		std::cout<<"Descant hit "<<i<<(static_cast<TDescant*>(det)->GetDescantHit(i)->GetDebugData() == nullptr ?
			//"
			// has no debug data": " has debug data")<<std::endl;
			//	}
			//}
		}

		// Fill
		std::lock_guard<std::mutex> lock(ttree_fill_mutex);
		fEventTree->Fill();
	}
}
