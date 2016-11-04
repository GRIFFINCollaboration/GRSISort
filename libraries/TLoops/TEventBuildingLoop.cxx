#include "TEventBuildingLoop.h"

#include "TGRSIOptions.h"
#include "TSortingDiagnostics.h"

#include <chrono>
#include <thread>

ClassImp(TEventBuildingLoop)

TEventBuildingLoop *TEventBuildingLoop::Get(std::string name, EBuildMode mode) {
	if(name.length()==0) {
		name = "build_loop";
	}

	TEventBuildingLoop *loop = (TEventBuildingLoop*)StoppableThread::Get(name);
	if(!loop) {
		loop = new TEventBuildingLoop(name, mode);
	}
	return loop;
}

TEventBuildingLoop::TEventBuildingLoop(std::string name, EBuildMode mode)
	: StoppableThread(name),
	fInputQueue(std::make_shared<ThreadsafeQueue<std::shared_ptr<const TFragment> > >()),
	fOutputQueue(std::make_shared<ThreadsafeQueue<std::vector<std::shared_ptr<const TFragment> > > >()),
	fBuildMode(mode), fSortingDepth(10000),
	fBuildWindow(200), fPreviousSortingDepthError(false) {

	switch(fBuildMode) {
		case kTimestamp:
			fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment> a, std::shared_ptr<const TFragment> b) {
					return a->GetTimeStamp() < b->GetTimeStamp();
					});
			break;

		case kTriggerId:
			fOrdered = decltype(fOrdered)([](std::shared_ptr<const TFragment> a, std::shared_ptr<const TFragment> b) {
					return a->GetTriggerId() < b->GetTriggerId();
					});
			break;
	}
}

TEventBuildingLoop::~TEventBuildingLoop() { }

void TEventBuildingLoop::ClearQueue() {
	std::shared_ptr<const TFragment> single_event;
	while(fInputQueue->Size()) {
		fInputQueue->Pop(single_event);
	}

	std::vector<std::shared_ptr<const TFragment> > event;
	while(fOutputQueue->Size()){
		fOutputQueue->Pop(event);
	}
}

bool TEventBuildingLoop::Iteration(){
	// Pull something off of the input queue.
	std::shared_ptr<const TFragment> input_frag = NULL;
	fInputSize = fInputQueue->Pop(input_frag, 0);
	if(fInputSize < 0) fInputSize = 0;

	if(input_frag) {
		++fItemsPopped;
		fOrdered.insert(input_frag);
		if(fOrdered.size() < fSortingDepth) {
			// Got a new event, but we want to have more to sort
			return true;
		} else {
			// Got a new event, and we have enough to sort.
		}
	} else {
		if(!fInputQueue->IsFinished()) {
			// If the parent is live, wait for it
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			return true;
		} else if (fOrdered.size() == 0) {
			// Parent is dead, and we have passed on all events
			if(fNextEvent.size()) {
				fOutputQueue->Push(fNextEvent);
			}
			fOutputQueue->SetFinished();
			return false;
		} else {
			// Parent is dead, but we still have items.
			// Continue through the function to process them.
		}
	}

	// We have data, and we want to add it to the next fragment;
	std::shared_ptr<const TFragment> next_fragment = *fOrdered.begin();
	fOrdered.erase(fOrdered.begin());
	CheckBuildCondition(next_fragment);
	fNextEvent.push_back(next_fragment);

	return true;
}

void TEventBuildingLoop::CheckBuildCondition(std::shared_ptr<const TFragment> frag) {
	switch(fBuildMode) {
		case kTimestamp:
			CheckTimestampCondition(frag);
			break;

		case kTriggerId:
			CheckTriggerIdCondition(frag);
			break;
	}
}

void TEventBuildingLoop::CheckTimestampCondition(std::shared_ptr<const TFragment> frag) {
	long timestamp = frag->GetTimeStamp();
	long event_start = (fNextEvent.size() ?
			( TGRSIOptions::Get()->StaticWindow() ? fNextEvent[0]->GetTimeStamp() : fNextEvent.back()->GetTimeStamp() ) :
			timestamp);

	if(timestamp < event_start) {
		TSortingDiagnostics::Get()->OutOfOrder(timestamp, event_start, frag->GetEntryNumber());
		if(!fPreviousSortingDepthError) {
			std::cerr <<std::endl<< "Sorting depth of " << fSortingDepth << " was insufficient. timestamp: " << timestamp << " Last: " << event_start << " \n"
				<< "Not all events were built correctly" << std::endl;
			std::cerr << "Please increase sort depth with --sort-depth=N" << std::endl;
			fPreviousSortingDepthError = true;
		}
	}

	//save timestamp every <BuildWindow> fragments
	if(frag->GetEntryNumber()%(TGRSIOptions::Get()->SortDepth()) == 0) {
		TSortingDiagnostics::Get()->AddTimeStamp(event_start);
	}

	if(timestamp > event_start + fBuildWindow ||
			timestamp < event_start - fBuildWindow) {
		fOutputQueue->Push(fNextEvent);
		fNextEvent.clear();
	}
}

void TEventBuildingLoop::CheckTriggerIdCondition(std::shared_ptr<const TFragment> frag) {
	long trigger_id = frag->GetTriggerId();
	long current_trigger_id = (fNextEvent.size() ?
			fNextEvent[0]->GetTriggerId() :
			trigger_id);

	if(trigger_id < current_trigger_id) {
		TSortingDiagnostics::Get()->OutOfOrder(trigger_id, current_trigger_id, frag->GetEntryNumber());
		if(!fPreviousSortingDepthError) {
			std::cerr <<std::endl<< "Sorting depth of " << fSortingDepth << " was insufficient.\n"
				<< "Not all events were built correctly" << std::endl;
			std::cerr << "Trigger id #" << trigger_id << " was incorrectly sorted before "
				<< "trigger id #" << current_trigger_id << std::endl;
			std::cerr << "Please increase sort depth with --sort-depth=N" << std::endl;
			fPreviousSortingDepthError = true;
		}
	}

	//save trigger id every <BuildWindow> fragments
	if(frag->GetEntryNumber()%(TGRSIOptions::Get()->SortDepth()) == 0) {
		TSortingDiagnostics::Get()->AddTimeStamp(current_trigger_id);
	}

	if(trigger_id != current_trigger_id) {
		fOutputQueue->Push(fNextEvent);
		fNextEvent.clear();
	}
}

std::string TEventBuildingLoop::EndStatus() {
	std::stringstream ss;
	ss<<fInputQueue->Name()<<": "<<fItemsPopped<<"/"<<fInputQueue->ItemsPopped()<<" items popped"<<std::endl;

	return ss.str();
}
