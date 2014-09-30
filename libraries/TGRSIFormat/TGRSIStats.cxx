
#include "TGRSIStats.h"
#include "TChannel.h"

ClassImp(TGRSIStats)


std::map<int,TGRSIStats*> *TGRSIStats::fStatsMap = new std::map<int,TGRSIStats*>;

time_t TGRSIStats::fLowestMidasTimeStamp = 0;
time_t TGRSIStats::fHighestMidasTimeStamp = 0;



TGRSIStats *TGRSIStats::GetStats(int temp_add) {
   Class()->IgnoreTObjectStreamer(true);
  if(fStatsMap->count(temp_add) == 0)
  	fStatsMap->insert( std::pair<int,TGRSIStats*>(temp_add,new TGRSIStats(temp_add)));
  return fStatsMap->at(temp_add);
}



TGRSIStats::TGRSIStats(int temp_add) { 
   fStatAddress = temp_add;
   Clear();
}

TGRSIStats::~TGRSIStats() { }

void TGRSIStats::Print(Option_t *opt) {
	printf( "Channel %i|%s deadtime = %f seconds\n",TChannel::GetChannel(GetAddress())->GetNumber(),TChannel::GetChannel(GetAddress())->GetChannelName(),GetDeadTime()*10*10E-9   );
}

void TGRSIStats::Clear(Option_t *opt) {
     fDeadTime = 0;;
     fLostEvents = 0;
     fLastChannelIdSeen = -1;
}












