#include "TUnpackedEvent.h"

#include "TClass.h"
#include "TDetector.h"
#include "TChannel.h"
#include "TSortingDiagnostics.h"

TUnpackedEvent::TUnpackedEvent()
{
}

TUnpackedEvent::~TUnpackedEvent() = default;

void TUnpackedEvent::Build()
{
   for(const auto& frag : fFragments) {
      TChannel* channel = TChannel::GetChannel(frag->GetAddress(), true);
      if(channel == nullptr) {
			// add to diagnostics, one time printing of error message is already taken care of by TChannel::GetChannel
			TSortingDiagnostics::Get()->MissingChannel(frag->GetAddress());
         continue;
      }

      TClass* detClass = channel->GetClassType();
      if(detClass == nullptr) {
			TSortingDiagnostics::Get()->AddDetectorClass(channel);
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
      std::shared_ptr<TDetector> output(static_cast<TDetector*>(cls->New()));
      fDetectors.push_back(output);
      return output;
   }
   return nullptr;
}

std::ostringstream TUnpackedEvent::Print()
{
	std::ostringstream str;
	str<<fDetectors.size()<<" detector types:"<<std::endl;
	for(const auto& det : fDetectors) {
		str<<"detector type "<<det->ClassName()<<std::endl;
		for(int i = 0; i < det->GetMultiplicity(); ++i) {
			auto hit = det->GetHit(i);
			str<<hit<<" "<<hex(hit->GetAddress(),4)<<" "<<std::setw(16)<<hit->GetTimeStamp()<<" "<<std::setw(16)<<static_cast<int>(hit->Charge())<<" "<<std::setw(16)<<hit->GetEnergy()<<std::endl;
		}
	}
	return str;
}
