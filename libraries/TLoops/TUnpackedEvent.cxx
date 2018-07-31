#include "TUnpackedEvent.h"

#include "TClass.h"
#include "TDetector.h"
#include "TChannel.h"

TUnpackedEvent::TUnpackedEvent()
{
}

TUnpackedEvent::~TUnpackedEvent() = default;

void TUnpackedEvent::Build()
{
   for(const auto& frag : fFragments) {
      TChannel* channel = TChannel::GetChannel(frag->GetAddress());
      if(channel == nullptr) {
         //std::cout<<"Failed to find channel for address "<<frag->GetAddress()<<std::endl;
			//TODO: add this to Diagnostics
         continue;
      }

      TClass* detClass = channel->GetClassType();
      if(detClass == nullptr) {
         std::cout<<"Failed to find detector class "<<channel->GetClassType()<<std::endl;
         continue;
      }

      GetDetector(detClass, true)->AddFragment(frag, channel);
   }

   BuildHits();
   ClearRawData();
}

void TUnpackedEvent::AddRawData(const std::shared_ptr<const TFragment>& frag)
{
   fFragments.push_back(frag);
}

void TUnpackedEvent::ClearRawData()
{
   fFragments.clear();
}

void TUnpackedEvent::BuildHits()
{
   for(const auto& det : fDetectors) {
      det->BuildHits();
   }
}

std::shared_ptr<TDetector> TUnpackedEvent::GetDetector(TClass* cls, bool make_if_not_found)
{
   for(auto det : fDetectors) {
      if(det->IsA() == cls) {
         return det;
      }
   }

   if(make_if_not_found) {
      // std::shared_ptr<TDetector> output = std::make_shared<TDetector>(*static_cast<TDetector*>(cls->New()));
      std::shared_ptr<TDetector> output(static_cast<TDetector*>(cls->New()));
      fDetectors.push_back(output);
      return output;
   }
   return nullptr;
}
