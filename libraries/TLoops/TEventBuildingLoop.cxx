#include "TEventBuildingLoop.h"

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
    input_queue(std::make_shared<ThreadsafeQueue<TFragment*> >()),
    output_queue(std::make_shared<ThreadsafeQueue<std::vector<TFragment*> > >()),
    build_mode(mode), sorting_depth(10000),
    fBuild_window(200) {

  switch(build_mode) {
    case kTimestamp:
      ordered = decltype(ordered)([](TFragment* a, TFragment* b) {
          return a->GetTimeStamp() < b->GetTimeStamp();
        });
      break;

    case kTriggerId:
      ordered = decltype(ordered)([](TFragment* a, TFragment* b) {
          return a->GetTriggerId() < b->GetTriggerId();
        });
      break;
  }
}

TEventBuildingLoop::~TEventBuildingLoop() { }

void TEventBuildingLoop::ClearQueue() {
  TFragment* single_event;
  while(input_queue->Size()) {
    input_queue->Pop(single_event);
    if(single_event) {
      delete single_event;
    }
  }

  std::vector<TFragment*> event;
  while(output_queue->Size()){
    output_queue->Pop(event);
    for(auto frag : event) {
      delete frag;
    }
  }
}

bool TEventBuildingLoop::Iteration(){
  // Pull something off of the input queue.
  TFragment* input_frag = NULL;
  input_queue->Pop(input_frag, 0);
  if(input_frag) {
    ordered.insert(input_frag);
    if(ordered.size() < sorting_depth) {
      // Got a new event, but we want to have more to sort
      return true;
    } else {
      // Got a new event, and we have enough to sort.
    }

  } else {
    if(!input_queue->IsFinished()) {
      // If the parent is live, wait for it
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      return true;
    } else if (ordered.size() == 0) {
      // Parent is dead, and we have passed on all events
      if(next_event.size()) {
        output_queue->Push(next_event);
      }
      output_queue->SetFinished();
      return false;
    } else {
      // Parent is dead, but we still have items.
      // Continue through the function to process them.
    }
  }

  // We have data, and we want to add it to the next fragment;
  TFragment* next_fragment = *ordered.begin();
  ordered.erase(ordered.begin());
  CheckBuildCondition(next_fragment);
  next_event.push_back(next_fragment);

  return true;
}

void TEventBuildingLoop::CheckBuildCondition(TFragment* frag) {
  switch(build_mode) {
    case kTimestamp:
      CheckTimestampCondition(frag);
      break;

    case kTriggerId:
      CheckTriggerIdCondition(frag);
      break;
  }
}

void TEventBuildingLoop::CheckTimestampCondition(TFragment* frag) {
  long timestamp = frag->GetTimeStamp();
  long event_start = (next_event.size() ?
                      next_event[0]->GetTimeStamp() :
                      timestamp);

  if(timestamp < event_start) {
    std::cerr << "Sorting depth of " << sorting_depth << " was insufficient. timestamp: " << timestamp << " Last: " << event_start << " \n"
              << "Not all events were built correctly" << std::endl;
  }

  if(timestamp > event_start + fBuild_window ||
     timestamp < event_start - fBuild_window) {
    output_queue->Push(next_event);
    next_event.clear();
  }
}

void TEventBuildingLoop::CheckTriggerIdCondition(TFragment* frag) {
  long trigger_id = frag->GetTriggerId();
  long current_trigger_id = (next_event.size() ?
                             next_event[0]->GetTriggerId() :
                             trigger_id);

  if(trigger_id < current_trigger_id) {
    std::cerr << "Sorting depth of " << sorting_depth << " was insufficient.\n"
              << "Not all events were built correctly" << std::endl;
    std::cerr << "Trigger id #" << trigger_id << " was incorrectly sorted before "
              << "trigger id #" << current_trigger_id << std::endl;
    std::cerr << "Please increase sort depth with --sort-depth=N" << std::endl;
  }

  if(trigger_id != current_trigger_id) {
    output_queue->Push(next_event);
    next_event.clear();
  }
}
