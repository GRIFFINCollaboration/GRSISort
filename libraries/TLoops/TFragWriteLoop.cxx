#include "TFragWriteLoop.h"

#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

#include "TFile.h"
#include "TThread.h"

#include "GValue.h"
#include "TChannel.h"
#include "TGRSIRunInfo.h"
#include "TThread.h"
#include "TTreeFillMutex.h"
#include "TGRSIOptions.h"
#include "TParsingDiagnostics.h"

TFragWriteLoop* TFragWriteLoop::Get(std::string name, std::string fOutputFilename){
  if(name.length()==0){
    name = "write_loop";
  }

  StoppableThread* thread = StoppableThread::Get(name);
  if(!thread){
    if(fOutputFilename.length()==0){
      fOutputFilename = "temp.root";
    }
    thread = new TFragWriteLoop(name,fOutputFilename);
  }

  return dynamic_cast<TFragWriteLoop*>(thread);
}

TFragWriteLoop::TFragWriteLoop(std::string name, std::string fOutputFilename)
  : StoppableThread(name),
    fOutputFile(NULL), fEventTree(NULL), fBadEventTree(NULL), fScalerTree(NULL),
	 fEventAddress(NULL), fBadEventAddress(NULL), fScalerAddress(NULL),
    fItemsHandled(0),fInputQueueSize(0),
    fInputQueue(std::make_shared<ThreadsafeQueue<TFragment*> >()),
    fBadInputQueue(std::make_shared<ThreadsafeQueue<TFragment*> >()),
    fScalerInputQueue(std::make_shared<ThreadsafeQueue<TEpicsFrag*> >()),
    fOutputQueue(std::make_shared<ThreadsafeQueue<TFragment*> >()) {

  if(fOutputFilename != "/dev/null"){
    TThread::Lock();

    //TPreserveGDirectory preserve;
    fOutputFile = new TFile(fOutputFilename.c_str(),"RECREATE");

    fEventTree = new TTree("FragmentTree","FragmentTree");
    fEventTree->Branch("TFragment", &fEventAddress);

    fBadEventTree = new TTree("BadFragmentTree","BadFragmentTree");
    fBadEventTree->Branch("TFragment", &fBadEventAddress);

    fScalerTree = new TTree("EpicsTree","EpicsTree");
    fScalerTree->Branch("TEpicsFrag", &fScalerAddress);

    TThread::UnLock();
  }

}

TFragWriteLoop::~TFragWriteLoop() {
  Write();
}

void TFragWriteLoop::ClearQueue() {
  while(fInputQueue->Size()){
    TFragment* event = NULL;
    fInputQueue->Pop(event);
    if(event){
      delete event;
    }
  }

  while(fOutputQueue->Size()){
    TFragment* event = NULL;
    fOutputQueue->Pop(event);
    if(event){
      delete event;
    }
  }
}

std::string TFragWriteLoop::Status() {
	std::stringstream ss;
	ss<<Name()<<":\t"<<std::setw(8)<<GetItemsPushed()<<"/"<<(fInputQueueSize>0 ? fInputQueueSize+GetItemsPushed():GetItemsPushed());
	return ss.str();
}

std::string TFragWriteLoop::EndStatus() {
	std::stringstream ss;
	ss<<"\r"<<Name()<<":\t"<<std::setw(8)<<GetItemsPushed()<<"/"<<(fInputQueueSize>0 ? fInputQueueSize+GetItemsPushed():GetItemsPushed())<<std::endl;;
	return ss.str();
}

bool TFragWriteLoop::Iteration() {
	TFragment* event = NULL;
	fInputQueueSize = fInputQueue->Pop(event,0);

	TFragment* badEvent = NULL;
	fBadInputQueue->Pop(badEvent,0);

	TEpicsFrag* scaler = NULL;
	fScalerInputQueue->Pop(scaler,0);

	bool has_anything = event || badEvent || scaler;
	bool all_parents_dead = (fInputQueue->IsFinished() && fBadInputQueue->IsFinished() &&
			fScalerInputQueue->IsFinished());

	if(event) {
		WriteEvent(*event);
		fOutputQueue->Push(event);
		fItemsHandled++;
	}

	if(badEvent) {
		WriteBadEvent(*badEvent);
		delete badEvent;
	}

	if(scaler) {
		WriteScaler(*scaler);
		delete scaler;
	}

	if(has_anything) {
		return true;
	} else if(all_parents_dead) {
		fOutputQueue->SetFinished();
		return false;
	} else {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		return true;
	}
}

void TFragWriteLoop::Write() {
	if(fOutputFile){
		fOutputFile->cd();
		fEventTree->Write(fEventTree->GetName(), TObject::kOverwrite);
		fBadEventTree->Write(fBadEventTree->GetName(), TObject::kOverwrite);
		fScalerTree->Write(fScalerTree->GetName(), TObject::kOverwrite);
		if(GValue::Size()) {
			GValue::Get()->Write();
		}

		if(TChannel::GetNumberOfChannels()) {
			//TChannel::GetDefaultChannel()->Write();
			TChannel::WriteToRoot();
		}

		TGRSIRunInfo::Get()->WriteToRoot(fOutputFile);
		TPPG::Get()->Write();

		if(TGRSIOptions::Get()->WriteDiagnostics()) {
			TParsingDiagnostics::Get()->ReadPPG(TPPG::Get());
			TParsingDiagnostics::Get()->Write();
		}

		fOutputFile->Close();
		fOutputFile->Delete();
	}
}

void TFragWriteLoop::WriteEvent(TFragment& event) {
	if(fEventTree){
		fEventAddress = &event;
		std::lock_guard<std::mutex> lock(ttree_fill_mutex);
		fEventTree->Fill();
		fEventAddress = NULL;
	}
}

void TFragWriteLoop::WriteBadEvent(TFragment& event) {
	if(fBadEventTree){
		fBadEventAddress = &event;
		std::lock_guard<std::mutex> lock(ttree_fill_mutex);
		fBadEventTree->Fill();
		fBadEventAddress = NULL;
	}
}

void TFragWriteLoop::WriteScaler(TEpicsFrag& scaler) {
	if(fScalerTree){
		fScalerAddress = &scaler;
		std::lock_guard<std::mutex> lock(ttree_fill_mutex);
		fScalerTree->Fill();
		fScalerAddress = NULL;
	}
}


