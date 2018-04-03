#include "TAnalysisWriteLoopClient.h"

#include "TGRSIOptions.h"
#include "TFragment.h"
#include "TTreeFillMutex.h"

std::map<TClass*, TDetector**> TAnalysisWriteLoopClient::fGlobalDetMap;
//std::mutex TAnalysisWriteLoopClient::fGlobalMapMutex;

TAnalysisWriteLoopClient::TAnalysisWriteLoopClient(std::string name, std::string outputFilename, Int_t localPort)
   : StoppableThread(name),fOutputFile(nullptr), fEventTree(nullptr), fOutOfOrderTree(nullptr), fOutOfOrderFrag(nullptr),
	  fOutOfOrder(false), fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
	fFirstClient = (name.compare("write_client_0") == 0);
   fOutputFile = static_cast<TParallelMergingFile*>(TFile::Open(Form("%s?pmerge=localhost:%d", outputFilename.c_str(), localPort), "RECREATE"));
	if(fOutputFile == nullptr) {
		std::cerr<<"client: Could not establish a connection with server 'localhost:"<<localPort<<"'"<<std::endl;
		throw;
	}
   fOutputFile->Write();

	fEventTree  = new TTree("AnalysisTree", "AnalysisTree");
	if(TGRSIOptions::Get()->SeparateOutOfOrder()) {
		fOutOfOrderTree = new TTree("OutOfOrderTree", "OutOfOrderTree");
		fOutOfOrderFrag = new TFragment;
		fOutOfOrderTree->Branch("Fragment", &fOutOfOrderFrag);
		fOutOfOrder = true;
	}
}

TAnalysisWriteLoopClient::~TAnalysisWriteLoopClient()
{
   for(auto& elem : fDetMap) {
      delete elem.second;
   }
}

void TAnalysisWriteLoopClient::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<TUnpackedEvent> event;
      fInputQueue->Pop(event);
   }
}

bool TAnalysisWriteLoopClient::Iteration()
{
	std::ofstream outfile;
	outfile.open("debug.txt", std::ios_base::app);

   std::shared_ptr<TUnpackedEvent> event;
   fInputSize = fInputQueue->Pop(event);
   if(fInputSize < 0) {
      fInputSize = 0;
   }
   ++fItemsPopped;

   if(fOutOfOrder) {
      std::shared_ptr<const TFragment> frag;
      fOutOfOrderQueue->Pop(frag, 0);
      if(frag != nullptr) {
			*fOutOfOrderFrag = *frag;
			fOutOfOrderFrag->ClearTransients();
			std::lock_guard<std::mutex> lock(ttree_fill_mutex);
			fOutOfOrderTree->Fill();
      }
   }

	outfile<<Name()<<" "<<fInputSize<<" "<<fItemsPopped<<" "<<event.get();

   if(event != nullptr) {
		WriteEvent(event);
		outfile<<" got event"<<std::endl;
      return true;
   }

   if(fInputQueue->IsFinished()) {
		outfile<<" done?"<<std::endl;
      return false;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	outfile<<" waiting"<<std::endl;
   return true;
}

void TAnalysisWriteLoopClient::OnEnd()
{
	fOutputFile->Write();
	delete fOutputFile;
}

void TAnalysisWriteLoopClient::AddBranch(TClass* cls)
{
	if(fDetMap.count(cls) == 0u) {
		// This uses the ROOT dictionaries, so we need to lock the threads.
		TThread::Lock();

		// Make a default detector of that type.
		TDetector* det_p  = reinterpret_cast<TDetector*>(cls->New());
		fDefaultDets[cls] = det_p;

		// Make the TDetector**
		auto det_pp   = new TDetector*;
		*det_pp       = det_p;
		fDetMap[cls]  = det_pp;

		// check if this detector class exists in the global map
		{
			//std::cout<<std::endl<<Name()<<" locking"<<std::endl;
			//std::lock_guard<std::mutex> lock(fGlobalMapMutex);
			if(fGlobalDetMap.count(cls) == 0u) {
				fGlobalDetMap[cls] = det_pp;
			}
			//std::cout<<std::endl<<Name()<<" done"<<std::endl;
		}

		// Make a new branch.
		TBranch* newBranch = fEventTree->Branch(cls->GetName(), cls->GetName(), det_pp);

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
			newBranch->Fill();
		}

		if(fFirstClient) {
			std::cout<<"\r"<<std::string(30, ' ')<<"\r"<<Name()<<": added \""<<cls->GetName()<<R"(" branch)"<<std::endl;
		}

		// Unlock after we are done.
		TThread::UnLock();
	}
}

void TAnalysisWriteLoopClient::WriteEvent(std::shared_ptr<TUnpackedEvent>& event)
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
		for(const auto& det : event->GetDetectors()) {
			TClass* cls = det->IsA();
			try {
				**fDetMap.at(cls) = *(det.get());
			} catch(std::out_of_range& e) {
				AddBranch(cls);
				**fDetMap.at(cls) = *(det.get());
			}
			(*fDetMap.at(cls))->ClearTransients();
		}

		// Check if other clients found new detectors
		{
			//std::cout<<std::endl<<Name()<<" locking for AddBranch"<<std::endl;
			//std::lock_guard<std::mutex> lock(fGlobalMapMutex);
			if(fDetMap.size() != fGlobalDetMap.size()) {
				// loop over global map of detectors and add those we don't have in our list
				for(auto const& det : fGlobalDetMap) {
					// AddBranch checks if detector exists
					//std::cout<<std::endl<<Name()<<" Adding branch "<<det.first->GetName()<<std::endl;
					AddBranch(det.first);
				}
			}
			//std::cout<<std::endl<<Name()<<" done with AddBranch"<<std::endl;
		}

		// Fill
		std::lock_guard<std::mutex> lock(ttree_fill_mutex);
		fEventTree->Fill();

		// write file every 100000 popped events (this sends the events to the server)
		if(fItemsPopped%100000 == 0) {
			fOutputFile->Write();
		}
	}
}
