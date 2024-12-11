#include "TRnTupleWriteLoop.h"

#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"
#include "TMessage.h"
#include "TSocket.h"
#include "TMemFile.h"
#include "TFileMerger.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TFileCacheWrite.h"
#include "TROOT.h"
#include "THashTable.h"
#include "TClassEdit.h"

#include "GValue.h"
#include "TChannel.h"
#include "TRunInfo.h"
#include "TGRSIOptions.h"
#include "TTreeFillMutex.h"
#include "TSortingDiagnostics.h"

TRnTupleWriteLoop* TRnTupleWriteLoop::Get(std::string name, std::string outputFilename)
{
   if(name.length() == 0) {
      name = "write_loop";
   }

   TRnTupleWriteLoop* loop = static_cast<TRnTupleWriteLoop*>(StoppableThread::Get(name));
   if(loop == nullptr) {
      if(outputFilename.length() == 0) {
         outputFilename = "temp.root";
      }
      loop = new TRnTupleWriteLoop(name, outputFilename);
   }

   return loop;
}

TRnTupleWriteLoop::TRnTupleWriteLoop(std::string name, std::string outputFilename)
   : StoppableThread(name),
     fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TUnpackedEvent>>>()),
     fOutOfOrderQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment>>>())
{
	auto model = ROOT::Experimental::RNTupleModel::CreateBare();
	fRNTupleWriter = ROOT::Experimental::RNTupleWriter::Recreate(std::move(model), "AnalysisTree", outputFilename);

	//if(TGRSIOptions::Get()->SeparateOutOfOrder()) {
	//	fOutOfOrderTree = new TTree("OutOfOrderTree", "OutOfOrderTree");
   //   fOutOfOrderFrag = new TFragment;
   //   fOutOfOrderTree->Branch("Fragment", &fOutOfOrderFrag);
   //   fOutOfOrder = true;
   //}
}

TRnTupleWriteLoop::~TRnTupleWriteLoop()
{
	for(auto& elem : fDetMap) {
      delete elem.second;
   }
}

void TRnTupleWriteLoop::ClearQueue()
{
	while(fInputQueue->Size() != 0u) {
		std::shared_ptr<TUnpackedEvent> event;
		fInputQueue->Pop(event);
	}
}

std::string TRnTupleWriteLoop::EndStatus()
{
	std::stringstream ss;
	ss<<Name()<<":\t"<<std::setw(8)<<ItemsPopped()<<"/"<<InputSize() + ItemsPopped()<<", "
		<<"??? good events"<<std::endl;
	return ss.str();
}

void TRnTupleWriteLoop::OnEnd()
{
	Write();
}

bool TRnTupleWriteLoop::Iteration()
{
   std::shared_ptr<TUnpackedEvent> event;
   InputSize(fInputQueue->Pop(event));
   if(InputSize() < 0) {
      InputSize(0);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
   } else {
		IncrementItemsPopped();
	}

	//if(fOutOfOrder) {
   //   std::shared_ptr<const TFragment> frag;
   //   fOutOfOrderQueue->Pop(frag, 0);
   //   if(frag != nullptr) {
	//		*fOutOfOrderFrag = *frag;
   //      fOutOfOrderFrag->ClearTransients();
   //      std::lock_guard<std::mutex> lock(ttree_fill_mutex);
   //      fOutOfOrderTree->Fill();
	//	}
	//}

	if(event != nullptr) {
		WriteEvent(event);
		return true;
	}

	if(fInputQueue->IsFinished()) {
		return false;
	}
	return true;
}

void TRnTupleWriteLoop::Write()
{
	//if(fOutputFile != nullptr) {
	//	gROOT->cd();
	//	TRunInfo* runInfo = TRunInfo::Get();
	//	TGRSIOptions* options = TGRSIOptions::Get();
	//	TPPG* ppg = TPPG::Get();
	//	TSortingDiagnostics* diag = TSortingDiagnostics::Get();

	//	fOutputFile->cd();
	//	if(GValue::Size() != 0) {
	//		GValue::Get()->Write("Values", TObject::kOverwrite);
	//	}
	//	if(TChannel::GetNumberOfChannels() != 0) {
	//		TChannel::WriteToRoot();
	//	}
	//	runInfo->WriteToRoot(fOutputFile);
	//	options->AnalysisOptions()->WriteToFile(fOutputFile);
	//	ppg->Write("PPG");

	//	if(options->WriteDiagnostics()) {
	//		diag->Write("SortingDiagnostics", TObject::kOverwrite);
	//	}

	//	fOutputFile->Write();
	//	delete fOutputFile;
	//	fOutputFile = nullptr;
	//}
}

void TRnTupleWriteLoop::AddBranch(TClass* cls)
{
   if(fDetMap.count(cls) == 0u) {
      // This uses the ROOT dictionaries, so we need to lock the threads.
      TThread::Lock();

      // Make a default detector of that type.
      TDetector* det_p  = reinterpret_cast<TDetector*>(cls->New());
      fDefaultDets[cls] = det_p;

      // Add to our local map
      auto det_pp  = new TDetector*;
      *det_pp      = det_p;
      fDetMap[cls] = det_pp;

      std::cout << std::endl
                << std::string(30, ' ') << Name() << ": adding \"" << cls->GetName() << "\" branch" << std::endl;
      // Make a new field.
      auto updater = fRNTupleWriter->CreateModelUpdater();
      try {
         auto newFieldResult = ROOT::Experimental::Detail::RFieldBase::Create(cls->GetName(), cls->GetName());
         if(!newFieldResult) {
            std::cout << "Failed to create field using \"" << cls->GetName() << "\"?" << std::endl;
         }
         auto newField = newFieldResult.Unwrap();
         updater->BeginUpdate();
         updater->AddField(std::move(newField));
         updater->CommitUpdate();
         std::cout << "Model: " << fRNTupleWriter->GetModel()->GetDescription() << std::endl;
      } catch(ROOT::Experimental::RException& e) {
         std::cout << "Failed to create field " << cls->GetName() << ", normalized " << TClassEdit::CleanType(cls->GetName()) << ", canonical " << TClassEdit::ResolveTypedef(cls->GetName()) << ": " << e.what() << std::endl;
			throw e;
      }

      std::cout << "\r" << std::string(30, ' ') << "\r" << Name() << ": added \"" << cls->GetName() << R"(" branch)" << std::endl;

      // Unlock after we are done.
      TThread::UnLock();
   }
}

void TRnTupleWriteLoop::WriteEvent(std::shared_ptr<TUnpackedEvent>& event)
{
	// Clear pointers from previous writes.
	// Note that we cannot just set this equal to nullptr,
	//   because ROOT would then construct a new object.
	// This contradicts the ROOT documentation for TBranchElement::SetAddress,
	//   which suggests that a new object would be constructed only when setting the address,
	//   not when filling the TTree.
	for(auto& elem : fDetMap) {
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

	// Fill
	std::lock_guard<std::mutex> lock(ttree_fill_mutex);
	fRNTupleWriter->Fill();
}

