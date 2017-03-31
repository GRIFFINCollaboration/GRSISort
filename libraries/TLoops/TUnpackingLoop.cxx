#include "TUnpackingLoop.h"

#include <chrono>
#include <thread>
#include <sstream>

#include "TGRSIOptions.h"
#include "TLstEvent.h"

//ClassImp(TUnpackingLoop)

TUnpackingLoop *TUnpackingLoop::Get(std::string name) {
	if(name.length()==0) {
		name = "unpacking_loop";
	}

	TUnpackingLoop *loop = static_cast<TUnpackingLoop*>(StoppableThread::Get(name));
	if(!loop) {
		loop = new TUnpackingLoop(name);
	}
	return loop;
}

TUnpackingLoop::TUnpackingLoop(std::string name)
	: StoppableThread(name),
	fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<TRawEvent> > >()),
	fFragsReadFromRaw(0),fGoodFragsRead(0), fEvaluateDataType(true) {
}

TUnpackingLoop::~TUnpackingLoop() { }

void TUnpackingLoop::ClearQueue() {
	std::shared_ptr<TRawEvent> singleEvent;
	while(fInputQueue->Size()) {
		fInputQueue->Pop(singleEvent);
	}

   fParser.ClearQueue();
	//while(fParser.GoodOutputQueue()->Size()){
	//	fParser.GoodOutputQueue()->Pop(frag);
	//	delete frag;
	//}
}

bool TUnpackingLoop::Iteration(){
	std::shared_ptr<TRawEvent> event;
	int error = fInputQueue->Pop(event);
	if(error<0) {
		fInputSize = 0;
		if(fInputQueue->IsFinished()){
			// Source is dead, push the last event and stop.
			fParser.SetFinished();
			BadOutputQueue()->SetFinished();
			ScalerOutputQueue()->SetFinished();
			return false;
		} else {
			// Wait for the source to give more data.
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			return true;
		}
	}
	if(fEvaluateDataType) {
		fDataType = (event->IsA() == TLstEvent::Class()) ? kLst:kMidas;
		fEvaluateDataType = false;
	}
	if(fDataType == kLst) {
		fParser.SetStatusVariables(&fItemsPopped, &fInputSize);
	} else {
		fInputSize = error;
		++fItemsPopped;
	}

	int frags = event->Process(fParser);
	if(frags>0) {
		fFragsReadFromRaw += frags;
		fGoodFragsRead += frags;
	} else  {
		fFragsReadFromRaw += 1;   // if the midas bank fails, we assume it only had one frag in it... this is just used for a print statement.
		if(!TGRSIOptions::Get()->SuppressErrors()) {
			event->SetBankList();
			event->Print(Form("a%i",(-1*frags)-1));
		}
	}

	return true;
}

std::string TUnpackingLoop::EndStatus() {
	std::stringstream ss;
	if(fFragsReadFromRaw > 0) {
		ss<<"\r"<<Name()<<":\t"<<fGoodFragsRead<<" good fragments out of "<<fFragsReadFromRaw<<" fragments => "<<(100.*fGoodFragsRead)/fFragsReadFromRaw<<"% passed"<<std::endl;
	} else {
		ss<<"\rno fragments read from midas => none parsed!"<<std::endl;
	}
	ss<<fParser.OutputQueueStatus();
	return ss.str();
}

