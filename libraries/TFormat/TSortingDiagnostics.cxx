#include "TSortingDiagnostics.h"

#include <fstream>
#include <string>

#include "TChannel.h"
#include "TGRSIOptions.h"

TSortingDiagnostics::TSortingDiagnostics() : TSingleton<TSortingDiagnostics>()
{
   Clear();
}

TSortingDiagnostics::TSortingDiagnostics(const TSortingDiagnostics&) : TSingleton<TSortingDiagnostics>()
{
   Clear();
}

TSortingDiagnostics::~TSortingDiagnostics() = default;

void TSortingDiagnostics::Copy(TObject& obj) const
{
   static_cast<TSortingDiagnostics&>(obj).fFragmentsOutOfOrder = fFragmentsOutOfOrder;
   static_cast<TSortingDiagnostics&>(obj).fFragmentsOutOfTimeOrder = fFragmentsOutOfTimeOrder;
   static_cast<TSortingDiagnostics&>(obj).fMissingDetectorClasses = fMissingDetectorClasses;
}

void TSortingDiagnostics::Clear(Option_t*)
{
   fFragmentsOutOfOrder.clear();
   fFragmentsOutOfTimeOrder.clear();
   fMissingDetectorClasses.clear();
}

void TSortingDiagnostics::OutOfTimeOrder(double newFragTime, double oldFragTime, long newEntry)
{
   fFragmentsOutOfTimeOrder[oldFragTime] = std::make_pair(oldFragTime - newFragTime, newEntry);
   // try and find a time before newFragTime
   size_t entry = 0;
   if(!fPreviousTimes.empty()) {
      for(entry = fPreviousTimes.size() - 1; entry > 0; --entry) {
         if(fPreviousTimes[entry] < newFragTime) {
            break;
         }
      }
   }
   long entryDiff = newEntry - (entry * TGRSIOptions::Get()->SortDepth());
   if(entryDiff > fMaxEntryDiff) {
      fMaxEntryDiff = entryDiff;
   }
}

void TSortingDiagnostics::OutOfOrder(long newFragTS, long oldFragTS, long newEntry)
{
   fFragmentsOutOfOrder[oldFragTS] = std::make_pair(oldFragTS - newFragTS, newEntry);
   // try and find a timestamp before newFragTS
   size_t entry = 0;
   if(!fPreviousTimeStamps.empty()) {
      for(entry = fPreviousTimeStamps.size() - 1; entry > 0; --entry) {
         if(fPreviousTimeStamps[entry] < newFragTS) {
            break;
         }
      }
   }
   long entryDiff = newEntry - (entry * TGRSIOptions::Get()->SortDepth());
   if(entryDiff > fMaxEntryDiff) {
      fMaxEntryDiff = entryDiff;
   }
}

void TSortingDiagnostics::MissingChannel(const UInt_t& address)
{
	if(fMissingChannels.find(address) != fMissingChannels.end()) {
		++(fMissingChannels[address]);
	} else {
		fMissingChannels[address] = 0;
	}
}

void TSortingDiagnostics::AddDetectorClass(TChannel* channel)
{
	if(fMissingDetectorClasses.find(channel->GetClassType()) != fMissingDetectorClasses.end()) {
		++(fMissingDetectorClasses[channel->GetClassType()]);
	} else {
		fMissingDetectorClasses[channel->GetClassType()] = 0;
		std::cout<<"Failed to find detector class "<<channel->GetClassType()<<" for channel:"<<std::endl;
		channel->Print();
	}
}

void TSortingDiagnostics::Print(Option_t* opt) const
{
   TString option = opt;
   option.ToUpper();
	if(!fMissingChannels.empty()) {
		std::cout<<"Missing channels:"<<std::endl;
		for(auto it : fMissingChannels) {
			std::cout<<hex(it.first,4)<<": "<<it.second<<std::endl;
		}
	}
	if(!fMissingDetectorClasses.empty()) {
		std::cout<<"Missing detector classes:"<<std::endl;
		for(auto it : fMissingDetectorClasses) {
			std::cout<<it.first->GetName()<<": "<<it.second<<std::endl;
		}
	}
   std::string color;
	if(!fHitsRemoved.empty()) {
		if(option.EqualTo("ERROR")) {
			color = DRED;
		}
		std::cout<<color<<"Removed hits per detector class:"<<RESET_COLOR<<std::endl;
		for(auto it : fHitsRemoved) {
			std::cout<<it.first->GetName()<<": "<<it.second.first<<"/"<<it.second.second<<" = "<<(100.*it.second.first)/it.second.second<<"%"<<std::endl;
		}
	} else {
      if(option.EqualTo("ERROR")) {
         color = DGREEN;
      }
		std::cout<<color<<"No hits were removed!"<<RESET_COLOR<<std::endl;
	}
	color = ""; // reset color string
   if(fFragmentsOutOfOrder.empty() && fFragmentsOutOfTimeOrder.empty()) {
      if(option.EqualTo("ERROR")) {
         color = DGREEN;
      }
      std::cout<<color<<"No fragments out of order!"<<RESET_COLOR<<std::endl;
      return;
   }
   if(option.EqualTo("ERROR")) {
      color = DRED;
   }
	if(!fFragmentsOutOfOrder.empty()) {
		std::cerr<<color<<NumberOfFragmentsOutOfOrder()<<" fragments were out of order, maximum entry difference was "
					<<fMaxEntryDiff<<"!"<<std::endl
					<<"Please consider increasing the sort depth with --sort-depth="<<fMaxEntryDiff<<RESET_COLOR
					<<std::endl;
	}
	if(!fFragmentsOutOfTimeOrder.empty()) {
		std::cerr<<color<<NumberOfFragmentsOutOfTimeOrder()<<" fragments were out of order, maximum entry difference was "
					<<fMaxEntryDiff<<"!"<<std::endl
					<<"Please consider increasing the sort depth with --sort-depth="<<fMaxEntryDiff<<RESET_COLOR
					<<std::endl;
	}
}

void TSortingDiagnostics::Draw(Option_t*)
{
}

void TSortingDiagnostics::WriteToFile(const char* fileName) const
{
   std::ofstream statsOut(fileName);
   statsOut<<std::endl
           <<"Number of fragments out of order = "<<NumberOfFragmentsOutOfOrder()<<std::endl
           <<"Number of fragments out of time order = "<<NumberOfFragmentsOutOfTimeOrder()<<std::endl
           <<"Maximum entry difference = "<<fMaxEntryDiff<<std::endl
           <<std::endl;
}

void TSortingDiagnostics::RemovedHits(TClass* detClass, long removed, long total)
{
	if(fHitsRemoved.find(detClass) == fHitsRemoved.end()) {
		fHitsRemoved[detClass] = std::make_pair(removed, total);
	} else {
		fHitsRemoved[detClass].first += removed;
		fHitsRemoved[detClass].second += total;
	}
}

