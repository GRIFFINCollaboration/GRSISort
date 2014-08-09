
#include "TGRSIStats.h"
#include "TChannel.h"

ClassImp(TGRSIStats)


std::map<int,TGRSIStats*> *TGRSIStats::fStatsMap = new std::map<int,TGRSIStats*>;

TGRSIStats *TGRSIStats::GetStats(int temp_add) {
  if(fStatsMap->count(temp_add) == 0)
  	fStatsMap->at(temp_add) = new TGRSIStats(temp_add);
  return fStatsMap->at(temp_add);
}



TGRSIStats::TGRSIStats(int temp_add) { fStatAddress = temp_add;}

TGRSIStats::~TGRSIStats() { }

void TGRSIStats::Print(Option_t *opt) {
	printf( "Channel %i|%s deatdtime = %i ns\n",TChannel::GetChannel(GetAddress())->GetNumber(),TChannel::GetChannel(GetAddress())->GetChannelName(),GetDeadTime()   );

}

void TGRSIStats::Clear(Option_t *opt) {
     fDeadTime = -1;;
     fLostEvents = -1;
     fLastChannelIdSeen = -1;

}











