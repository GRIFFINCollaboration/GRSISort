
#include "TGRSIStats.h"
#include "TChannel.h"

ClassImp(TGRSIStats)

////////////////////////////////////////////////////////////////
//                                                            //
// TGRSIStats                                                 //
//                                                            //
// This Class records necessary stats for each channel in a   //
// a sub-run. The stats are created during data parsing of    //
// the MIDAS file and are automatically written to the        //
// fragment tree. If the -record_stats flag is used, a text   //
// file is written as well.                                   //
//                                                            //
////////////////////////////////////////////////////////////////

std::map<int,TGRSIStats*> *TGRSIStats::fStatsMap = new std::map<int,TGRSIStats*>;

time_t TGRSIStats::fLowestMidasTimeStamp = 0;
time_t TGRSIStats::fHighestMidasTimeStamp = 0;

Int_t TGRSIStats::fLowestNetworkPacket = 0;
Int_t TGRSIStats::fHighestNetworkPacket = 0;

Long_t TGRSIStats::fGoodEvents = 0;

TGRSIStats *TGRSIStats::GetStats(int temp_add) {
#if MAJOR_ROOT_VERSION < 6
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   if(fStatsMap->count(temp_add) == 0)
  	fStatsMap->insert( std::pair<int,TGRSIStats*>(temp_add,new TGRSIStats(temp_add)));
   return fStatsMap->at(temp_add);
}

TGRSIStats::TGRSIStats(int temp_add) { 
   fStatAddress = temp_add;
   Clear();
}

TGRSIStats::~TGRSIStats() { }

void TGRSIStats::Print(Option_t *opt) const {
//Prints the stats for the current channel.
	printf( "Channel %i|%s deadtime = %f seconds\n",TChannel::GetChannel(GetAddress())->GetNumber(),TChannel::GetChannel(GetAddress())->GetChannelName(),GetDeadTime()*10*10E-9   );
}

void TGRSIStats::Clear(Option_t *opt) {
//Clears the stats for the current channel.
     fDeadTime = 0;;
     fLostEvents = 0;
     fLastChannelIdSeen = -1;
}












