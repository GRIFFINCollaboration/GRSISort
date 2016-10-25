#include "TUnpackedEvent.h"

#include "TClass.h"
#include "TDetector.h"
#include "TChannel.h"

TUnpackedEvent::TUnpackedEvent() { }

TUnpackedEvent::~TUnpackedEvent() { }

void TUnpackedEvent::Build() {
  for(auto frag : fFragments) {
    TChannel* channel = TChannel::GetChannel(frag->GetAddress());
    if(!channel) {
      continue;
    }

    TClass* detClass = channel->GetClassType();
    if(!detClass) {
      continue;
    }

    GetDetector(detClass, true)->AddFragment(frag, channel);
  }

  BuildHits();
  ClearRawData();
}

void TUnpackedEvent::AddRawData(std::shared_ptr<TFragment> frag) {
  fFragments.push_back(frag);
}

void TUnpackedEvent::ClearRawData() {
  fFragments.clear();
}

void TUnpackedEvent::BuildHits() {
  for(auto det : fDetectors) {
    det->BuildHits();
  }
}

std::shared_ptr<TDetector>  TUnpackedEvent::GetDetector(TClass* cls, bool make_if_not_found) {
  for(auto det : fDetectors) {
    if(det->IsA() == cls) {
      return det;
    }
  }

  if(make_if_not_found) {
    //std::shared_ptr<TDetector> output = std::make_shared<TDetector>(*static_cast<TDetector*>(cls->New()));
    std::shared_ptr<TDetector> output(static_cast<TDetector*>(cls->New()));
    fDetectors.push_back(output);
    return output;
  } else {
    return NULL;
  }
}
