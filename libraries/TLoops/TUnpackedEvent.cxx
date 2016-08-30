#include "TUnpackedEvent.h"

#include "TClass.h"
#include "TDetector.h"
#include "TChannel.h"

TUnpackedEvent::TUnpackedEvent() { }

TUnpackedEvent::~TUnpackedEvent() {
  for(auto det : detectors) {
    delete det;
  }
}

void TUnpackedEvent::Build() {
  for(auto frag : fragments) {
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

void TUnpackedEvent::AddRawData(TFragment* frag) {
  fragments.push_back(frag);
}

void TUnpackedEvent::ClearRawData() {
  for(auto frag : fragments) {
    delete frag;
  }
  fragments.clear();
}

void TUnpackedEvent::BuildHits() {
  for(auto det : detectors) {
    det->BuildHits();
  }
}

TDetector* TUnpackedEvent::GetDetector(TClass* cls, bool make_if_not_found) {
  for(auto det : detectors) {
    if(det->IsA() == cls) {
      return det;
    }
  }

  if(make_if_not_found) {
    TDetector* output = (TDetector*)cls->New();
    detectors.push_back(output);
    return output;
  } else {
    return NULL;
  }
}
