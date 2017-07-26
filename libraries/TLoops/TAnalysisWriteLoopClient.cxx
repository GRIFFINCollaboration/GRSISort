#include "TAnalysisWriteLoopClient.h"

#include "TGRSIOptions.h"
#include "TFragment.h"

TAnalysisWriteLoopClient* TAnalysisWriteLoopClient::Get(std::string name, std::string outputFilename)
{
   if(name.length() == 0) {
      name = "write_loop";
   }

   TAnalysisWriteLoopClient* loop = static_cast<TAnalysisWriteLoopClient*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if(outputFilename.length() == 0) {
         outputFilename = "temp.root";
      }
      loop = new TAnalysisWriteLoopClient(name, outputFilename);
   }

   return loop;
}

TAnalysisWriteLoopClient::TAnalysisWriteLoopClient(std::string name, std::string outputFilename)
   : StoppableThread(name),fOutputFile(nullptr), fEventTree(nullptr), fOutOfOrderTree(nullptr), fOutOfOrderFrag(nullptr),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
   fOutputFile = static_cast<TParallelMergingFile*>(TFile::Open(Form("%s?pmerge=localhost:9090", outputFilename.c_str()), "RECREATE"));
	if(fOutputFile == nullptr) {
		std::cerr<<"client: Could not establish a connection with server 'localhost':9090"<<std::endl;
		throw;
	}
   fOutputFile->Write();
   fOutputFile->UploadAndReset();       // We do this early to get assigned an index.

	fEventTree  = new TTree("AnalysisTree", "AnalysisTree");
	if(TGRSIOptions::Get()->SeparateOutOfOrder()) {
		fOutOfOrderTree = new TTree("OutOfOrderTree", "OutOfOrderTree");
		fOutOfOrderFrag = new TFragment;
		fOutOfOrderTree->Branch("Fragment", &fOutOfOrderFrag);
	}
}

TAnalysisWriteLoopClient::~TAnalysisWriteLoopClient()
{
   for(auto& elem : fDetMap) {
      delete elem.second;
   }

   Write();
}

void TAnalysisWriteLoopClient::ClearQueue()
{
   while(fInputQueue->Size() != 0u) {
      std::shared_ptr<TUnpackedEvent> event;
      fInputQueue->Pop(event);
   }
}

std::string TAnalysisWriteLoopClient::EndStatus()
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

bool TAnalysisWriteLoopClient::Iteration()
{
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

   if(event != nullptr) {
		WriteEvent(event);
      return true;
   }

   if(fInputQueue->IsFinished()) {
      return false;
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return true;
}

void TAnalysisWriteLoopClient::Write()
{
   if(fOutputFile != nullptr) {
      fOutputFile->cd();

      fEventTree->Write(fEventTree->GetName(), TObject::kOverwrite);

      if(fOutOfOrderTree != nullptr) {
         fOutOfOrderTree->Write(fOutOfOrderTree->GetName(), TObject::kOverwrite);
      }

      if(GValue::Size() != 0) {
         GValue::Get()->Write();
      }
      if(TChannel::GetNumberOfChannels() != 0) {
         TChannel::WriteToRoot();
      }
      TGRSIRunInfo::Get()->WriteToRoot(fOutputFile);
      TGRSIOptions::Get()->AnalysisOptions()->WriteToFile(fOutputFile);
      TPPG::Get()->Write();

      if(TGRSIOptions::Get()->WriteDiagnostics()) {
         TSortingDiagnostics::Get()->Write();
      }

      fOutputFile->Close();
      fOutputFile->Delete();
   }
}

void TAnalysisWriteLoopClient::OnEnd()
{
	fFile->Write();
	delete fFile;

	return true;
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
		auto** det_pp = new TDetector*;
		*det_pp       = det_p;
		fDetMap[cls]  = det_pp;

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

		std::cout<<"\r"<<std::string(30, ' ')<<"\rAdded \""<<cls->GetName()<<R"(" branch)"<<std::endl;

		// Unlock after we are done.
		TThread::UnLock();
	}
}

void TAnalysisWriteLoopClient::WriteEvent(TUnpackedEvent& event)
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

int main(int argc, char** argv) {
	TThread::Initialize();

	std::future<bool> serverFuture = std::async(std::launch::async, server);

	std::vector<std::future<bool> > clientFutures;
	clientFutures.push_back(std::async(std::launch::async, client));
	clientFutures.push_back(std::async(std::launch::async, client));
	clientFutures.push_back(std::async(std::launch::async, client));
	clientFutures.push_back(std::async(std::launch::async, client));

	if(!serverFuture.get()) {
		std::cout<<"Server failed!"<<std::endl;
	} else {
		std::cout<<"Server finished successful!"<<std::endl;
	}

	for(size_t i = 0; i < clientFutures.size(); ++i) {
		if(!clientFutures[i].get()) {
			std::cout<<"Client "<<i<<" failed!"<<std::endl;
		} else {
			std::cout<<"Client "<<i<<" finished successful!"<<std::endl;
		}
	}

	return 0;
}
